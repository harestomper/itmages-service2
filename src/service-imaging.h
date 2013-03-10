/*
 *      service-imaging.h
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

#ifndef _SERVICE_IMAGING_H_
#define _SERVICE_IMAGING_H_

#include "service-types.h"

G_BEGIN_DECLS


gboolean service_imaging_resize (const gchar *filein,
                                     const gchar *fileout,
                                     gint width,
                                     gint height,
                                     gboolean aspect);

gboolean service_imaging_thombnail (const gchar *filename,
                                     const gchar *thumb,
                                     gint width,
                                     gint height,
                                     gboolean cut_out);

gboolean service_imaging_rotate (const gchar *filein, const gchar *fileout, gint angle);

gboolean service_imaging_geometry (const gchar *filename,
                                     gint           *width,
                                     gint          *height,
                                     gint64          *size,
                                     gchar          **mime,
                                     gchar          **hash);

gboolean service_imaging_save_buffer (const gchar *filename, const gchar *filename_out);


G_END_DECLS

#endif /* _SERVICE_IMAGING_H_ */


