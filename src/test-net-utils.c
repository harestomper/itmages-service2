/*
 *      test-net-utils.c
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

/*
gcc -Wall `pkg-config --cflags --libs glib-2.0,gobject-2.0,libcurl,libxml-2.0,MagickWand` -o test-net-utils service-net-utils.c service-xml-utils.c service-utils.c service-image-info.c service-imaging.c test-net-utils.c
*/
#include <glib.h>
#include <glib-object.h>
#include <stdio.h>
#include "service-types.h"
#include "service-net-utils.h"

#define V_EMPTY "NULL"
static gboolean complete_callback (ServiceResponse *response)
{
    if (response->task_type == SERVICE_TASK_UPLOAD && response->items) {
        GSList *node;
        for (node = response->items; node; node = node->next) {
            ServiceImageInfo *item;
            item = node->data;
            printf ("id:string:%s\n", item->id ? item->id : V_EMPTY);
            printf ("key:string:%s\n", item->key ? item->key : V_EMPTY);
            printf ("full:string:%s\n", item->full ? item->full : V_EMPTY);
            printf ("small:string:%s\n", item->small ? item->small : V_EMPTY);
            printf ("server:string:%s\n", item->server ? item->server : V_EMPTY);
    //        printf ("short:string:%s\n", item->short_id ? item->short_id : V_EMPTY);
            printf ("filename:string:%s\n", item->filename ? item->filename : V_EMPTY);
            printf ("mime:string:%s\n", item->mime ? item->mime : V_EMPTY);
    //        printf ("application:string:%s\n", item->appname ? item->appname : V_EMPTY);
            printf ("hash:string:%s\n", item->hash ? item->hash : V_EMPTY);
    //        printf ("local_user:string:%s\n", item->local_user ? item->local_user : V_EMPTY);
            printf ("username:string:%s\n", item->username ? item->username : V_EMPTY);
     //       printf ("hosting:string:%s\n", item->hosting ? item->hosting : V_EMPTY);
     //       printf ("device:string:%s\n", item->device ? item->device : V_EMPTY);

            printf ("album:integer:%i\n", item->album);
            printf ("oid:integer:%i\n", item->oid);
            printf ("width:integer:%i\n", item->width);
            printf ("height:integer:%i\n",item->height);
            printf ("size:long:%" G_GINT64_FORMAT "\n", item->size);
            printf ("created:long:%" G_GINT64_FORMAT "\n", item->created);
        }
        
    } else {
        printf ("Response: %i, %s, %s\n", (gint) response->status, response->error, response->username);
    }
    return FALSE;
}


int main (int argc, char **argv)
{
    ServiceNetInfo info  = {0,};

    info.task_id = 123;
    info.task_type = SERVICE_TASK_GET;
    info.username = "elstop";
    info.password = "11021978";
    info.offset = 0;
    info.end_function = complete_callback;
    
//    info.filename = "/home/almidon/Pictures/background.png";
//    info.filename = "http://storage7.static.itmages.ru/i/12/0624/s_1340492970_8139125_419b75514f.png";
//    info.filename_out = "/tmp/s_1340492970_8139125_419b75514f.png";

 //   if (service_net_login (&info))
        service_net_get (&info);
    return 0;
}

