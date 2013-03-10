/*
 *      service-pool.h
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


#ifndef __SERVICE_POOL_H__
#define __SERVICE_POOL_H__

#include <glib-object.h>

G_BEGIN_DECLS


#define SERVICE_TYPE_POOL             (service_pool_get_type ())
#define SERVICE_POOL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SERVICE_TYPE_POOL, ServicePool))
#define SERVICE_POOL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), SERVICE_TYPE_POOL, ServicePoolClass))
#define SERVICE_IS_POOL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SERVICE_TYPE_POOL))
#define SERVICE_IS_POOL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), SERVICE_TYPE_POOL))
#define SERVICE_POOL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), SERVICE_TYPE_POOL, ServicePoolClass))

typedef struct _ServicePool           ServicePool;
typedef struct _ServicePoolClass      ServicePoolClass;
typedef struct _ServicePoolPrivate    ServicePoolPrivate;

struct _ServicePool
{
  GObject parent;
  /* add your public declarations here */
  
  ServicePoolPrivate *priv;
};

struct _ServicePoolClass
{
  GObjectClass parent_class;
};



GType         service_pool_get_type         (void);
ServicePool*  service_pool_new              (void);
ServicePool*  service_pool_default          (void);
gint          service_pool_append           (ServicePool *this, gint task_type, ServiceImageInfo *iminfo);
gboolean      service_pool_cancel           (ServicePool *this, gint task_id);
void          service_pool_stop_all         (ServicePool *this);
gint          service_pool_get_max_threads  (ServicePool *this);
void          service_pool_set_max_threads  (ServicePool *this, gint value);
gint          service_pool_get_started      (ServicePool *this);
gboolean      service_pool_get_is_started   (ServicePool *this, gint task_id);
gint*         service_pool_get_tasks        (ServicePool *this, gint *n_items);


//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

G_END_DECLS

#endif /* __SERVICE_POOL_H__ */
