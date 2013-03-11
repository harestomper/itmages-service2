/*
 * nw-object.h
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


#ifndef __NW_OBJECT_H__
#define __NW_OBJECT_H__

#include <glib-object.h>
#include "nw-recv.h"

G_BEGIN_DECLS


#define NW_TYPE_OBJECT             (nw_object_get_type ())
#define NW_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NW_TYPE_OBJECT, NwObject))
#define NW_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NW_TYPE_OBJECT, NwObjectClass))
#define NW_IS_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NW_TYPE_OBJECT))
#define NW_IS_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NW_TYPE_OBJECT))
#define NW_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NW_TYPE_OBJECT, NwObjectClass))

typedef struct _NwObject         NwObject;
typedef struct _NwObjectClass    NwObjectClass;
typedef struct _NwObjectPrivate  NwObjectPrivate;

struct _NwObject
{
  GObject parent;
  /* add your public declarations here */
  NwObjectPrivate *priv;
};

struct _NwObjectClass
{
  GObjectClass parent_class;
};

typedef void (*NwObjectStatusFunc) (NwObject *self, NwRecvStatus status, gpointer user_data);
typedef void (*NwObjectActionFunc) (NwObject *self, NwRecvType action_type, gpointer user_data);
typedef gboolean (*NwObjectProgressFunc) (NwObject *self, guint total, guint now, NwDirect direct, gpointer user_data);

GType nw_object_get_type (void);

NwObject*       nw_object_new                   (void);
NwRecvStatus    nw_object_get_status            (NwObject *self);
const gchar*    nw_object_get_message           (NwObject *self);
NwDirect        nw_object_get_progress          (NwObject *self, guint *total, guint *now);
NwRecvType      nw_object_get_action            (NwObject *self);
const NwRecv*   ne_object_get_request           (NwObject *self);
const gchar*    nw_object_get_cookies           (NwObject *self);
gboolean        nw_object_perform               (NwObject *self, NwRecv *recv, gboolean need_copy);
void            nw_object_cancel                (NwObject *self);
gboolean        nw_object_start                 (NwObject *self);

void            nw_object_set_status_func       (NwObject *self, NwObjectStatusFunc func, gpointer user_data);
void            nw_object_set_action_func       (NwObject *self, NwObjectActionFunc func, gpointer user_data);
void            nw_object_set_progress_func     (NwObject *self, NwObjectProgressFunc func, gpointer user_data);

G_END_DECLS

#endif /* __NW_OBJECT_H__ */
