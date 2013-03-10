/*
 *      service-types.h
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


#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <glib-object.h>
#include <wand/MagickWand.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <curl/curl.h>

#ifndef __SERVICE_TYPES_H__
#define __SERVICE_TYPES_H__
G_BEGIN_DECLS

typedef struct  _ServiceResponse    ServiceResponse;
typedef struct  _ServiceNetInfo      ServiceNetInfo;
typedef struct  _ServiceIOData      ServiceIOData;

G_END_DECLS
#endif /* __SERVICE_TYPES_H__ */

#include "service-utils.h"
#include "service-imaging.h"
#include "service-net-utils.h"
#include "service-xml-utils.h"
#include "service-image-info.h"



