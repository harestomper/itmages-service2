/*
 * nw-pool.c
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

#include "nw-pool.h"

struct _NwPoolPrivate
{
  /* add your private declarations here */
  gpointer delete_me;
};

static void nw_pool_finalize (GObject *object);

G_DEFINE_TYPE (NwPool, nw_pool, G_TYPE_OBJECT)


static void
nw_pool_class_init (NwPoolClass *klass)
{
  GObjectClass *g_object_class;

  g_object_class = G_OBJECT_CLASS (klass);

  g_object_class->finalize = nw_pool_finalize;

  g_type_class_add_private ((gpointer)klass, sizeof (NwPoolPrivate));
}


static void
nw_pool_finalize (GObject *object)
{
  NwPool *self;

  g_return_if_fail (NW_IS_POOL (object));

  self = NW_POOL (object);

  G_OBJECT_CLASS (nw_pool_parent_class)->finalize (object);
}


static void
nw_pool_init (NwPool *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, NW_TYPE_POOL, NwPoolPrivate);
}


NwPool *
nw_pool_new (void)
{
  return g_object_new (NW_TYPE_POOL, NULL);
}
