/*
 *      service-net-utils.h
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


#ifndef _SERVICE_NET_UTILS_H_
#define _SERVICE_NET_UTILS_H_


#include "service-types.h"

G_BEGIN_DECLS

//------------------------------------------------------------------------------

//typedef struct _ServiceResponse     ServiceResponse;

//------------------------------------------------------------------------------
/*
    The callback of this type should be returned the result for "cancelled"
    field of ServiceNetInfo structure. The TRUE if this task should be
    a cancelled or FALSE otherwise.
*/
typedef gboolean (*service_net_callback) (ServiceResponse *response);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _ServiceNetInfo {
    gint                    task_id;        // ID of a new process. If its is <= 0 then generated is automatically.
    gint                    task_type;      // The pointed to type of action.
    gint                    task_stage;

    gchar*                  filename;       // Filename of picture to upload  or URL of picture to download
    gchar*                  mimetype;       
    gchar*                  filename_out;   // Filename to the downloaded picture or NULL, to skip saving
    gchar*                  id;             // ID of the uploaded picture
    gchar*                  key;            // Key of the uploaded picture
    gint                    offset;         // Used on the receving albums
    GList*                  items;          // List of ServiceImageInfo for actions GET, SYNC
    
    gchar*                  username;       // Username of the accaount of the ITmages or NULL to work without authorization
    gchar*                  password;       // Password to authorization to ITmages or NULL to work without authorization
    gchar*                  proxynode;      // URL of the proxy server or NULL to work without proxy
    gint                    proxytype;      // CURL_PROXY_TYPE to set type of proxy server or <= -100 to disable proxy
    gint                    proxyport;      // Used port of the proxy server
    gchar*                  proxyuser;      // Authorization of the proxy server or NULL/ to skip authorization
    gchar*                  proxypass;      // Password to authoryzation on the proxy or NULL to skip password

    gint                    pipe_writer;    // Pipe channel to receiving result data from functions or <= 0
    GMutex*                 mutex_writer;   // GMutex to lock the use 'pipe_writer' from other threads
    gboolean                cancelled;      // Flag to stop NET function 
    pthread_t               thread_id;      // The thread identifier. In the funtions is not used.

    /*  Temporary data of the internal use */
    gchar*                  result_buffer;      // String buffer of the current receiving data
    gint                    result_len_buffer;  // Length of the result_buffer
    gint                    dn_total;           // Progress data
    gint                    dn_chunk;           // Progress data
    gint                    up_total;           // Progress data
    gint                    up_chunk;           // Progress data

    service_net_callback    start_function;         // Callback of the start tasks or NULL
    service_net_callback    end_function;           // Callback of the complete tasks or NULL
    service_net_callback    progress_function;      // Progress callback or NULL
    curl_write_callback     curl_write_function;    // Write function for libcurl or NULL to use default callback
    curl_progress_callback  curl_progress_function; // Progress function for libcurl or NULL to use default callback
    gpointer                user_data;             

};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _ServiceResponse {
    gboolean           status;
    gchar*             username;
    gchar*             error;
    gchar*             short_id;
    gchar*             id;
    GSList*            items;
    gint               n_items;
    gint               total_n_items;
    gint               total_per_page;

    gint               task_id;
    gint               task_type;
    gint               task_stage;
    gint               dn_total;
    gint               dn_chunk;
    gint               up_total;
    gint               up_chunk;

    gpointer           user_data;
};

GType               service_response_get_type       (void);
ServiceResponse*    service_response_new            (void);
ServiceResponse*    service_response_copy           (ServiceResponse *response);
void                service_response_free           (ServiceResponse *response);    
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/*
    The structure for sends and receives across of the pipe channel.
*/
struct _ServiceIOData {

    gint        task_id;
    gint        task_type;
    gint        task_stage;

    gchar*      result_data;
    gint        result_n_data;
    gchar*      result_mesg;
    gint        result_code;

    gint        dn_total;
    gint        dn_chunk;
    gint        up_total;
    gint        up_chunk;
    
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
enum {
    SERVICE_STAGE_NONE,
    SERVICE_STAGE_START,
    SERVICE_STAGE_STOP,
    SERVICE_STAGE_CANCELLED,
    SERVICE_STAGE_PROGRESS
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
enum {
    SERVICE_TASK_NONE,
    SERVICE_TASK_UPLOAD,
    SERVICE_TASK_DOWNLOAD,
    SERVICE_TASK_DELETE,
    SERVICE_TASK_SHORT,
    SERVICE_TASK_LOGIN,
    SERVICE_TASK_LOGOUT,
    SERVICE_TASK_CHECK,
    SERVICE_TASK_INFO,
    SERVICE_TASK_GET,
    SERVICE_TASK_SYNC,
    SERVICE_TASK_TAG,
    SERVICE_TASK_COMMENT,
    SERVICE_TASK_FEEDBACK,
    SERVICE_TASK_CONNECT
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean               service_net_login       (ServiceNetInfo *info);
gboolean               service_net_logout      (ServiceNetInfo *info);
gboolean               service_net_check       (ServiceNetInfo *info);
gboolean               service_net_upload      (ServiceNetInfo *info);
gboolean               service_net_delete      (ServiceNetInfo *info);
gboolean               service_net_download    (ServiceNetInfo *info);
gboolean               service_net_short       (ServiceNetInfo *info);
gboolean               service_net_get         (ServiceNetInfo *info);
gboolean               service_net_info        (ServiceNetInfo *info);
gboolean               service_net_status      (ServiceNetInfo *info);
gboolean               service_net_tag         (ServiceNetInfo *info);
gboolean               service_net_sync        (ServiceNetInfo *info);


G_END_DECLS

#endif /* _SERVICE_XML_UTILS_H_ */
