/*
 *      service-sql-statements.c
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
#include "service-sql-statements.h"

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint sqlite_column_index (sqlite3_stmt *stmt, const gchar *key)
{
    gint column_count, idx;

    column_count = sqlite3_column_count (stmt);

    for (idx = 0; idx < column_count; idx++) {
      const gchar *column_name;

      column_name = sqlite3_column_name (stmt, idx);

      if (g_strcmp0 (column_name, key) == 0)
          return idx;
    }

    return -1;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar *sqlite_column_text (sqlite3_stmt *stmt, const gchar *key)
{
    gint idx;

    idx = sqlite_column_index (stmt, key);

    if (idx >= 0) {
      const guchar *temp;
      gchar *result = NULL;

      temp = sqlite3_column_text (stmt, idx);

      if (temp)
          result = g_strdup ((const gchar *) temp);

      return result;

    } else {
      return NULL;
    }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint sqlite_column_int (sqlite3_stmt *stmt, const gchar *key)
{
    gint idx;

    idx = sqlite_column_index (stmt, key);

    if (idx >= 0)
      return sqlite3_column_int (stmt, idx);
    else 
      return 0;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint64 sqlite_column_int64 (sqlite3_stmt *stmt, const gchar *key)
{
    gint idx;

    idx = sqlite_column_index (stmt, key);

    if (idx >= 0)
      return sqlite3_column_int64 (stmt, idx);
    else 
      return 0;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
double sqlite_column_double (sqlite3_stmt *stmt, const gchar *key)
{
    gint idx;

    idx = sqlite_column_index (stmt, key);

    if (idx >= 0)
      return sqlite3_column_double (stmt, idx);
    else 
      return 0.0;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void sqlite_bind_text (sqlite3_stmt *stmt,
                              const gchar *key,
                              const gchar *saved_value,
                              const gchar *new_value,
                              gboolean change_if_null)
{
    gint idx;
    gchar *value;
    gchar *temp;

    temp = g_strdup_printf (":%s", key);
    idx = sqlite3_bind_parameter_index (stmt, temp);
    g_free (temp);

    if (idx > 0)
        if ((!change_if_null || saved_value == NULL) && g_strcmp0 (new_value, saved_value) != 0)
            value = g_strdup (new_value);
        else
            value = g_strdup (saved_value);

        if (value)
          sqlite3_bind_text (stmt, idx, value, -1, g_free);
        else
          salite3_bind_null (stmt, idx);
    }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void sqlite_bind_int (sqlite3_stmt *stmt,
                             const gchar *key,
                             gint saved_value,
                             gint new_value,
                             gboolean change_if_null)
{
    gint idx;
    gchar *temp;

    temp = g_strdup_printf (":%s", key);
    idx = sqlite3_bind_parameter_index (stmt, temp);
    g_free (temp);

    if (idx > 0)
        sqlite3_bind_int (stmt, idx,
            ((!change_if_null || saved_value == 0) && new_value != saved_value) ? new_value : saved_value);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void sqlite_bind_int64 (sqlite3_stmt *stmt,
                               const gchar *key,
                               gint64 saved_value,
                               gint64 new_value,
                               gboolean change_if_null)
{
    gint idx;
    gchar *temp;

    temp = g_strdup_printf (":%s", key);
    idx = sqlite3_bind_parameter_index (stmt, temp);
    g_free (temp);

    if (idx > 0)
        sqlite3_bind_int64 (stmt, idx,
          ((!change_if_null || saved_value == 0) && new_value != saved_value) ? new_value : saved_value);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void sqlite_bind_double (sqlite3_stmt *stmt,
                                const gchar *key,
                                gdouble saved_value,
                                gdouble new_value,
                                gboolean change_if_null)
{
    gint idx;
    gchar *temp;

    temp = g_strdup_printf (":%s", key);
    idx = sqlite3_bind_parameter_index (stmt, temp);
    g_free (temp);

    if (idx > 0)
        sqlite3_bind_double (stmt, idx,
          ((!change_if_null || saved_value == 0.0) && new_value != saved_value) ? new_value : saved_value);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean  service_prepare_stmt    (sqlite3            *db,
                                   sqlite3_stmt    **stmt,
                                   const gchar   *request,
                                   GMutex          *mutex,
                                   const gchar *location)
{
    gboolean result = FALSE;

    if (stmt)
        result = sqlite3_prepare_v2 (db, request, -1, stmt, NULL) == SQLITE_OK;
    else {
        gint code;
        code = sqlite3_exec (db, request, -1, NULL, NULL, NULL);
        result = code == SQLITE_DONE || code == SQLITE_OK;
    }

    if (!result)
    {
        if (location)
            g_error ("%s: %i, %s\n%s", location, sqlite3_errcode (db), sqlite3_errmsg (db), request);
        else
            g_debug ("%i: %s\n%s", sqlite3_errcode (db), sqlite3_errmsg (db), request);
    }

    if (mutex)
        g_mutex_init (mutex);
        
    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

