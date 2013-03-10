/*
 *      service-sql-statements.h
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

#ifndef _SERVICE_SQL_STATEMENTS_H_
#define _SERVICE_SQL_STATEMENTS_H_

#include <glib.h>
#include <stdio.h>
#include <sqlite3.h>

G_BEGIN_DECLS

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define SD_MAIN_TABLE       "images"
#define SD_ALBUM_TABLE      "albums"
#define SD_TAGS_TABLE       "tags"
#define SD_COMMENTS_TABLE   "comments"


/*------------------------------------------------------------------------------
 * I M A G E   I N F O
 */
#define STMT_INFO_CREATE           "CREATE TABLE IF NOT EXISTS " \
                                   SD_MAIN_TABLE                \
                                   "(oid INTEGER PRIMARY KEY," \
                                   "id TEXT,"                  \
                                   "key TEXT,"                 \
                                   "full TEXT,"                \
                                   "small TEXT,"               \
                                   "server TEXT,"              \
                                   "short_id TEXT,"            \
                                   "filename TEXT,"            \
                                   "mime TEXT,"                \
                                   "appname TEXT,"             \
                                   "hash TEXT,"                \
                                   "local_user TEXT,"          \
                                   "hosting TEXT,"             \
                                   "device TEXT,"              \
                                   "username TEXT,"            \
                                   "album INTEGER,"            \
                                   "album_ex INTEGER,"         \
                                   "width INTEGER,"            \
                                   "height INTEGER,"           \
                                   "size INTEGER,"             \
                                   "created INTEGER,"          \
                                   "modified INTEGER,"         \
                                   "user_data1 LONGVARCHAR,"   \
                                   "user_data2 LONGVARCHAR,"   \
                                   "user_data3 LONGVARCHAR,"   \
                                   "user_data4 LONGVARCHAR)"   \
                        
#define STMT_INFO_OID_BY_HASH     "SELECT oid FROM "SD_MAIN_TABLE" WHERE hash == :hash LIMIT 1"
#define STMT_INFO_NEW_HASH        "INSERT INTO "SD_MAIN_TABLE" (hash, created) VALUES (:hash, strftime ('%s', 'now', 'utc'))"
#define STMT_INFO_REMOVE          "DELETE FROM "SD_MAIN_TABLE" WHERE oid == :oid"
#define STMT_INFO_GET_OID_LIST    "SELECT oid FROM "SD_MAIN_TABLE
#define STMT_INFO_SET             "UPDATE "SD_MAIN_TABLE" SET " \
                                  "id = :id,"                   \
                                  "key = :key,"                 \
                                  "full = :full,"               \
                                  "small = :small,"             \
                                  "server = :sercer,"           \
                                  "short_id = :short_id,"       \
                                  "filename = :filename,"       \
                                  "mime = :mime,"               \
                                  "appname = :appname,"         \
                                  "hash = :hash,"               \
                                  "local_user = :local_user,"   \
                                  "hosting = :hosting,"         \
                                  "device = :device,"           \
                                  "username = :username,"       \
                                  "album = :album,"             \
                                  "album_ex = :album_ex,"       \
                                  "width = :width,"             \
                                  "height = :height,"           \
                                  "size = :size,"               \
                                  "utime = :utime "             \
                                  " WHERE oid == :oid"
#define STMT_INFO_SET_ALBUM       "UPDATE "SD_INFO_TABLE" SET album = :new_album WHERE album == :album_id"
/*-----------------------------------------------------------------------------
 *  A L B U M S
 */
#define STMT_ALBUM_CRESTE          "CREATE TABLE IF NOT EXISTS "     \
                                   SD_ALBUM_TABLE                    \
                                   " ("                              \
                                   "album_id INTEGER PRIMARY KEY, "  \
                                   "album_ex INTEGER, "              \
                                   "albumname TEXT UNIQUE NOT NULL," \
                                   "description TEXT, "              \
                                   "cover_id TEXT, "                 \
                                   "created INTEGER, "               \
                                   "modified INTEGER, "              \
                                   "synchronized INTEGER "           \
                                   ")"

#define STMT_ALBUM_NEW            "INSERT INTO "SD_ALBUM_TABLE      \
                                  "(albumname, created) "           \
                                  "VALUES (:albumname, strftime ('%s', 'now', 'utc'))"
                                  
#define STMT_ALBUM_GET_OID        "SELECT album_id FROM "SD_ALBUM_TABLE" WHERE albumname == :albumname"
#define STMT_ALBUM_GET_IMAGES     "SELECT album_id FROM "SD_INFO_TABLE" WHERE album == :album_id"
#define STMT_ALBUM_GET            "SELECT "                         \
                                  "album_id"                        \
                                  ", album_ex"                      \
                                  ", albumname"                     \
                                  ", description"                   \
                                  ", cover_id"                      \
                                  ", created"                       \ 
                                  ", modified"                      \
                                  ", synchronized"                  \
                                  " FROM "SD_ALBUM_TABLE            \
                                  " WHERE album_id == :album_id"

#define STMT_ALBUM_SET            "UPDATE "SD_ALBUM_TABLE" SET "    \
                                  "album_ex = :album_ex"            \
                                  ", albumname = :albumname"        \
                                  ", cover_id = :cover_id"          \
                                  ", description = :description"    \
                                  ", modified = :modified"          \
                                  ", synchronized = :synchronized"  \
                                  " WHERE album_id == :album_id"
                                  
#define STMT_ALBUM_OID_FROM_NAME  "SELECT album_id FROM "SD_ALBUM_TABLE" WHERE albumname == :albumname"
#define STMT_ALBUM_OID_FROM_SOID  "SELECT album_id FROM "SD_ALBUM_TABLE" WHERE album_ex == :album_ex"
#define STMT_ALBUM_NAME_FROM_SOID "SELECT albumname FROM "SD_ALBUM_TABLE" WHERE album_ex == :album_ex"
#define STMT_ALBUM_REMOVE          "DELETE FROM "SD_ALBUM_TABLE" WHERE album_id == :album_id"

/*------------------------------------------------------------------------------
 * T A G S
 */
#define STMT_TAGS_CREATE           "CREATE TABLE IF NOT EXISTS "  \
                                   SD_TAGS_TABLE                  \
                                   " ( "                          \
                                   "tag_id INTEGER PRIMARY KEY"   \
                                   ", image_id INTEGER"           \
                                   ", tag TEXT NOT NULL"          \
                                   ", created INTEGER"            \
                                   ", modified INTEGER"           \
                                   ", synchronized INTEGER"       \
                                   ")"
#define STMT_TAG_NEW               "INSERT INTO "SD_TAGS_TABLE" (image_id, tag) VALUES (:image_id, :tag)"
#define STMT_TAG_GET_IMAGES        "SELECT image_id FROM "SD_TAGS_TABLE" WHERE tag LIKE ':tag'"
#define STMT_TAG_HAS_TAG           "SELECT count (image_id) FROM "SD_TAGS_TABLE" WHERE image_id == :image_id AND tag == :tag"
#define STMT_TAG_GET_LIST          "SELECT DISTINCT tag FROM "SD_TAGS_TABLE
#define STMT_TAG_LIST_OF_IMAGE     "SELECT DISTINCT tag FROM "SD_TAGS_TABLE" WHERE image_id == :image_id"
#define STMT_TAG_REMOVE_IMAGE      "DELETE FROM "SD_TAGS_TABLE" WHERE image_id == :image_id"
#define STMT_TAG_REMOVE_TAG        "DELETE FROM "SD_TAGS_TABLE" WHERE tag LIKE :tag"
#define STMT_TAG_GET               "SELECT tag_id, image_id, created, modified, synchronized FROM "SD_TAGS_TABLE" WHERE modified == max (modified) AND tag == :tag LIMIT 1"


/*------------------------------------------------------------------------------
 * C O M M E N T A R I E S
 */
#define STMT_CREATE_COMMENTS       "CREATE TABLE IF NOT EXISTS " \
                                   SD_COMMETS_TABLE              \
                                   " ( "                         \
                                   "oid INTEGER PRIMARY KEY, "   \
                                   "image_id INTEGER, "          \
                                   "comment TEXT NOT NULL, "     \
                                   "created INTEGER, "           \
                                   "synchronized INTEGER "       \
                                   " )"

#define STMT_COMMENT_GET_BY_IMID   "SELECT comment FROM "SD_COMMENTS_TABLE" WHERE image_id == ?"
#define STMT_COMMENT_GET_TIME      "SELECT created FROM "SD_COMMENTS_TABLE" WHERE oid == ?"
#define STMT_COMMENT_GET_OID       "SELECT oid FROM "SD_COMMENTS_TABLE" WHERE image_id == ? AND comment == ?"
#define STMT_COMMENT_GET_SYNC      "SELECT synchronized FROM "SD_COMMENTS_TABLE" WHERE oid == ?"
#define STMT_COMMENT_SET           "UPDATE "SD_COMMENTS_TABLE" SET comment = :comment WHERE oid == :oid"
#define STMT_COMMENT_SET_SYNC      "UPDATE "SD_COMMENTS_TABLE" SET synchronized = :synchronized WHERE oid == :oid"
#define STMT_COMMENT_REMOVE        "DELETE FROM "SD_COMMENTS_TABLE" WHERE oid == :oid"
#define STMT_COMMENT_REMOVE_ALL    "DELETE FROM "SD_COMMENTS_TABLE" WHERE image_id == :image_id"

#define STMT_COMMENT_INSERT        "INSERT INTO "SD_COMMENTS_TABLE                \
                                   " (image_id, comment, created, synchronized)"  \
                                   " VALUES (:image_id, :comment, :created, :synchronized)"

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean  service_prepare_stmt    (sqlite3            *db,
                                   sqlite3_stmt    **stmt,
                                   const gchar   *request,
                                   GMutex          *mutex,
                                   const gchar *location);

gint      sqlite_column_index     (sqlite3_stmt *stmt, const gchar *key);
gchar*    sqlite_column_text      (sqlite3_stmt *stmt, const gchar *key);
gint      sqlite_column_int       (sqlite3_stmt *stmt, const gchar *key);
gint64    sqlite_column_int64     (sqlite3_stmt *stmt, const gchar *key);
double    sqlite_column_double    (sqlite3_stmt *stmt, const gchar *key);

void      sqlite_bind_text        (sqlite3_stmt       *stmt,
                                   const gchar         *key,
                                   const gchar *saved_value,
                                   const gchar   *new_value,
                                   gboolean change_if_null);

void      sqlite_bind_int         (sqlite3_stmt       *stmt,
                                   const gchar         *key,
                                   gint         saved_value,
                                   gint           new_value,
                                   gboolean change_if_null);

void      sqlite_bind_int64       (sqlite3_stmt       *stmt,
                                   const gchar         *key,
                                   gint64       saved_value,
                                   gint64         new_value,
                                   gboolean change_if_null);

void      sqlite_bind_double      (sqlite3_stmt       *stmt,
                                   const gchar         *key,
                                   gdouble      saved_value,
                                   gdouble        new_value,
                                   gboolean change_if_null);

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------


G_END_DECLS

#endif /* _SERVICE_SQL_STATEMENTS_H_ */

