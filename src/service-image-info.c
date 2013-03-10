/*
 *      service-image-info.c
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


#include <stdio.h>
#include "service-image-info.h"

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define V_EMPTY   "none"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServiceImageInfo* service_image_info_new (void)
{
    ServiceImageInfo *info;

    info = g_slice_new (ServiceImageInfo);
    info->id = NULL;
    info->key = NULL;
    info->full = NULL;
    info->small = NULL;
    info->server = NULL;
    info->short_id = NULL;
    info->filename = NULL;
    info->mime = NULL;
    info->appname = NULL;
    info->hash = NULL;
    info->local_user = NULL;
    info->username = NULL;
    info->password = NULL;
    info->proxynode = NULL;
    info->proxyport = 0;
    info->proxytype = -100;
    info->proxyuser = NULL;
    info->proxypass = NULL;
    info->hosting = NULL;
    info->device = NULL;
    info->album = 0;
    info->oid = 0;
    info->width = 0;
    info->height = 0;
    info->size = 0;
    info->created = 0;
    info->modified = 0;
    info->state = SII_STATE_NONE;
    info->message = NULL;
    info->user_data1 = NULL;
    info->user_data2 = NULL;
    info->user_data3 = NULL;
    info->user_data4 = NULL;

    return info;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define srvfree(v)  ((v) ?  g_free (v) : NULL)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void  service_image_info_free  (ServiceImageInfo *info)
{
    ServiceImageInfo *v = info;

    if (v == NULL)
        return;

    srvfree (v->id);
    srvfree (v->key);
    srvfree (v->full);
    srvfree (v->small);
    srvfree (v->server);
    srvfree (v->short_id);
    srvfree (v->filename);
    srvfree (v->appname);
    srvfree (v->hash);
    srvfree (v->mime);
    srvfree (v->local_user);
    srvfree (v->username);
    srvfree (v->hosting);
    srvfree (v->device);
    srvfree (v->message);

    g_slice_free (ServiceImageInfo, info);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void service_image_info_freev (ServiceImageInfo **info, gint n_info)
{
    gint n;
    if (info == NULL)
        return;

    for (n = 0; n < n_info; n++)
        service_image_info_free (info [n]);

    g_free (info);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServiceImageInfo *service_image_info_copy (ServiceImageInfo *src)
{
    ServiceImageInfo *dest;

    if (src == NULL)
        return NULL;
        
    dest = service_image_info_new ();
    dest->id = g_strdup (src->id);
    dest->key = g_strdup (src->key);
    dest->full = g_strdup (src->full);
    dest->small = g_strdup (src->small);
    dest->server = g_strdup (src->server);
    dest->short_id = g_strdup (src->short_id);
    dest->filename = g_strdup (src->filename);
    dest->mime = g_strdup (src->mime);
    dest->appname = g_strdup (src->appname);
    dest->hash = g_strdup (src->hash);
    dest->local_user = g_strdup (src->local_user);
    dest->device = g_strdup (src->device);
    dest->username = g_strdup (src->username);
    dest->password = g_strdup (src->password);
    dest->proxynode = g_strdup (src->proxynode);
    dest->proxyport = src->proxyport;
    dest->proxyuser = g_strdup (src->proxyuser);
    dest->proxytype = src->proxytype;
    dest->state = src->state;
    dest->message = g_strdup (src->message);
    dest->oid = src->oid;
    dest->width = src->width;
    dest->height = src->height;
    dest->size = src->size;
    dest->created = src->created;
    dest->modified = src->modified;
    dest->album = src->album;
    dest->album_ex = src->album_ex;

    return dest;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar* service_image_info_to_string (ServiceImageInfo *info)
{
    GString *string;
    gchar *result;

    string = g_string_new ("");

    g_string_append_printf (string, "id:string:%s\n", info->id ? info->id : V_EMPTY);
    g_string_append_printf (string, "key:string:%s\n", info->key ? info->key : V_EMPTY);
    g_string_append_printf (string, "full:string:%s\n", info->full ? info->full : V_EMPTY);
    g_string_append_printf (string, "small:string:%s\n", info->small ? info->small : V_EMPTY);
    g_string_append_printf (string, "server:string:%s\n", info->server ? info->server : V_EMPTY);
    g_string_append_printf (string, "short:string:%s\n", info->short_id ? info->short_id : V_EMPTY);
    g_string_append_printf (string, "filename:string:%s\n", info->filename ? info->filename : V_EMPTY);
    g_string_append_printf (string, "mime:string:%s\n", info->mime ? info->mime : V_EMPTY);
    g_string_append_printf (string, "application:string:%s\n", info->appname ? info->appname : V_EMPTY);
    g_string_append_printf (string, "hash:string:%s\n", info->hash ? info->hash : V_EMPTY);
    g_string_append_printf (string, "local_user:string:%s\n", info->local_user ? info->local_user : V_EMPTY);
    g_string_append_printf (string, "username:string:%s\n", info->username ? info->username : V_EMPTY);
    g_string_append_printf (string, "hosting:string:%s\n", info->hosting ? info->hosting : V_EMPTY);
    g_string_append_printf (string, "device:string:%s\n", info->device ? info->device : V_EMPTY);

    g_string_append_printf (string, "album:integer:%i\n", info->album);
    g_string_append_printf (string, "oid:integer:%i\n", info->oid);
    g_string_append_printf (string, "width:integer:%i\n", info->width);
    g_string_append_printf (string, "height:integer:%i\n",info->height);
    g_string_append_printf (string, "size:long:%" G_GINT64_FORMAT "\n", info->size);
    g_string_append_printf (string, "utime:long:%" G_GINT64_FORMAT "\n", info->created);

    result = g_strdup (string->str);
    g_string_free (string, TRUE);

    return result;
}    
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServiceImageInfo* service_image_info_from_string (const gchar *str_info)
{
    return NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar *service_image_info_get_hash (const gchar *filename)
{
    GChecksum *checksum;
    const gchar *hash;
    gchar *result;
    gchar *contents;
    gsize length;

    checksum = g_checksum_new (G_CHECKSUM_MD5);

    if (g_file_get_contents (filename, &contents, &length, NULL))
    {
        g_checksum_update (checksum, (guchar*) contents, (gssize) length);
        hash = g_checksum_get_string (checksum);
        result = g_strdup (hash);

        g_free (contents);
    } 

    g_checksum_free (checksum);
    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
AlbumInfo* album_info_new (void)
{
    AlbumInfo *info;

    info = g_slice_new (AlbumInfo);
    info->album_id = 0;
    info->album_ex = 0;
    info->album_name = NULL;
    info->description = NULL;
    info->cover_id = NULL;
    info->created = 0;
    info->modified = 0;
    info->synchronized = FALSE;
    info->n_images = 0;
    info->images = NULL;

    return info;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void album_info_free (AlbumInfo *info)
{
    if (info == NULL) return;

    if (info->album_name) g_free (info->album_name);
    if (info->description) g_free (info->description);
    if (info->cover_id) g_free (info->cover_id);
    if (info->images)   g_free (info->images);
    
    g_slice_free (AlbumInfo, info);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
AlbumInfo* album_info_copy (AlbumInfo *info)
{
    AlbumInfo *result;

    result = album_info_new ();
    result->album_id = info->album_id;
    result->album_ex = info->album_ex;
    result->album_name = g_strdup (info->album_name);
    result->description = g_strdup (info->description);
    result->cover_id = g_strdup (info->cover_id);
    result->created = info->created;
    result->modified = info->modified;
    result->synchronized = info->synchronized;
    result->n_images = info->n_images;

    if (result->n_images)
    {
        gint n;
        result->images  = g_new0 (gint, result->n_images + 1);

        for (n = 0; n < result->n_images; n++)
            result->images [n] = info->images [n];
    }  

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
TagInfo* tag_info_new (void)
{
    TagInfo *info;
    info = g_slice_new (TagInfo);
    info->tag_id = 0;
    info->tag = NULL;
    info->created = 0;
    info->modified = 0;
    info->synchronized = FALSE;
    info->n_images = 0;
    info->images = NULL;

    return info;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void tag_info_free (TagInfo *info)
{
    if (info == NULL) return;
    
    if (info->images) g_free (info->images);
    if (info->tag)    g_free (info->tag);
    g_slice_free (TagInfo, info);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
TagInfo* tag_info_copy (TagInfo *info)
{
    TagInfo *v;
    v = tag_info_new ();
    v->tag_id = info->tag_id;
    v->tag = g_strdup (info->tag);
    v->created = info->created;
    v->modified = info->modified;
    v->synchronized = info->synchronized;
    v->n_images = info->n_images;

    if (v->n_images)
    {
        gint n;
        v->images = g_new0 (gint, v->n_images + 1);

        for (n = 0; n < v->n_images; n++)
            v->images [n] = info->images [n];
    }

    return v;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
G_DEFINE_BOXED_TYPE (TagInfo, tag_info, tag_info_copy, tag_info_free)
G_DEFINE_BOXED_TYPE (AlbumInfo, album_info, album_info_copy, album_info_free)
G_DEFINE_BOXED_TYPE (ServiceImageInfo, service_image_info, service_image_info_copy, service_image_info_free)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

