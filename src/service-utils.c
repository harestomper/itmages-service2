/*
 *      service-utils.c
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


#include "service-utils.h"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_check_path (const gchar *path, gboolean create_parents)
{
    gboolean result = FALSE;

    result = g_file_test (path, G_FILE_TEST_EXISTS);

    if (!result && create_parents) {
        gchar *parent = g_path_get_dirname (path);
        result = g_mkdir_with_parents (parent, 0770) != -1;
    }

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar*  service_build_modified_filename (const gchar *orig_filename, const gchar *suffix)
{
    gchar *filename = NULL;
    gchar *norm_suffix;
    gchar *extension = NULL;
    gint idx = 0;

    if (orig_filename == NULL || strlen (orig_filename) == 0)
        return NULL;
        
    extension = g_strrstr (orig_filename, ".");

    if (extension == NULL || strlen (extension) == 0)
        extension = g_strdup ("");
    else
        extension = g_strdup_printf (".%s", extension);

    if (suffix == NULL || strlen (suffix) == 0)
        norm_suffix = g_strdup ("");
    else
        norm_suffix = g_strdup_printf ("-%s", suffix);

    filename = g_strdup_printf ("%s%s%s", orig_filename, norm_suffix, extension);

    while (service_check_path (filename, FALSE)) {
        g_free (filename);
        filename = g_strdup_printf ("%s%s-%i%s", orig_filename, norm_suffix, idx, extension);
        idx++;
    }

    if (extension)
        g_free (extension);

    if (norm_suffix)
        g_free (norm_suffix);

    return filename;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint64 service_get_current_time (void) {
    GDateTime *dt;
    gint64 result;

    dt = g_date_time_new_now_utc ();
    result = g_date_time_to_unix (dt);
    g_date_time_unref (dt);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------


