/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * itmages-service2
 * Copyright (C) Voldemar Khramtsov 2012 <harestomper@gmail.com>
 * 
 * itmages-service2 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * itmages-service2 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SERVICE_DATABASE_H_
#define _SERVICE_DATABASE_H_

#include <gobject.h>
#include "service-image-info.h"

G_BEGIN_DECLS

#define SERVICE_TYPE_DATABASE             (service_database_get_type ())
#define SERVICE_DATABASE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SERVICE_TYPE_DATABASE, ServiceDatabase))
#define SERVICE_DATABASE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), SERVICE_TYPE_DATABASE, ServiceDatabaseClass))
#define SERVICE_IS_DATABASE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SERVICE_TYPE_DATABASE))
#define SERVICE_IS_DATABASE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), SERVICE_TYPE_DATABASE))
#define SERVICE_DATABASE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), SERVICE_TYPE_DATABASE, ServiceDatabaseClass))
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
typedef struct _ServiceDatabase         ServiceDatabase;
typedef struct _ServiceDatabaseClass    ServiceDatabaseClass;
typedef struct _ServiceDatabasePrivate  ServiceDatabasePrivate;
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _ServiceDatabaseClass
{
  GObjectClass parent_class;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _ServiceDatabase
{
  GObject parent_instance;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
GType service_database_get_type (void) G_GNUC_CONST;

ServiceDatabase*  service_database_default                (const gchar *database_name);

/* Image info table */
gboolean          service_database_info_table_create      (ServiceDatabase *this);
gint              service_database_info_new               (const gchar *hash);
gint              service_database_info_get_oid           (const gchar *hash);
gboolean          service_database_info_set               (ServiceImageInfo *info);
gboolean          service_database_info_get               (ServiceImageInfo **info);
gboolean          service_database_info_remove            (ServiceImageInfo *info);
ServiceImageInfo* service_database_info_get_image         (gint oid);
GSList*           service_database_info_get_list          (void);
gint*             service_database_info_get_oid_list      (gint *n_elements);


/* Albums */
gboolean          service_database_album_table_create     (ServiceDatabase *this);
gint              service_database_album_new              (const gchar *album_name);
gboolean          service_database_album_set_name         (gint oid, const gchar* album_name);
gboolean          service_database_album_set_soid         (gint oid, gint soid);
gboolean          service_database_album_set_cover        (gint oid, const gchar *cover_id);
gboolean          service_database_album_set_description  (gint oid, const gchar *description);
gboolean          service_database_album_set_sync         (gint oid, gboolean setting);

gint              service_database_album_get_soid         (gint oid, const gchar *album_name);
gchar*            service_database_album_get_name         (gint oid);
gchar*            service_database_album_get_name_from_soid (gint soid);
gint              service_database_album_get_oid          (const gchar *album_name);
gchar*            service_database_album_get_cover        (gint oid);
gchar*            service_database_album_get_description  (gint oid);
gint64            service_database_album_get_created      (gint oid);
gint64            service_database_album_get_modified     (gint oid);
gboolean          service_database_album_get_sync         (gint oid);
gint*             service_database_album_get_images       (gint oid, guint *n_images);
gboolean          service_database_album_remove           (gint oid, const gchar* album_name);
AlbumInfo*        service_database_album_info_get         (gint album_id, gboolean with_images);
gboolean          service_database_album_info_set         (AlbumInfo* info);


/* Tags info table */
gboolean          service_database_tag_table_create       (ServiceDatabase *this);
gboolean          service_database_tag_has_tag            (gint image_id, const gchar *tag);
gboolean          service_database_tag_new                (gint image_id, const gchar *tag);
gchar**           service_database_tag_get_list           (gint *n_tags);
gchar**           service_database_tag_get_list_of_image  (gint image_id, gint *n_tags);
gint*             service_database_tag_get_images         (const gchar *tag, gint *n_images);
gboolean          service_database_tag_remove_image       (gint image_id);
gboolean          service_database_tag_remove_tag         (const gchar *tag);
gboolean          service_database_tag_info_get           (const gchar *tag, TagInfo **info, gboolean with_images);
gboolean          service_database_tag_info_set           (TagInfo *info);


/* Comments info table */
gboolean          service_database_comment_table_create   (ServiceDatabase *this);
gint              service_database_comment_new            (gint image_id, const gchar *comment);
gchar*            service_database_comment_by_image_id    (gint image_id);
gint64            service_database_comment_get_time       (gint oid);
gint              service_database_comment_get_oid        (gint image_id, const gchar *comment);
gboolean          service_database_comment_get_sync       (gint oid);
void              service_database_comment_set_sync       (gint oid, gboolean setting);
gboolean          service_database_comment_set            (gint oid, const gchar *comment);
gboolean          service_database_comment_remove         (gint oud);
gboolean          service_database_comment_image_remove   (gint image_id);


G_END_DECLS

#endif /* _SERVICE_DATABASE_H_ */
