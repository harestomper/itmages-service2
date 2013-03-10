/*
 *      service-pool.c
 *      
 *      Copyright 2012 Voldemar Khramtsov <harestomper@gmail.com>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include "service-pool.h"
#include <pthread.h>
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define SERVICE_POOL_MAX_THREADS    10

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _ServicePoolPrivate
{
    GSList *children;
    gint    pipe_writer;
    gint    pipe_reader;
    GMutex  mutex_writer;
    GMutex  mutex_counter;
    guint   started_threads;
    guint   max_threads;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
G_DEFINE_BOXED_TYPE (ServiceThreadInfo, service_pth_data, service_pth_data_copy, service_pth_data_free)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
enum {
    SIGNAL_START,
    SIGNAL_STOP,
    SIGNAL_PROGRESS,
    SIGNAL_CANCEL,
    SIGNAL_LAST
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static ServicePool *default_service_pool = NULL;
static guint service_pool_signals [SIGNAL_LAST];
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_pool_finalize      (GObject *object);
static gboolean service_pool_watch_func (GIOChannel *channel, GIOCondition condition, ServicePool *this);
static void service_pool_change_counter (ServicePool *this, gchar direction);
static gboolean service_pool_has_slot   (ServicePool *this);
static ServiceThreadInfo *service_pool_lookup (ServicePool *this, gint task_id);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
G_DEFINE_TYPE (ServicePool, service_pool, G_TYPE_OBJECT)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_pool_class_init (ServicePoolClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = service_pool_finalize;

  g_type_class_add_private ((gpointer) klass, sizeof (ServicePoolPrivate));

  service_pool_signals [SIGNAL_START] = g_signal_new (
                                "task-started",
                                G_OBJECT_CLASS_TYPE (object_class),
                                G_SIGNAL_RUN_LAST,
                                0,
                                NULL,
                                NULL,
                                g_cclosure_marshal_VOID__BOXED,
                                G_TYPE_NONE,
                                1,
                                SERVICE_TYPE_RESPONSE);
                                
  service_pool_signals [SIGNAL_STOP] = g_signal_new (
                                "task-completed",
                                G_OBJECT_CLASS_TYPE (object_class),
                                G_SIGNAL_RUN_LAST,
                                0,
                                NULL,
                                NULL,
                                g_cclosure_marshal_VOID__BOXED,
                                G_TYPE_NONE,
                                1,
                                SERVICE_TYPE_RESPONSE);
                                
  service_pool_signals [SIGNAL_PROGRESS] = g_signal_new (
                                "task-progerss",
                                G_OBJECT_CLASS_TYPE (object_class),
                                G_SIGNAL_RUN_LAST,
                                0,
                                NULL,
                                NULL,
                                g_cclosure_marshal_VOID__BOXED,
                                G_TYPE_NONE,
                                1,
                                SERVICE_TYPE_RESPONSE);
                                
  service_pool_signals [SIGNAL_CANCEL] = g_signal_new (
                                "task-cancelled",
                                G_OBJECT_CLASS_TYPE (object_class),
                                G_SIGNAL_RUN_LAST,
                                0,
                                NULL,
                                NULL,
                                g_cclosure_marshal_VOID__BOXED,
                                G_TYPE_NONE,
                                1,
                                SERVICE_TYPE_RESPONSE);
                                
  
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_pool_finalize (GObject *object)
{
  ServicePool *this;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SERVICE_IS_POOL (object));

  this = SERVICE_POOL (object);

  G_OBJECT_CLASS (service_pool_parent_class)->finalize (object);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_pool_init (ServicePool *this)
{
    ServicePoolPrivate *priv;
    GIOChannel *channel;
    gint pipes [2];
    
    this->priv = G_TYPE_INSTANCE_GET_PRIVATE (this, SERVICE_TYPE_POOL, ServicePoolPrivate);
    priv = this->priv;

    priv->children = NULL;
    priv->started_threads = 0;
    priv->max_threads = SERVICE_POOL_MAX_THREADS;
    pipe (pipes);
    priv->pipe_reader = pipes [0];
    priv->pipe_writer = pipes [1];

    g_mutex_init (&priv->mutex_writer);
    g_mutex_init (&priv->mutex_counter);
//    priv->mutex_writer = g_mutex_new ();
//    priv->mutex_counter = g_mutex_new ();

    channel = g_io_channel_unix_new (priv->pipe_reader);

    g_io_add_watch (channel, G_IO_IN, (GIOFunc) service_pool_watch_func, this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServicePool *service_pool_new (void)
{
    return g_object_new (SERVICE_TYPE_POOL, NULL);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServicePool *service_pool_default (void)
{
    if (default_service_pool == NULL)
        default_service_pool = service_pool_new ();

    return default_service_pool;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_pool_change_counter (ServicePool *this, gchar direction)
{
    g_mutex_lock (this->priv->mutex_counter);

    if (direction > 0)
        this->priv->started_threads++;
    else
        this->priv->started_threads--;

    g_mutex_unlock (this->priv->mutex_counter);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean service_pool_has_slot   (ServicePool *this)
{
    return this->priv->max_threads == 0 || this->priv->max_threads > this->priv->started_threads;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint service_pool_append (ServicePool *this, gint task_type, ServiceImageInfo *iminfo)
{
    g_return_val_if_fail (SERVICE_IS_POOL (this), 0);

    ServiceThreadInfo *pth;
    ServicePoolPrivate *priv;

    priv = this->priv;
    pth = service_pth_data_new (this);
    pth->task_id = iminfo && iminfo->oid > 0 ? iminfo->oid : (gint) pth;
    pth->task_type = task_type;
    pth->image_info = service_image_info_copy (iminfo);
    priv->children = g_slist_append (priv->children, pth);

    return pth->task_id;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean service_pool_watch_func (GIOChannel *channel, GIOCondition condition, ServicePool *this)
{
    g_return_val_if_fail (SERVICE_IS_POOL (this), TRUE);

    ServiceResponse *response = NULL;
    ServiceIOData io = {0};
    ServicePoolPrivate *priv;
    guint signal_id = 0;

    priv = this->priv;

    read (priv->pipe_reader, (void*) &io, sizeof (ServiceIOData));

    switch (io.task_type)
    {
        case SERVICE_STAGE_START:
            signal_id = SIGNAL_START;
            break;

        case SERVICE_STAGE_STOP:
            signal_id = SIGNAL_STOP;
            break;

        case SERVICE_STAGE_PROGRESS:
            signal_id = SIGNAL_PROGRESS;
            break;

        case SERVICE_STAGE_CANCEL:
            signal_id = SIGNAL_CANCEL;
            break;
            
        default:
            return TRUE;
    }

    if (io.result_data)
        response = service_xml_get_response (io.result_data);

    if (response == NULL)
    {
        response = service_response_new ();
        response->status = TRUE;
    }
        
    response->task_id = io.task_id;
    response->task_type = io.task_type;
    response->up_total = io.up_total;
    response->up_chunk = io.up_chunk;
    response->dn_total = io.dn_total;
    response->dn_chunk = io.dn_chunk;

    g_signal_emit (this, service_pool_signals [signal_id], 0, response);

    service_response_free (response);

    return TRUE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static ServiceThreadInfo *service_pool_lookup (ServicePool *this, gint task_id)
{
    g_return_val_if_fail (SERVICE_IS_POOL (this), NULL);

    GSList *node;

    for (node = this->priv->children; node; node = node->next)
    {
        ServiceThreadInfo *data;
        data = node->data;

        if (data->task_id == task_id)
            return data;
    }

    return NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_pool_cancel (ServicePool *this, gint task_id)
{
    g_return_val_if_fail (SERVICE_IS_POOL (this), FALSE);

    ServiceThreadInfo *pth;

    if (pth)
    {
        if (pth->thread_id)
            
            
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void          service_pool_stop_all         (ServicePool *this);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint          service_pool_get_max_threads  (ServicePool *this);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void          service_pool_set_max_threads  (ServicePool *this, gint value);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint          service_pool_get_started      (ServicePool *this);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean      service_pool_get_is_started   (ServicePool *this, gint task_id);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint*         service_pool_get_tasks        (ServicePool *this, gint *n_items);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServiceThreadInfo* service_pth_data_new (ServicePool *pool)
{
    ServiceThreadInfo *pth;

    pth = g_slice_new0 (ServiceThreadInfo);

    if (SERVICE_IS_POOL (pool)) {
        pth->pipe_write = pool->priv->pipe_writer;
        pth->mutex_writer = &pool->priv->mutex_writer;
    } else {
        pth->mutex_writer = NULL;
        pth->pipe_writer = 0;
    }
        
    pth->cancelled = FALSE;
    pth->thread_id = (pthread_t) 0;
    pth->task_id = 0;
    pth->task_type = 0;
    pth->task_stage = 0;

    return pth;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServiceThreadInfo* service_pth_data_copy (const ServiceThreadInfo *data)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void service_pth_data_free (ServiceThreadInfo *data);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

