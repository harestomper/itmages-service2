/*
 * nw-object.c
 * 
 * Copyright 2013 Voldemar Khramtsov <harestomper@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "nw-object.h"
#include <curl/curl.h>
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
enum
{
    SIGNAL_ACTION_CHANGED,
    SIGNAL_STATUS_CHANGED,
    SIGNAL_PROGRESS,
    SIGNAL_LAST
};

static guint nw_object_signals [SIGNAL_LAST];
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct _NwObjectPrivate
{
    CURL*                       handle;
    NwRecv*                     request;
    gboolean                    request_need_free;
    gboolean                    cancelled;

    guint                       progress_total;
    guint                       progress_now;
    NwDirect                    progress_direct;

    NwRecvType                  current_action;
    NwRecvStatus                status;
    gchar*                      message;
    gchar*                      cookies;

    gchar*                      response_data;
    guint                       response_len;

    NwObjectStatusFunc          status_function;
    gpointer                    status_user_data;

    NwObjectActionFunc          action_function;
    gpointer                    action_user_data;

    NwObjectProgressFunc        progress_function;
    gpointer                    progress_user_data;
};
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void nw_object_finalize (GObject *object);
static void nw_cclosure_marshal_VOID__UINT_UINT_ENUM (
                                        GClosure     *closure,
                                        GValue       *return_value G_GNUC_UNUSED,
                                        guint         n_param_values,
                                        const GValue *param_values,
                                        gpointer      invocation_hint G_GNUC_UNUSED,
                                        gpointer      marshal_data);
static gint curl_progress_function (NwObject *self, gdouble dt, gdouble dn, gdouble ut, gdouble un);
static void emit_action_signal (NwObject *self, NwRecvType action);
static void emit_status_signal (NwObject *self, NwRecvStatus status);
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
G_DEFINE_TYPE (NwObject, nw_object, G_TYPE_OBJECT)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void nw_object_class_init (NwObjectClass *klass)
{
    GObjectClass *g_object_class;

    g_object_class = G_OBJECT_CLASS (klass);

    g_object_class->finalize = nw_object_finalize;

    nw_object_signals [SIGNAL_ACTION_CHANGED] = g_signal_new (
                        "action-changed",
                        G_OBJECT_CLASS_TYPE (klass),
                        G_SIGNAL_RUN_FIRST,
                        0, NULL, NULL,
                        g_cclosure_marshal_VOID__ENUM,
                        G_TYPE_NONE,
                        1,
                        NW_TYPE_RECV_TYPE);
                        
    nw_object_signals [SIGNAL_STATUS_CHANGED] = g_signal_new (
                        "status-changed",
                        G_OBJECT_CLASS_TYPE (klass),
                        G_SIGNAL_RUN_FIRST,
                        0, NULL, NULL,
                        g_cclosure_marshal_VOID__ENUM,
                        G_TYPE_NONE,
                        1,
                        NW_TYPE_RECV_STATUS);
                        
    nw_object_signals [SIGNAL_PROGRESS] = g_signal_new (
                        "progress",
                        G_OBJECT_CLASS_TYPE (klass),
                        G_SIGNAL_RUN_FIRST,
                        0, NULL, NULL,
                        nw_cclosure_marshal_VOID__UINT_UINT_ENUM,
                        G_TYPE_NONE,
                        3,
                        G_TYPE_UINT,
                        G_TYPE_UINT,
                        NW_TYPE_DIRECT);
                        
    g_type_class_add_private ((gpointer) klass, sizeof (NwObjectPrivate));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void nw_object_finalize (GObject *object)
{
    NwObject *self;

    g_return_if_fail (NW_IS_OBJECT (object));

    self = NW_OBJECT (object);

    if (self->priv->handle)
        curl_easy_cleanup (self->priv->handle);
         
    if (self->priv->message)
        g_free (self->priv->message);

    if (self->priv->cookies)
        g_free (self->priv->cookies);

    if (self->priv->response_data)
        g_free (self->priv->response_data);

    if (self->priv->request && self->priv->request_need_free)
        nw_recv_free (self->priv->request);
        
    G_OBJECT_CLASS (nw_object_parent_class)->finalize (object);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void nw_object_init (NwObject *self)
{
    NwObjectPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, NW_TYPE_OBJECT, NwObjectPrivate);

    priv = self->priv;

    priv->status = NW_RECV_STATUS_UNDEFINED;
    priv->current_action = NW_RECV_TYPE_NONE;
    priv->progress_value = 0;
    priv->cancelled = FALSE;
    priv->message = NULL;
    priv->cookies = NULL;
    priv->request = NULL;
    priv->request_need_free = FALSE;
    priv->handle = NULL;
    priv->response_data = NULL;
    priv->response_len = 0;
    priv->status_function = NULL;
    priv->action_function = NULL;
    priv->progress_function = NULL;
    priv->status_user_data = NULL;
    priv->action_user_data = NULL;
    priv->progress_user_data = NULL;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
NwObject* nw_object_new (void)
{
    return g_object_new (NW_TYPE_OBJECT, NULL);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/*
    Set new request to process
    If you have plan to dont't save original request, then
    'need_copy' should be set to TRUE
    To start the processing of the new request, nw_object_start should be called after.
*/
gboolean nw_object_perform (NwObject *self, NwRecv *recv, gboolean need_copy);
{
    NwObjectPrivate *priv;
    
    g_return_val_if_fail (NW_IS_OBJECT (self), FALSE);

    priv = self->priv;

    if (priv->request && priv->request_need_free) {
        nw_recv_free (priv->request);
        priv->request = NULL;
        priv->request_need_free = FALSE;
    }
    
    if (need_copy) {
        priv->request = nw_recv_copy (recv);
        priv->request_need_free = TRUE;
    } else {
        priv->request = recv;
    }
        
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void nw_object_cancel (NwObject *self)
{
    g_return_if_fail (NW_IS_OBJECT (self));

    self->priv->cancelled = TRUE;
    emit_status_signal (self, NW_RECV_STATUS_CANCELLED);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void emit_status_signal (NwObject *self, NwRecvStatus status)
{
    NwObjectPrivate *priv = self->priv;

    priv->status = status;
    
    if (priv->status_function)
        priv->status_function (self, status, priv->status_user_data);

    g_signal_emit (self, nw_object_signals [SIGNAL_STATUS_CHANGED], 0, status);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void emit_action_signal (NwObject *self, NwRecvType action)
{
    NwObjectPrivate *priv;
    NwRecvType real_type;
    gboolean found = FALSE;

    priv = self->priv;
    
    for (real_type = 1; real_type <= action; real_type <<= 1)
        if (real_type & action)
            found = TRUE;

    if (found) {
        if (priv->action_function)
            priv->action_function (self, real_type, priv->action_user_data);

        g_signal_emit (self, nw_object_signals [SIGNAL_ACTION_CHANGED], 0, real_type);
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
gboolean nw_object_start (NwObject *self)
{
    gboolean result = FALSE;
    NwObjectPrivate *priv;

    g_return_if_fail (NW_IS_OBJECT (self), FALSE);

    priv = self->priv;

    if (priv->request && priv->status != NW_RECV_STATUS_RUNNING
                                    && priv->status != NW_RECV_STATUS_OK) {
        priv->cancelled = FALSE;
        emit_status_signal (self, NW_RECV_STATUS_UNDEFINED);
        result = TRUE;
    } else
        return FALSE;

    /*
        Here actions to start
    */

    return result;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/*
    returned current values of the total size 'total', current
    sent/resaved data 'now' and direction
    NW_DIRECT_UP -- if send
    NW_DIRECT_DN -- if recaving data
    NW_DIRECT_NONE -- if no any progress
    
*/
NwDirect nw_object_get_progress (NwObject *self, guint *total, guint *now)
{
    g_return_val_if_fail (NW_IS_OBJECT (self), 0);

    return self->priv->progress_direct;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
NwRecvStatus nw_object_get_status (NwObject *self)
{
    g_return_val_if_fail (NW_IS_OBJECT (self), NW_RECV_STATUS_UNDEFINED);

    return self->priv->status;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
const gchar* nw_object_get_message (NwObject *self)
{
    g_return_val_if_fail (NW_IS_OBJECT (self), NULL);

    return self->priv->message;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
NwRecvType nw_object_get_action (NwObject *self)
{
    g_return_val_if_fail (NW_IS_OBJECT (self), NW_RECV_TYPE_NONE);

    return self->priv->current_action;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
const gchar* nw_object_get_cookies (NwObject *self)
{
    g_return_val_if_fail (NW_IS_OBJECT (self), NULL);

    return self->priv->cookies;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
const NwRecv* ne_object_get_request (NwObject *self)
{
    g_return_val_if_fail (NW_IS_OBJECT (self), NULL);

    return (const NwRecv*) self->priv->request;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static NwDataItem* parse_response_data (NwObject *self, guint *n_items)
{
    
    return NULL;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static gint curl_progress_function (NwObject *self, gdouble dt, gdouble dn, gdouble ut, gdouble un)
{
    gint result = 0;
    gboolean emit_ok = TRUE;
    gboolean cancelled = FALSE;
    NwObjectPrivate *priv;

    priv = self->priv;

    if (!priv->cancelled) {
        if (dt && dn) {
            priv->progress_total = dt;
            priv->progress_now = dn;
            priv->progress_direct = NW_DIRECT_DN;
        } else if (ut && un) {
            priv->progress_total = ut;
            priv->progress_now = un;
            priv->progress_direct = NW_DIRECT_UP;
        } else {
            emit_ok = FALSE;
        }

        if (emit_ok) {
            if (priv->progress_function) {
                cancelled = priv->progress_function (self,
                                priv->progress_total,
                                priv->progress_now,
                                priv->progress_user_data);
                if (!priv->cancelled)
                    priv->cancelled = cancelled;
            } 

            if (!priv->cancelled) {
                g_signal_emit (self,
                                nw_object_signals [SIGNAL_PROGRESS], 0,
                                priv->progress_total,
                                priv->progress_now,
                                priv->progress_direct);
            }
        }
    }

    return (priv->cancelled ? 1 : 0);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void nw_object_set_status_func (NwObject *self, NwObjectStatusFunc func, gpointer user_data)
{
    g_return_if_fail (NW_IS_OBJECT (self));

    self->priv->status_function = func;
    self->priv->status_user_data = user_data;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void nw_object_set_action_func (NwObject *self, NwObjectActionFunc func, gpointer user_data)
{
    g_return_if_fail (NW_IS_OBJECT (self));

    self->priv->action_function = func;
    self->priv->action_user_data = user_data;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void nw_object_set_progress_func (NwObject *self, NwObjectProgressFunc func, gpointer user_data)
{
    g_return_if_fail (NW_IS_OBJECT (self));

    self->priv->progress_function = func;
    self->priv->progress_user_data = user_data;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void nw_cclosure_marshal_VOID__UINT_UINT_ENUM (GClosure     *closure,
                                    GValue       *return_value G_GNUC_UNUSED,
                                    guint         n_param_values,
                                    const GValue *param_values,
                                    gpointer      invocation_hint G_GNUC_UNUSED,
                                    gpointer      marshal_data)
{
    typedef void (*GMarshalFunc_VOID__UINT_UINT_ENUM) (gpointer     data1,
                                                       guint        arg_1,
                                                       guint        arg_2,
                                                       gint         arg_3,
                                                       gpointer     data2);
    register GMarshalFunc_VOID__UINT_UINT_ENUM callback;
    register GCClosure *cc = (GCClosure*) closure;
    register gpointer data1, data2;

    g_return_if_fail (n_param_values == 2);

    if (G_CCLOSURE_SWAP_DATA (closure)) {
        data1 = closure->data;
        data2 = g_value_peek_pointer (param_values + 0);
    } else {
        data1 = g_value_peek_pointer (param_values + 0);
        data2 = closure->data;
    }

    callback = (GMarshalFunc_VOID__UINT_UINT_ENUM) (marshal_data ? marshal_data : cc->callback);

    callback (data1,
                g_value_get_uint (param_values + 1),
                g_value_get_uint (param_values + 2),
                g_value_get_enum (param_values + 3),
                data2);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
