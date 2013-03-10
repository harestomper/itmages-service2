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

#include "config.h"
#include "service-database.h"
#include "service-sql-statements.h"

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
G_DEFINE_TYPE (ServiceDatabase, service_database, G_TYPE_OBJECT);
#define DbPrivate()  ((service_database_default (NULL))->priv)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static ServiceDatabase *default_database = NULL;
static gint service_database_load_status = 0;
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _ServiceDatabasePrivate {

  sqlite3      *db;
  
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_database_init (ServiceDatabase *this)
{
    this->priv = G_TYPE_INSTANCE_GET_PRIVATE (this, SERVICE_TYPE_DATABASE, ServiceDatabasePrivate);
    this->priv->db = NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_database_finalize (GObject *object)
{
  /* TODO: Add deinitalization code here */
  ServiceDatabasePrivate *priv;

  priv = SERVICE_DATABASE (object);

  sqlite3_close (priv->db);
  
  G_OBJECT_CLASS (service_database_parent_class)->finalize (object);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_database_class_init (ServiceDatabaseClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  GObjectClass* parent_class = G_OBJECT_CLASS (klass);

  object_class->finalize = service_database_finalize;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static ServiceDatabase *service_database_new (const gchar *database_name)
{
    GObject *object;
    ServiceDatabase *this;
    ServiceDatabasePrivate *priv;
    gchar *database_filename;
    gchar *user_config_dir;

    object = g_object_new (SERVICE_TYPE_DATABASE, NULL);
    this = SERVICE_DATABASE (object);
    priv = this->priv;

    user_config_dir = g_build_filename (G_DIR_SEPARATOR_S, g_get_user_config_dir (), PACKAGE_NAME, NULL);

    if (database_name)
        database_filename = g_strdup (database_name);
    else
        database_filename = g_build_filename (G_DIR_SEPARATOR_S, user_config_dir, PACKAGE_NAME".sqlite", NULL);

    if (!g_file_test (user_config_dir, G_FILE_TEST_EXISTS))
        g_assert (g_mkdir_with_parents (user_config_dir, 0770) < 0);

    if (sqlite3_open_v2 (database_filename, &priv->db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK)
        g_error ("%s: %i, %s", G_STRLOC, sqlite3_errcode (priv->db), sqlite3_errmsg (priv->db);

    service_database_info_table_create (this);
    service_database_tag_table_create (this);
    service_database_comment_table_create (this);

    g_free (user_config_dir);
    g_free (database_filename);
    
    return this;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServiceDatabase *service_database_default (const gchar *database_name)
{
   if (default_database == NULL) {
      default_database = service_database_new (database_name);
      service_database_load_status = 1;
   }

   return default_database;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// I M A G E  I N F O 
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_info_table_create (ServiceDatabase *this)
{
    gboolean result = FALSE;
    gint code;

    g_return_val_if_fail (SERVICE_IS_DATABASE (this), FALSE);

    code = service_prepare_stmt (this->priv->db, NULL, STMT_INFO_CREATE, NULL, G_STRLOC);
    result = code == SQLITE_OK || code == SQLITE_DONE;

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint service_database_info_new (const gchar *hash)
{
    gint result = -1;
    sqlite3_stmt *stmt = NULL;
    ServiceDatabasePrivate *priv;

    g_return_val_if_fail (hash != NULL, -1);

    priv = DbPrivate ();

    result = service_database_info_get_oid (hash);

    if (result < 1 && service_prepare_stmt (priv->db, &stmt, STMT_INFO_NEW_HASH, NULL, NULL))
    {
        sqlite_bind_text (stmt, "hash", NULL, hash, FALSE);

        if (sqlite3_step (stmt) == SQLITE_DONE)
            result = service_database_info_get_oid (hash);

    }

    sqlite3_finalize (stmt);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint service_database_info_get_oid (const gchar *hash)
{
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt = NULL;
    gint result = -1;
    
    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_INFO_OID_BY_HASH, NULL, NULL))
    {
        sqlite_bind_text (stmt, "hash", NULL, hash, FALSE);

        if (sqlite3_step (stmt) == SQLITE_ROW)
            result = sqlite3_column_int (stmt, 0);

    }

    sqlite3_finalize (stmt);
        
    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_info_set (ServiceImageInfo *info)
{
    gboolean result = FALSE;
    sqlite3_stmt *stmt = NULL;
    ServiceDatabasePrivate *priv;
    ServiceImageInfo *saved = NULL;
    gint oid = -1;

    g_return_val_if_fail (info != NULL, FALSE);

    if (info->oid) {
        oid = info->oid;

    } else if (info->hash) {
        oid = service_database_info_get_oid (info->hash);

        if (oid < 1)
            oid = service_database_info_new (info->hash);

    } else if (info->filename) {
        if (service_imaging_geometry (info->filename,
                                      &info->width,
                                      &info->height,
                                      &info->size,
                                      &info->mime,
                                      &info->hash))
        {
            oid = service_database_info_get_oid (info->hash);

            if (oid < 1)
                oid = service_database_info_new (info->hash);
        }
    }

    if (oid < 1)
        return FALSE;

    saved = service_database_info_get_image (oid);

    if (saved == NULL)
        return FALSE;
        
    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_INFO_SET, NULL, NULL))
    {
        sqlite_bind_text (stmt, "id", saved->id, info->id, TRUE);
        sqlite_bind_text (stmt, "key", saved->key, info->key, TRUE);
        sqlite_bind_text (stmt, "full", saved->full, info->full, TRUE);
        sqlite_bind_text (stmt, "small", saved->small, info->small, TRUE);
        sqlite_bind_text (stmt, "server", saved->server, info->server, TRUE);
        sqlite_bind_text (stmt, "short_id", saved->short_id, info->short_id, TRUE);
        sqlite_bind_text (stmt, "filename", saved->filename, info->filename, TRUE);
        sqlite_bind_text (stmt, "mime", saved->mime, info->mime, TRUE);
        sqlite_bind_text (stmt, "hash", saved->hash, info->hash, TRUE);
        sqlite_bind_text (stmt, "local_user", saved->local_user, info->local_user, TRUE);
        sqlite_bind_text (stmt, "appname", saved->appname, info->appname, TRUE);
        sqlite_bind_text (stmt, "device", saved->device, info->device, TRUE);
        sqlite_bind_int64 (stmt, "size", saved->size, info->size, TRUE);
        sqlite_bind_int64 (stmt, "created", saved->created, info->created, TRUE);
        sqlite_bind_int64 (stmt, "modified", saved->modified, info->modified, FALSE);
        sqlite_bind_int (stmt, "width", saved->width, info->width, TRUE);
        sqlite_bind_int (stmt, "height", saved->height, info->height, TRUE);
        sqlite_bind_int (stmt, "album", saved->album, info->album, FALSE);
        sqlite_bind_int (stmt, "album_ex", saved->album_ex, info->album_ex, FALSE);
        result = sqlite3_step (stmt) == SQLITE_DONE;
    }

    sqlite3_finalize (stmt);
    service_image_info_free (saved);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_info_get (ServiceImageInfo **info)
{
    ServiceImageInfo *saved = NULL;
    gint oid = -1;
    gboolean result = FALSE;

    if (info == NULL)
        return FALSE;
        
    if ((*info)->oid > 0) {
        oid = (*info)->oid;

    } else if ((*info)->hash) {
        oid = service_database_info_get_oid ((*info)->hash);

    } else if ((*info)->filename) {
        gchar *hash;

        hash = service_image_info_get_hash ((*info)->filename);

        if (hash != NULL) {
            oid = service_database_info_get_oid (hash);
            g_free (hash);
        }
    }

    if (oid > 0) {
        saved = service_database_info_get_image (oid);

        if (saved != NULL) {
            service_image_info_free (*info);
            *info = saved;
            result = TRUE;
        }
    }

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_info_remove (ServiceImageInfo *info)
{
    gint oid = -1;
    gboolean result = FALSE;

    if (info == NULL)
        return FALSE;
        
    if ((*info)->oid > 0) {
        oid = (*info)->oid;

    } else if ((*info)->hash) {
        oid = service_database_info_get_oid ((*info)->hash);

    } else if ((*info)->filename) {
        gchar *hash;

        hash = service_image_info_get_hash ((*info)->filename);

        if (hash != NULL) {
            oid = service_database_info_get_oid (hash);
            g_free (hash);
        }
    }

    if (oid > 0) {
        sqlite3_stmt *stmt = NULL;
        ServiceDatabasePrivate *priv;

        priv = DbPrivate ();

        if (service_prepare_stmt (priv->db, &stmt, STMT_INFO_REMOVE, NULL, NULL))
        {
            sqlite_bind_int (stmt, "oid", 0, key, FALSE);
            result = sqlite3_step (stmt) == SQLITE_DONE;
        }

        sqlite3_finalize (stmt);
    }

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServiceImageInfo* service_database_info_get_image (gint oid)
{
    sqlite3_stmt *stmt;
    ServiceDatabasePrivate *priv;
    ServiceImageInfo *info = NULL;

    priv = DbPrivate ();
    
    if (service_prepare_stmt (priv->db, &stmt, STMT_INFO_BY_OID, NULL, NULL))
    {
        sqlite_bind_int (stmt, "oid", 0, oid, FALSE);

        if (sqlite3_step (stmt) == SQLITE_ROW)
        {
            info = service_image_info_new ();
            info->oid = oid;
            info->id = sqlite_column_text (stmt, "id");
            info->key = sqlite_column_text (stmt. "key");
            info->full = sqlite_column_text (stmt, "full");
            info->small = sqlite_column_text (stmt, "small");
            info->server = sqlite_column_text (stmt, "server");
            info->short_id = sqlite_column_text (stmt, "short_id");
            info->filename = sqlite_column_text (stmt, "filename");
            info->hash = sqlite_column_text (stmt, "hash");
            info->mime = sqlite_column_text (stmt, "mime");
            info->appname = sqlite_column_text (stmt, "appname");
            info->local_user = sqlite_column_text (stmt, "local_user");
            info->device = sqlite_column_text (stmt, "device");
            info->width = sqlite_column_int (stmt, "width");
            info->height = sqlite_column_int (stmt, "height");
            info->size = sqlite_column_int64 (stmt, "size");
            info->created = sqlite_column_int64 (stmt, "created");
            info->modified = sqlite_column_int64 (stmt, "modified");
            info->album = sqlite_column_int (stmt, "album");
            info->album_ex = sqlite_column_int (stmt, "album_ex");
        }
    }

    sqlite3_finalize (stmt);

    return info;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
GSList* service_database_info_get_list  (void)
{
    GSList *result = NULL;
    gint n_elements, n, *values;

    values = service_database_info_get_oid_list (&n_elements);

    for (n = 0; n < n_elements; n++)
    {
        ServiceImageInfo *saved = NULL;
        saved = service_database_info_get_image (values [n]);

        if (saved)
            result = g_slist_append (result, saved);
    }

    if (values)
        g_free (values);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint* service_database_info_get_oid_list (gint *n_elements)
{
    gint *result = NULL;
    gint n_elem = 0, n;
    sqlite3_stmt *stmt = NULL;
    ServiceDatabasePrivate *priv;

    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_INFO_GET_OID_LIST, NULL, NULL))
    {
        while (sqlite3_step (stmt) == SQLITE_ROW) n_elem++;
        result = g_new0 (gint, n_elem + 1);

        for (n = 0; n < n_elem; n++)
            result [n] = sqlite3_column_int (stmt, 0);
    }

    sqlite3_finalize (stmt);
    
    if (n_elements)
      *n_elements = n_elem;

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// A L B U M   I N F O
//------------------------------------------------------------------------------
gboolean service_database_album_table_create (ServiceDatabase *this)
{
    gboolean result = FALSE;
    gint code;

    g_return_val_if_fail (SERVICE_IS_DATABASE (this), FALSE);

    code = service_prepare_stmt (this->priv->db, NULL, STMT_ALBUM_CREATE, NULL, G_STRLOC);
    result = code == SQLITE_OK || code == SQLITE_DONE;

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint service_database_album_new (const gchar *album_name)
{
    gint result = -1;
    sqlite3_stmt *stmt = NULL;
    ServiceDatabasePrivate *priv;

    g_return_val_if_fail (hash != NULL, -1);

    priv = DbPrivate ();

    result = service_database_album_get_oid (album_name);

    if (result < 1 && service_prepare_stmt (priv->db, &stmt, STMT_ALBUM_NEW, NULL, NULL))
    {
        sqlite_bind_text (stmt, "albumname", NULL, album_name, FALSE);

        if (sqlite3_step (stmt) == SQLITE_DONE)
            resukt = service_database_album_get_oid (album_name);
    }

    sqlite3_finalize (stmt);
    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_album_set_name (gint album_id, const gchar* album_name)
{
    AlbumInfo *info = NULL;
    gboolean result = FALSE;

    if (album_name == NULL)
        return FALSE;
        
    info = service_database_album_info_get (album_id, FALSE);

    if (info == NULL)
        return FALSE;
        
    g_free (info->albumname);
    info->albumname = g_strdup (album_name);
    result = service_database_album_info_set (info);
    album_info_free (info);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_album_set_soid (gint album_id, gint soid)
{
    AlbumInfo *info = NULL;
    gboolean result = FALSE;

    info = service_database_album_info_get (album_id, FALSE);

    if (info == NULL)
        return FALSE;
        
    info->album_ex = soid;
    result = service_database_album_info_set (info);
    album_info_free (info);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_album_set_cover (gint album_id, const gchar *cover_id)
{
    AlbumInfo *info = NULL;
    gboolean result = FALSE;

    info = service_database_album_info_get (album_id, FALSE);

    if (info == NULL)
        return FALSE;
        
    g_free (info->cover_id);
    info->cover_id = g_strdup (cover_id);
    result = service_database_album_info_set (info);
    album_info_free (info);

    return result;

}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_album_set_description (gint album_id, const gchar *description)
{
    AlbumInfo *info = NULL;
    gboolean result = FALSE;

    info = service_database_album_info_get (album_id, FALSE);

    if (info == NULL)
        return FALSE;
        
    g_free (info->description);
    info->description = g_strdup (description);
    result = service_database_album_info_set (info);
    album_info_free (info);

    return result;

}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_album_set_sync  (gint album_id, gboolean setting)
{
    AlbumInfo *info = NULL;
    gboolean result = FALSE;

    info = service_database_album_info_get (album_id, FALSE);

    if (info == NULL)
        return FALSE;
        
    info->synchronized = setting;
    result = service_database_album_info_set (info);
    album_info_free (info);

    return result;

}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint service_database_album_get_soid (gint album_id, const gchar *album_name)
{
    AlbumInfo *info = NULL;
    gint result = -1;

    if (album_id < 1 && album_name == NULL)
        return -1;

    if (album_id < 1)
        album_id = service_database_album_get_oid (album_name);

    if (album_id < 1)
        return -1;
        
    info = service_database_album_info_get (album_id, FALSE);

    if (info == NULL)
        return -1;

    result = info->album_ex;
    album_info_free (info);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar* service_database_album_get_name (gint album_id)
{
    AlbumInfo *info;
    gchar *result = NULL;

    info = service_database_album_info_get (album_id);

    if (info == NULL)
        return NULL;

    result = g_strdup (info->albumname);
    album_info_free (info);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar* service_database_album_get_name_from_soid (gint album_id)
{
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt = NULL;
    gchar *result = NULL;

    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_ALBUM_NAME_FROM_SOID, NULL, NULL))
    {
        sqlite_bind_int (stmt, "album_ex", 0, album_id, FALSE);

        if (sqlite3_step (stmt) == SQLITE_ROW) 
            result = sqlite_column_text (stmt, "albumname");
    }

    sqlite3_finalize (stmt);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint service_database_album_get_oid (const gchar *album_name)
{
    gint result = -1;
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt;
    
    if (album_name == NULL)
        return -1;
        
    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_ALBUM_GET_OID, NULL, NULL))
    {
        sqlite_bind_int (stmt, "albumname", NULL, album_name, FALSE);

        if (sqlite3_step (stmt) == SQLITE_ROW)
            result = sqlite3_column_int (stmt, 0);
    }

    sqlite3_finalize (stmt);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar* service_database_album_get_cover (gint album_id)
{
    AlbumInfo *info;
    gchar *result = NULL;

    info = service_database_album_info_get (album_id);

    if (info == NULL)
        return NULL;

    result = g_strdup (info->cover_id);
    album_info_free (info);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar* service_database_album_get_description (gint album_id)
{
    AlbumInfo *info;
    gchar *result = NULL;

    info = service_database_album_info_get (album_id);

    if (info == NULL)
        return NULL;

    result = g_strdup (info->description);
    album_info_free (info);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint64 service_database_album_get_created  (gint album_id)
{
    AlbumInfo *info;
    gint64 result = 0;

    info = service_database_album_info_get (album_id);

    if (info == NULL)
        return NULL;

    result = info->created;
    album_info_free (info);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint64 service_database_album_get_modified (gint album_id)
{
    AlbumInfo *info;
    gint64 result = 0;

    info = service_database_album_info_get (album_id);

    if (info == NULL)
        return NULL;

    result = info->modified;
    album_info_free (info);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_album_get_sync (gint album_id)
{
    AlbumInfo *info;
    gboolean result = FALSE;

    info = service_database_album_info_get (album_id);

    if (info == NULL)
        return NULL;

    result = info->synchronized;
    album_info_free (info);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint* service_database_album_get_images (gint album_id, guint *n_images)
{
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt = NULL;
    gint *values, n_elem = 0, n;

    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_ALBUM_GET_IMAGES, NULL, NULL))
    {
        sqlite_bind_int (stmt, "album_id", 0, album_id, FALSE);
        while (sqlite3_step (stmt) == SQLITE_ROW) n_elem++;

        if (n_elem)
        {
            values = g_new0 (gint, n_elem + 1);
            sqlite3_reset (stmt);
            sqlite_bind_int (stmt, "album_id", 0, album_id, FALSE);

            for (n = 0; n < n_elem; n++)
                values [n] = sqlite3_column_int (stmt, 0);
        }
    }

    sqlite3_finalize (stmt);

    if (n_elemets)
      *n_elements = n_elem;

    return values;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_album_remove (gint album_id, const gchar* album_name)
{
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt = NULL;
    gboolean result = FALSE;

    if (album_id < 1 && album_name == NULL)
        return FALSE;

    priv = DbPrivate ();

    if (album_id < 1)
        album_id = service_database_album_get_oid (album_name);

    if (album_id < 1)
        return FALSE;

    if (service_prepare_stmt (priv->db, &stmt, STMT_ALBUM_REMOVE, NULL, NULL))
    {
        sqlite_bind_int (stmt, "album_id", 0, album_id, FALSE);
        result = sqlite3_step (stmt) == SQLITE_DONE;

        if (result)
        {
            sqlite3_finalize (stmt);
            stmt = NULL;

            if (service_prepare_stmt (priv->db, &stmt, STMT_INFO_SET_ALBUM, NULL, NULL))
            {
                sqlite_bind_int (stmt, "album_id", 0, album_id, FALSE);
                sqlite_bind_int (stmt, "new_album", 0, 0, FALSE);
                result = sqlite3_step (stmt) == SQLITE_DONE;
            }
    }

    sqlite3_finalize (stmt);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
AlbumInfo* service_database_album_info_get (gint album_id, gboolean with_images)
{
    AlbumInfo *info = NULL;
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt = NULL;

    if (oid < 0)
        return NULL;

    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_ALBUM_GET, NULL, NULL))
    {
        sqlite_bind_int (stmt, "oid", 0, oid, FALSE);

        if (sqlite3_step (stmt) == SQLITE_ROW)
        {
            info = album_info_new ();
            info->album_id = album_id;
            info->album_ex = sqlite_column_int (stmt, "album_ex");
            info->album_name = sqlite_column_text (stmt, "albumname");
            info->description = sqlite_column_text (stmt, "description");
            info->cover_id = sqlite_column_text (stmt, "cover_id");
            info->created = sqlite_column_int64 (stmt, "created");
            info->modified = sqlite_column_int64 (stmt, "modified");
            info->synchronized = sqlite_column_int (stmt, "syncronized") == 1;
            info->n_images = 0;

            if (with_images)
                info->images = service_database_album_get_images (album_id, &info->n_images);
        }
    }

    sqlite3_finalize (stmt);

    return info;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_album_info_set (AlbumInfo* info)
{
    ServiceDatabasePrivate *priv;
    AlbumInfo *saved = NULL;
    sqlite3_stmt *stmt = NULL;
    gboolean result = FALSE;
    gint album_id = -1;

    priv = DbPrivate ();

    if (info == NULL) {
        return FALSE;

    } else if (info->album_id > 0) {
        album_id = info->album_id;

    } else if (info->album_ex > 0 || info->albumname) {
        const gchar *request = info->albumname ? STMT_ALBUM_OID_FROM_NAME : STMT_ALBUM_OID_FROM_SOID;

        if (service_prepare_stmt (priv->db, &stmt, request, NULL, NULL))
        {
            if (info->album_ex > 0)
                sqlite_bind_int (stmt, "album_ex", 0 , info->album_ex, FALSE);
            else
                sqlite_bind_text (stmt, "albumname", NULL, info->albumname, FALSE);

            if (sqlite3_step (stmt) == SQLITE_ROW)
                album_id = sqlite_column_inf (stmt, "album_id");
            else if (info->albumname)
                album_id = service_database_album_new (info->albumname);
        }

        sqlite3_finalize (stmt);
        stmt = NULL;
        
    } else {
      return FALSE;
    }

    if (album_id < 1)
        return FALSE;

    saved = service_database_album_info_get (album_id, FALSE);

    if (service_prepare_stmt (priv->db, &stmt, STMT_ALBUM_SET, NULL, NULL))
    {
        sqlite_bind_text (stmt, "albumname", NULL, info->albumname ? info->albumname : saved->albumname, FALSE);
        sqlite_bind_text (stmt, "cover_id", NULL, info->cover_id, FALSE);
        sqlite_bind_text (stmt, "description", NULL, info->description, FALSE);
        sqlite_bind_int (stmt, "album_ex", 0, info->album_ex);
        sqlite_bind_int (stmt, "synchronized", 0, info->synchronized, FALSE);
        sqlite_bind_int64 (stmt, "modified", 0, service_get_current_time (), FALSE);
        result = sqlite3_step (stmt) == SQLITE_DONE;
    }

    if (saved)
        album_info_free (saved);

    sqlite3_finalize (stmt);

    return result;
    
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// T A G   I N F O
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_tag_table_create (ServiceDatabase *this)
{
    gboolean result = FALSE;
    gint code;

    g_return_val_if_fail (SERVICE_IS_DATABASE (this), FALSE);

    code = service_prepare_stmt (this->priv->db, NULL, STMT_TAGS_CREATE, NULL, G_STRLOC);
    result = code == SQLITE_OK || code == SQLITE_DONE;

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_tag_has_tag (gint image_id, const gchar *tag)
{
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt = NULL;
    gboolean result = FALSE;
    
    if (image_id < 1 || tag == NULL || strlen (tag) == 0)
        return FALSE;

    priv = DbPrivate ();
    
    if (service_prepare_stmt (priv->db, &stmt, STMT_TAG_HAS_TAG, NULL, NULL))
    {
        sqlite_bind_text (stmt, "tag", NULL, tag, FALSE);
        sqlite_bind_int (stmt, "image_id", 0, image_id, FALSE);
        result = sqlite3_step (stmt) == SQLITE_ROW;
    }

    sqlite3_finalize (stmt);
    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint service_database_tag_new  (gint image_id, const gchar *tag)
{
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt = NULL;
    gboolean result = FALSE;

    if (image_id < 1 || tag == NULL || strlen (tag) == 0)
        return FALSE;

    if (service_database_tag_has_tag (image_id, tag))
        return FALSE;

    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_TAG_NEW, NULL, NULL)) 
    {
        sqlite_bind_int (stmt, "image_id", 0, image_id, FALSE);
        sqlite_bind_text (stmt, "tag", NULL, tag, FALSE);
        result = sqlite3_step (stmt) == SQLITE_DONE;
    }
    
    sqlite3_finalize (stmt);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar** service_database_tag_get_list (gint *n_tags) 
{
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt = NULL;
    gchar **result = NULL;
    gint n_elem = 0, n;

    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_TAGS_GET_LIST, NULL, NULL))
    {
       while (sqlite3_step (stmt) == SQLITE_ROW) n_elem++;

       if (n_elem > 0)
       {
          result = g_new0 (gchar*, n_elem + 1);

          for (n = 0; n < n_elem; n++)
              result [n] = g_strdup ((const gchar*) sqlite3_column_text (stmt, 0));

          result [n_elem] = NULL;
       }
    }

    sqlite3_finalize (stmt);

    if (n_tags)
        *n_tags = n_elem;

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar** service_database_tag_get_list_of_image (gint image_id, gint *n_tags)
{
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt = NULL;
    gchar **result = NULL;
    gint n_elem = 0, n;

    if (n_tags)
        *n_tags = 0;
        
    if (image_id < 1)
        return NULL;

    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_TAG_LIST_OF_IMAGE, NULL, NULL))
    {
        sqlite_bind_int (stmt, "image_id", 0, image_id, FALSE);
        while (sqlite3_step (stmt) == SQLITE_ROW) n_elem++;

        if (n_elem > 0)
        {
            sqlite_bind_int (stmt, "image_id", 0, image_id, FALSE);
            result = g_new0 (gchar*, n_elem + 1);

            for (n = 0; n < n_elem; n++)
                result [n] = g_strdup ((const gchar*) sqlite3_column_text (stmt, 0));

            result [n_elem] = NULL;
        }
    }

    sqlite3_finalize (stmt);

    if (n_tags)
      *n_tags = n_elem;

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint* service_database_tag_get_images  (const gchar *tag, guint *n_images)
{
    sqlite3_stmt *stmt = NULL;
    ServiceDatabasePrivate *priv;
    gint *values, n_elem = 0, n;

    if (tag == NULL)
        return NULL;

    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_TAG_GET_IMAGES, NULL, NULL))
    {
        sqlite_bind_text (stmt, "tag", NULL, tag, FALSE);
        while (sqlite3_step (stmt) == SQLITE_ROW) n_elem++;

        if (n_elem > 0)
        {
            values = g_new0 (gint, n_elem + 1);
            sqlite3_reset (stmt);
            sqlite_bind_text (stmt, "tag", NULL, tag, FALSE);

            for (n = 0; n < n_elem; n++)
                values [n] = sqlite3_column_int (stmt, 0);
        }
    }

    sqlite3_finalize (stmt);

    if (n_images)
        *n_images = n_elem;

    return values;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_tag_remove_image (gint image_id)
{
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt = NULL;
    gboolean result = FALSE;

    if (image_id < 1) return FALSE;

    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_TAG_REMOVE_IMAGE, NULL, NULL))
    {
        sqlite_bind_int (stmt, "image_id", 0, image_id, FALSE);
        result = sqlite3_step (stmt) == SQLITE_DONE;
    }

    sqlite3_finalize (stmt);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_tag_remove_tag (const gchar *tag)
{
    ServiceDatabasePrivate *priv;
    sqlite3_stmt *stmt = NULL;
    gboolean result = FALSE;

    if (image_id == NULL || strlen (tag) == 0) return FALSE;

    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_TAG_REMOVE_TAG, NULL, NULL))
    {
        sqlite_bind_text (stmt, "tag", NULL, tag, FALSE);
        result = sqlite3_step (stmt) == SQLITE_DONE;
    }

    sqlite3_finalize (stmt);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_tag_info_get (const gchar *tag, TagInfo **info, gboolean with_images)
{
    gboolean result = FALSE;
    sqlite3_stmt *stmt = NULL;
    ServiceDatabasePrivate *priv;

    if (tag == NULL) return FALSE;
    priv = DbPrivate ();

    if (service_prepare_stmt (priv->db, &stmt, STMT_TAG_GET, NULL, NULL))
    {
        sqlite_bind_text (stmt, "tag", NULL, tag, FALSE);
        result = sqlite3_step (stmt) == SQLITE_EOW;

        if (result && info)
        {
            TagInfo *dest = NULL;

            dest = tag_info_new ();
            dest->tag = g_strdup (tag);
            dest->tag_id = sqlite_column_text (stmt, "tag_id");
            dest->created = sqlite_column_text (stmt, "created");
            dest->modified = sqlite_column_text (stmt, "modified");

            if (with_images)
                dest->images = service_database_tag_get_images (tag, &dest->n_images);

            tag_info_free (*info);
            *info = dest;
            result = TRUE;
        }
    }

    sqlite3_finalize (stmt);
    
    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_tag_info_set (TagInfo *info)
{
    gboolean result = FALSE;
    gint n;

    if (info == NULL) return FALSE;
    if (info->tag == NULL || strlen (info->tag) == 0) return FALSE;
    if (info->n_images == 0 || info->images == NULL) return FALSE;

    for (n = 0; n < info->n_images; n++)
    {
        gint oid;
        oid = service_database_tag_new (info->images [n], info->tag);

        if (!result && oid > 0)
            result = TRUE;
    }

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// C O M M E N T S
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_comment_table_create  (void)
{
    return TRUE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint service_database_comment_new (gint image_id, const gchar *comment)
{
    return -1;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar* service_database_comment_by_image_id (gint image_id)
{
    return NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint64 service_database_comment_get_time (gint oid)
{
    return 0;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint service_database_comment_get_oid (gint image_id, const gchar *comment)
{
    return -1;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_comment_get_sync (gint oid)
{
    return TRUE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void service_database_comment_set_sync (gint oid, gboolean setting) {

}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_comment_set (gint oid, const gchar *comment)
{
    return TRUE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_comment_remove (gint oid)
{
    return TRUE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_database_comment_image_remove (gint image_id)
{
    return TRUE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

