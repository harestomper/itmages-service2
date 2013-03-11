/*
 * mw-recv.c
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


#include <stdio.h>
#include "nw-recv.h"

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
NwRecv* nw_recv_new (NwRecvType t)
{
    NwRecv *retval;

    retval = g_slice_new0 (NwRecv);
    retval->type = t;
    
    return retval;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
NwRecv* nw_recv_copy (NwRecv *src)
{
    NwRecv *retval;

    if (src == NULL)
        return NULL;
        
    retval = nw_recv_new (src->type);
    *retval = *src;
    retval->message = g_strdup (src->message);

    for (retval->n_items = 0; retval->n_items < src->n_items; retval->n_items++) {
        retval->items = realloc (retval->items, sizeof (*retval->items) * (retval->n_items + 1));
        retval->items [retval->n_items] = nw_data_item_copy (src->items [retval->n_items]);
    }

    return retval;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void nw_recv_free (gpointer data)
{
    gint n;
    NwRecv *recv = (NwRecv *) data;

    if (recv == NULL)
        return;

    for (n = 0; n < recv->n_items; n++)
        nw_data_free (recv->items [n]);

    if (recv->items)
        g_free (recv->items);

    g_slice_free (NwRecv, recv);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
NwDataItem* nw_data_item_new (NwDataItemType t, const gchar *key)
{
    NwDataItem *ret;

    ret = malloc (sizeof (*ret));

    if (ret) {
        memset (ret, 0x00, sizeof (*ret));
        ret->key = g_strdup (key);
        ret->type = t;
    }

    return ret;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
NwDataItem* nw_data_item_copy (NwDataItem *src)
{
    NwDataItem *ret;

    if (src == NULL)
        return;
        
    ret = nw_data_item_new (src->type, src->key);

    if (ret->type == NW_TYPE_STR)
        ret->val.s = g_strdup (str->val.s);
    else
        ret->val = str->val;

    return ret;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void nw_data_item_free (gpointer data)
{
    NwDataItem *item = (NwDataItem *) data;

    if (items == NULL)
        return;

    if (item->type == NW_TYPE_STR && item->val.s)
        g_free (item->val.s);

    g_free (item);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
G_DEFINE_BOXED_TYPE (NwRecv, nw_recv, nw_recv_copy, nw_recv_free)
G_DEFINE_BOXED_TYPE (NwDataItem, nw_data_item, nw_data_item_copy, nw_data_item_free)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
