/*
 *      service-image-info.h
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

#ifndef _SERVICE_IMAGE_INFO_H_
#define _SERVICE_IMAGE_INFO_H_

#include <glib.h>
#include <glib-object.h>
#include <stdio.h>

G_BEGIN_DECLS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define SERVICE_TYPE_IMAGE_INFO     (service_image_info_get_type ())
#define ALBUM_TYPE_INFO             (album_info_get_type ())
#define TAG_TYPE_INFO               (tag_info_get_type ())
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
typedef struct _TagInfo             TagInfo;
typedef struct _AlbumInfo           AlbumInfo;
typedef struct _ServiceImageInfo    ServiceImageInfo;
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
typedef enum {
  SII_STATE_NONE,
  SII_STATE_RUN,
  SII_STATE_DONE,
  SII_STATE_FAILED
} ServiceImageInfoState;
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _ServiceImageInfo {
  gchar *id;
  gchar *key;
  gchar *full;
  gchar *small;
  gchar *server;
  gchar *short_id;
  gchar *filename;
  gchar *mime;
  gchar *appname;     // Programm sender
  gchar *hash;        // MD5 hash of image file
  gchar *local_user;  // User sender
  gchar *hosting;     // Hosting's URL
  gchar *device;      // Device sender

  gchar *username;   // in case
  gchar *password;   // for session
  gchar *proxynode;  // for session
  gchar *proxyport;  // for session
  gchar *proxyuser;  // for session
  gchar *proxypass;  // for session
  gint   proxytype;  // for session

  ServiceImageInfoState state;
  gchar *message;

  gint  oid;
  gint  width;
  gint  height;
  gint64 size;
  gint64 created;
  gint64 modified;
  gint   album;       // ID of album
  gint   album_ex;    // ID of external album

  void  *user_data1;
  void  *user_data2;
  void  *user_data3;
  void  *user_data4;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
GType                 service_image_info_get_type     (void) G_GNUC_CONST;
ServiceImageInfo*     service_image_info_new          (void);
void                  service_image_info_free         (ServiceImageInfo *info);
void                  service_image_info_freev        (ServiceImageInfo **info, gint n_info);
ServiceImageInfo*     service_image_info_copy         (ServiceImageInfo *src);
gchar*                service_image_info_to_string    (ServiceImageInfo *info);
ServiceImageInfo*     service_image_info_from_string  (const gchar *str_info);
gchar*                service_image_info_get_hash     (const gchar *filename);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _AlbumInfo {
    gint      album_id;
    gint      album_ex;
    gchar    *album_name;
    gchar    *description;
    gchar    *cover_id;
    gint64    created;
    gint64    modified;
    gboolean  synchronized;
    gint      n_images;
    gint     *images;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
GType             album_info_get_type (void) G_GNUC_CONST;
AlbumInfo*        album_info_new (void);
void              album_info_free (AlbumInfo *info);
AlbumInfo*        album_info_copy (AlbumInfo *info);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _TagInfo {
    gint      tag_id;
    gchar*    tag;
    gint64    created;
    gint64    modified;
    gboolean  synchronized;
    gint      n_images;
    gint*     images;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
GType             tag_info_get_type (void)  G_GNUC_CONST;
TagInfo*          tag_info_new      (void);
void              tag_info_free     (TagInfo *info);
TagInfo*          tag_info_copy     (TagInfo *info);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------


G_END_DECLS

#endif /* _SERVICE_IMAGE_INFO_H_ */

