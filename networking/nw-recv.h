/*
 * nw-recv.h
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

#ifndef __NW_RECV_H__
#define __NW_RECV_H__

#include <glib-object.h>

typedef struct  _NwRecv          NwRecv;
typedef enum    _NwRecvType      NwRecvType;
typedef struct  _NwDataItem      NwDataItem;
typedef enum    _NwDataItemType  NwDataItemType;
typedef enum    _NwRecvStatus    NwRecvStatus;
typedef enum    _NwDirect        NwDirect;


enum _NwRecvType
{
    NW_RECV_TYPE_NONE         = 0,
    NW_RECV_TYPE_PING         = (1 << 1),
    NW_RECV_TYPE_CHECK        = (1 << 2),
    NW_RECV_TYPE_LOGIN        = (NW_RECV_TYPE_CHECK | (1 << 3)),
    NW_RECV_TYPE_LOGOUT       = (NW_RECV_TYPE_CHECK | (1 << 4)),
    NW_RECV_TYPE_INFO         = (NW_RECV_TYPE_LOGIN | (1 << 5)),
    NW_RECV_TYPE_GET          = (NW_RECV_TYPE_LOGIN | (1 << 6)),
    NW_RECV_TYPE_DELETE       = (NW_RECV_TYPE_LOGIN | (1 << 7)),
    NW_RECV_TYPE_UPLOAD       = (1 << 8),
    NW_RECV_TYPE_SHORT        = (1 << 9),
    NW_RECV_TYPE_LOAD         = (1 << 10),
    NW_RECV_TYPE_STOP         = (1 << 11),
    NW_RECV_TYPE_START        = (1 << 12),
    NW_RECV_TYPE_PAUSE        = (1 << 13),
};


enum _NwDataItemType
{
    NW_DATA_ITEM_TYPE_NONE,
    NW_DATA_ITEM_TYPE_INT,
    NW_DATA_ITEM_TYPE_STR,
    NW_DATA_ITEM_TYPE_BOOL,
    NW_DATA_ITEM_TYPE_DATE_TIME
};

enum _NwDirect
{
    NW_DIRECT_NONE,
    NW_DIRECT_UP,
    NW_DIRECT_DN
}

enum _NwRecvStatus
{
    NW_RECV_STATUS_UNDEFINED,
    NW_RECV_STATUS_RUNNING,
    NW_RECV_STATUS_OK,
    NW_RECV_STATUS_FAILED,
    NW_RECV_STATUS_CANCELLED
};

struct _NwDataItem
{
    NwDataItemType      type;
    gchar*              key;
    union val {
        gchar*          s;
        gint64          i;
        guint32         t;
        gboolean        b;
    };
};


struct _NwRecv
{
    /* Request section */
    NwRecvType   type;
    const gchar* id;
    const gchar* key;
    const gchar* tag;
    const gchar* filename;
    const gchar* cookies;

    /* Response section */
    guint        n_items;
    NwDataItem** items;
    gchar*       message;
    NwRecvStatus status;
    
};

NwRecv*         nw_recv_new             (NwRecvType t);
NwRecv*         nw_recv_copy            (NwRecv *src);
void            nw_recv_free            (gpointer data);

NwDataItem*     nw_data_item_new        (NwDataItemType t, const gchar *key);
NwDataItem*     nw_data_item_copy       (NwDataItem *src);
void            nw_data_item_free       (gpointer data);


GType nw_recv_get_type (void) G_GNUC_CONST;
#define NW_TYPE_RECV (nw_recv_get_type ())

GType nw_data_item_get_type (void) G_GNUC_CONST;
#define NW_TYPE_DATA_ITEM (nw_data_item_get_type ())

GType nw_recv_status_get_type (void) G_GNUC_CONST;
#define NW_TYPE_RECV_STATUS (nw_recv_status_get_type ())

GType nw_data_item_type_get_type (void) G_GNUC_CONST;
#define NW_TYPE_DATA_ITEM_TYPE (nw_data_item_type_get_type ())

GType nw_recv_type_get_type (void) G_GNUC_CONST;
#define NW_TYPE_RECV_TYPE (nw_recv_type_get_type ())

GType nw_direct_get_type (void) G_GNUC_CONST;
#define NW_TYPE_DIRECT (nw_direct_get_type ())


#endif /* __NW_RECV_H__ */
