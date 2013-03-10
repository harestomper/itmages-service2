/*
 *      test-xml-utils.c
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


#include "service-types.h"

static void print_image_info (ServiceImageInfo *info)
{
    printf ("Item %s:\n", info->filename);
    printf ("Object: %d\n", (int) info);
    printf ("\tid: %s\n", info->id);
    printf ("\tkey: %s\n", info->key);
    printf ("\tfull: %s\n", info->full);
    printf ("\tsmall: %s\n", info->small);
    printf ("\tserver: %s\n", info->server);
    printf ("\tuser: %s\n", info->username);
    printf ("\thash: %s\n", info->hash);
    printf ("\talbum: %i\n", info->album);
    printf ("\tCreated: %li\n", (long int) info->created);
    printf ("\tSize: %li\n", (long int) info->size);
    printf ("\tWidth: %i\n", info->width);
    printf ("\tHeight: %i\n", info->height);
    printf ("\n");
}

int main (int argc, char **argv)
{

    gchar *xml_data = NULL;
    gsize length;

    if (argc < 2)
        return 0;
        
    if (g_file_get_contents (argv [1], &xml_data, &length, NULL))
    {
        ServiceResponse *response;
        gint n;
        response = service_xml_get_response (xml_data);
        if (response)
        {
            printf ("Response: \n");
            printf ("\tStatus:  %i\n", (gint) response->status);
            printf ("\tError:   %s\n", response->error ? response->error : "NULL");
            printf ("\tUser: %s\n", response->username);
            printf ("\tShort: %s\n", response->short_id ? response->short_id : "NULL");
            printf ("\tID: %s\n", response->id ? response->id : "NULL");
            printf ("\tN_ITEMS: %i\n", response->n_items);
            printf ("\tTotal items: %i\n", response->total_n_items);
            printf ("\tPer page: %i\n", response->total_per_page);

            for (n = 0; n < response->n_items; n++)
                print_image_info (response->items [n]);

            if (response->item)
                print_image_info (response->item);
        }
                
    }
    return 0;
}

