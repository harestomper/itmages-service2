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
    NwRecv *ret;
    guint n;

    if (src == NULL)
        return NULL;
        
    ret = nw_recv_new (src->type);
    *ret = *src;
    ret->message = g_strdup (src->message);
    ret->len = 0;
    ret->list = NULL;

    for (n = 0; n < src->len && src->list [n]; n++) {
        ret->list = realloc (ret->list, sizeof (*ret->list) * (n + 2));
        ret->list [n] = nw_data_item_copy (src->list [n]);
    }

    ret->result [n + 1] = NULL;
    ret->len = n;

    return ret;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void nw_recv_free (gpointer data)
{
    gint n;
    NwRecv *recv = (NwRecv *) data;

    if (recv == NULL)
        return;

    if (recv->message)
        g_free (recv->message);

    for (n = 0; n < recv->len && recv->list [n]; n++)    
        nw_data_item_free (ret->list [n]);

    if (recv->list)
        g_free (ret->list);
        
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
        ret->prop = NULL;
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
        ret->val.str = g_strdup (str->val.str);
    else
        ret->val = str->val;

    ret->prop = nw_data_item_copy (src->prop);
    
    return ret;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void nw_data_item_free (gpointer data)
{
    NwDataItem *item = (NwDataItem *) data;

    if (items == NULL)
        return;

    if (item->type == NW_TYPE_STR && item->val.str)
        g_free (item->val.str);

    if (item->key)
        g_free (item->key);
        
    nw_data_item_free (item->prop);
    
    g_free (item);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
NwDataItem* nw_data_item_add (NwDataItem **dest, NwDataItemType t, const gchar *key)
{
    NwDataItem *ret = NULL;

    if (dest == NULL)
        ret = nw_data_item_new (t, key);
    else if (*dest == NULL)
        *dest = ret = nw_data_item_new (t, key);
    else
        ret = nw_data_item_add (&dest->prop, t, key);

    return ret;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
gboolean nw_data_item_next (NwDataItem **item)
{
    if (item && *item)
        *item = (*item)->prop;

    return (item && *item);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
G_DEFINE_BOXED_TYPE (NwRecv, nw_recv, nw_recv_copy, nw_recv_free)
G_DEFINE_BOXED_TYPE (NwDataItem, nw_data_item, nw_data_item_copy, nw_data_item_free)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
