/*
 *      service-utils.h
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


#ifndef _SERVICE_UTILS_H_
#define _SERVICE_UTILS_H_

#include "service-types.h"

G_BEGIN_DECLS



gboolean  service_check_path (const gchar *path, gboolean create_parents);
gchar*    service_build_modified_filename (const gchar *orig_filename, const gchar *suffix);
gint64    service_get_current_time (void);

G_END_DECLS

#endif /* _SERVICE_UTILS_H_ */

