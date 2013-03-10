/*
 * nw-pool.h
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


#ifndef __NW_POOL_H__
#define __NW_POOL_H__

#include <glib-object.h>

G_BEGIN_DECLS


#define NW_TYPE_POOL             (nw_pool_get_type ())
#define NW_POOL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NW_TYPE_POOL, NwPool))
#define NW_POOL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NW_TYPE_POOL, NwPoolClass))
#define NW_IS_POOL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NW_TYPE_POOL))
#define NW_IS_POOL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NW_TYPE_POOL))
#define NW_POOL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NW_TYPE_POOL, NwPoolClass))

typedef struct _NwPool         NwPool;
typedef struct _NwPoolClass    NwPoolClass;
typedef struct _NwPoolPrivate  NwPoolPrivate;

struct _NwPool
{
  GObject parent;
  /* add your public declarations here */
  NwPoolPrivate *priv;
};

struct _NwPoolClass
{
  GObjectClass parent_class;
};


GType nw_pool_get_type (void);

NwPool *nw_pool_new (void);


G_END_DECLS

#endif /* __NW_POOL_H__ */
