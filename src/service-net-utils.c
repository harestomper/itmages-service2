/*
 *      service-net-utils.c
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


#include "service-net-utils.h"



#define SERVICE_NET_USERAGENT       "ITmages upload service (C, rev 2.0.1)"
#define SERVICE_NET_API_URL         "http://itmages.ru/api/v2/%s"
#define SERVICE_NET_COOKIE_PROTO    "%s@itmages.txt"
#define SERVICE_NET_COOKIE_DIR      "itmagesd"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static GHashTable *cookies_table = NULL;
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static size_t service_net_default_write_function (char *buffer, size_t size, size_t nitems, void *user_data);
static void service_net_send_back_message (ServiceNetInfo *info, ServiceResponse **response_out);
static gint service_net_default_progress_function (void *user_data, double dt, double dr, double ut, double ur);
static CURL* service_net_new_handle (ServiceNetInfo *info);
static void service_net_set_cookes (CURL *curl, const gchar* username);
static void service_net_save_cookes (void);
static gchar* service_net_build_cookie_name (const gchar* username);
static void service_net_perform_connecton (CURL *curl, ServiceNetInfo *info, ServiceResponse **response);
static void service_net_set_fake_message (ServiceNetInfo *info, const gchar* message);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServiceResponse* service_response_new (void)
{
    ServiceResponse *response;
    response = g_slice_new0 (ServiceResponse);

    return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServiceResponse* service_response_copy (ServiceResponse *response)
{
    ServiceResponse *result = NULL;

    if (response != NULL)
    {
        result = service_response_new ();

        result->status = response->status;
        result->username = g_strdup (response->username);
        result->error = g_strdup (response->error);
        result->short_id = g_strdup (response->short_id);
        result->id = g_strdup (response->id);
        result->n_items = response->n_items;
        result->total_n_items = response->total_n_items;
        result->total_per_page = response->total_per_page;

        result->task_id = response->task_id;
        result->task_type = response->task_type;
        result->dn_total = response->dn_total;
        result->dn_chunk = response->dn_chunk;
        result->up_total = response->up_total;
        result->up_chunk = response->up_chunk;

        if (result->n_items > 0)
        {
            GSList *node;

            for (node = response->items; node; node = node->next)
            {
                ServiceImageInfo *dest, *src;

                src = node->data;
                dest = service_image_info_copy (src);

                if (dest != NULL)
                    response->items = g_slist_append (response->items, dest);
            }
        }
    }

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void service_response_free (ServiceResponse *response)
{
    if (response == NULL)
        return;

    g_free (response->username);
    g_free (response->error);
    g_free (response->short_id);
    g_free (response->id);

    if (response->items)
        g_slist_free_full (response->items, (GDestroyNotify) service_image_info_free); 
        
    g_slice_free (ServiceResponse, response);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_net_send_back_message (ServiceNetInfo *info, ServiceResponse **response_out)
{
    ServiceIOData io = {0};
    ServiceResponse *response = NULL;
    service_net_callback result_function = NULL;
    gboolean callback_result = FALSE;

    switch (info->task_stage)
    {
        case SERVICE_STAGE_START:
            result_function = info->start_function;
            break;

        case SERVICE_STAGE_STOP:
            if (info->result_buffer || info->task_type == SERVICE_TASK_DOWNLOAD) {

                if (info->task_type == SERVICE_TASK_DOWNLOAD) {
                    response = service_response_new ();
                    response->status = service_imaging_save_buffer (
                                                        info->filename,
                                                        info->filename_out);
                    if (!response->status && info->result_buffer) {
                        service_response_free (response);
                        response = service_xml_get_response (info->result_buffer);
                    }
                } else {
                    response = service_xml_get_response (info->result_buffer);
                }

            } else {
                response = service_response_new ();
                response->status = FALSE;
                response->error = g_strdup ("Buffer of data is empty");
            }

        case SERVICE_STAGE_CANCELLED:
            result_function = info->end_function;
            break;

        case SERVICE_STAGE_PROGRESS:
            result_function = info->progress_function;
            break;

        default:
            return;
    }
    
    if (response == NULL) {
        response = service_response_new ();
        response->status = TRUE;
    }

//    io.status = response->status;
    io.task_id = response->task_id = info->task_id;
    io.task_type = response->task_type = info->task_type;
    io.task_stage = response->task_stage = info->task_stage;
    io.dn_chunk = response->dn_chunk = info->dn_chunk;
    io.dn_total = response->dn_total = info->dn_total;
    io.up_chunk = response->up_chunk = info->up_chunk;
    io.up_total = response->up_total = info->up_total;
    io.result_mesg = response->error;
    io.result_data = info->result_buffer;
    io.result_n_data = info->result_len_buffer;

    if (result_function)
        callback_result = result_function (response);

    if (!info->cancelled)
        info->cancelled = callback_result;

    if (info->mutex_writer && info->pipe_writer) {
        g_mutex_lock (info->mutex_writer);
        write (info->pipe_writer, &io, sizeof (ServiceIOData));
        g_mutex_unlock (info->mutex_writer);
    }

printf ("%s\n", G_STRLOC);
    if (response_out) {
printf ("%s\n", G_STRLOC);
        *response_out = response;
    } else {
printf ("%s\n", G_STRLOC);
        service_response_free (response);
    }
printf ("%s\n", G_STRLOC);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static size_t service_net_default_write_function (char *buffer, size_t size, size_t nitems, void *user_data)
{
    GString *string;
    ServiceNetInfo *info = (ServiceNetInfo *) user_data;

    string = g_string_new (info->result_buffer ? info->result_buffer : "");
    g_string_append_len (string, (const gchar *) buffer, (gssize) nitems);

    if (info->result_buffer)
        g_free (info->result_buffer);

    info->result_buffer = g_strdup (string->str);
    info->result_len_buffer = string->len;
    printf ("%s: %s\n", G_STRLOC, string->str);
    g_string_free (string, TRUE);

    return (size_t) info->result_len_buffer;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gint service_net_default_progress_function (void *user_data, double dt, double dr, double ut, double ur)
{
    ServiceNetInfo *info = (ServiceNetInfo *) user_data;

    info->dn_total = (gint) dt;
    info->dn_chunk = (gint) dr;
    info->up_total = (gint) ut;
    info->up_chunk = (gint) ur;
    info->task_stage = SERVICE_STAGE_PROGRESS;
    service_net_send_back_message (info, NULL);

    return info->cancelled;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static CURL* service_net_new_handle (ServiceNetInfo *info)
{
    CURL *curl;
    curl_progress_callback progress_function = service_net_default_progress_function;
    curl_write_callback write_function = service_net_default_write_function;
    gchar *url;
    const gchar *suffix = NULL;
    gchar* postfields = NULL;

    if (info->result_buffer) {
        g_free (info->result_buffer);
        info->result_buffer = NULL;
        info->result_len_buffer = 0;
    }

    if (info->task_id <= 0)
        info->task_id = (gint) info;
        
    if (info->curl_progress_function != NULL)
        progress_function = info->curl_progress_function;

    if (info->curl_write_function != NULL)
        write_function = info->curl_write_function;
        
    curl = curl_easy_init ();

    curl_easy_setopt (curl, CURLOPT_NOSIGNAL, TRUE);
    curl_easy_setopt (curl, CURLOPT_NOPROGRESS, FALSE);
    curl_easy_setopt (curl, CURLOPT_VERBOSE, FALSE);
    curl_easy_setopt (curl, CURLOPT_PROGRESSDATA, info);
    curl_easy_setopt (curl, CURLOPT_WRITEDATA, info);
    curl_easy_setopt (curl, CURLOPT_USERAGENT, SERVICE_NET_USERAGENT);
    curl_easy_setopt (curl, CURLOPT_PROGRESSFUNCTION, progress_function);
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, write_function);

    if (info->proxytype > -1 && info->proxynode)
    {
        curl_easy_setopt (curl, CURLOPT_PROXY, info->proxynode);
        curl_easy_setopt (curl, CURLOPT_PROXYPORT, info->proxyport);
        curl_easy_setopt (curl, CURLOPT_PROXYTYPE, info->proxytype);

        if (info->proxyuser) {
            curl_easy_setopt (curl, CURLOPT_PROXYUSERNAME, info->proxyuser);
            curl_easy_setopt (curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);

            if (info->proxypass)
                curl_easy_setopt (curl, CURLOPT_PROXYPASSWORD, info->proxypass);
        }

        if (info->proxypass && info->proxyuser) {
            gchar *userpwd;
            userpwd = g_strdup_printf ("%s:%s", info->proxyuser, info->proxypass);
            curl_easy_setopt (curl, CURLOPT_PROXYUSERPWD, userpwd);
            g_free (userpwd);
        }
    }

    switch (info->task_type)
    {
        case SERVICE_TASK_CHECK:
            suffix = "check";
            postfields = g_strdup_printf ("username=%s", info->username);
            break;

        case SERVICE_TASK_LOGIN:
            suffix = "login";
            postfields = g_strdup_printf ("username=%s&password=%s",
                                         info->username, info->password);
            break;

        case SERVICE_TASK_LOGOUT:
            suffix = "logout";
            break;

        case SERVICE_TASK_UPLOAD:
            suffix = "add";
            break;
            
        case SERVICE_TASK_DELETE:
            suffix = "delete";
            postfields = g_strdup_printf ("id=%s&key=%s&username=%s",
                                        info->id, info->key, info->username);
            break;
        case SERVICE_TASK_SHORT:
            suffix = "short";
            postfields = g_strdup_printf ("id=%s&key=%s", info->id, info->key);
            break;
            
        case SERVICE_TASK_INFO:
            suffix = "info";
            postfields = g_strdup_printf ("id=%s&key=%s&type=picture",
                                          info->id, info->key);
            break;

        case SERVICE_TASK_GET:
        case SERVICE_TASK_SYNC:
            suffix = "get";
            postfields = g_strdup_printf ("offset=%i&type=picture", info->offset);
            break;
            
        case SERVICE_TASK_TAG:
        case SERVICE_TASK_CONNECT:
        case SERVICE_TASK_COMMENT:
        case SERVICE_TASK_FEEDBACK:
        case SERVICE_TASK_DOWNLOAD:
        default: break;
    }

    if (suffix)
    {
        url = g_strdup_printf (SERVICE_NET_API_URL, suffix);
        curl_easy_setopt (curl, CURLOPT_URL, url);
//        g_free (url);
    }

    if (postfields)
    {
        curl_easy_setopt (curl, CURLOPT_POST, TRUE);
        curl_easy_setopt (curl, CURLOPT_POSTFIELDS, postfields);
//        g_free (postfields);
    }
    
    if (info->username)
        service_net_set_cookes (curl, info->username);
    
    return curl;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gchar* service_net_build_cookie_name (const gchar* username)
{
    gchar* cookie_folder;
    gchar* cookie_filename;
    gchar* cookie_name;

    cookie_folder = g_build_filename (G_DIR_SEPARATOR_S,
                                      g_get_user_config_dir (),
                                      SERVICE_NET_COOKIE_DIR,
                                      NULL);

    if (!g_file_test (cookie_folder, G_FILE_TEST_EXISTS))
        g_mkdir_with_parents (cookie_folder, 0750);

    cookie_name  = g_strdup_printf (SERVICE_NET_COOKIE_PROTO, username);
    cookie_filename = g_build_filename (G_DIR_SEPARATOR_S,
                                        cookie_folder,
                                        cookie_name,
                                        NULL);

    g_free (cookie_folder);
    g_free (cookie_name);

    return cookie_filename;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_net_set_cookes (CURL *curl, const gchar* username)
{
    gchar* cookie_filename;

    cookie_filename = service_net_build_cookie_name (username);
    curl_easy_setopt (curl, CURLOPT_COOKIEFILE, cookie_filename);
    curl_easy_setopt (curl, CURLOPT_COOKIEJAR, cookie_filename);

    g_free (cookie_filename);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_net_save_cookes (void)
{
    if (cookies_table == NULL)
        return;

    GList *keys, *node;

    keys = g_hash_table_get_keys (cookies_table);

    for (node = keys; node; node = node->next)
    {
        gchar *filename;
        const gchar *username;
        const gchar *contents;

        username = node->data;
        filename = service_net_build_cookie_name (username);
        contents = g_hash_table_lookup (cookies_table, username);

        if (filename && contents)
            g_file_set_contents (filename, contents, -1, NULL);

        g_free (filename);
    }

    if (keys)
        g_list_free (keys);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_net_perform_connecton (CURL *curl, ServiceNetInfo *info, ServiceResponse **response)
{
    CURLcode result_code;
    info->task_stage = SERVICE_STAGE_START;
    service_net_send_back_message (info, NULL);
    result_code = curl_easy_perform (curl);

printf ("%s: %i: %s\n", G_STRLOC, (int) result_code, curl_easy_strerror (result_code));
    if (result_code != 0 && info->result_buffer == NULL)
    {
        const gchar* message;
printf ("%s\n", G_STRLOC);
        message = curl_easy_strerror (result_code);
        service_net_set_fake_message (info, message);
    }

    info->task_stage = SERVICE_STAGE_STOP;
    service_net_send_back_message (info, response);
    curl_easy_cleanup (curl);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void service_net_set_fake_message (ServiceNetInfo *info, const gchar* message)
{

    if (message == NULL)
        return;
    
    if (info->result_buffer)
    {
        g_free (info->result_buffer);
        info->result_buffer = NULL;
        info->result_len_buffer = 0;
    }

    info->result_buffer = g_strdup_printf ("%s%s%s%s%s",
                                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n",
                                "<response status=\"fail\" user=\"Guest\">\n",
                                "<error>", message,
                                "</error>\n</response>");

    info->result_len_buffer = strlen (info->result_buffer);

}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_login (ServiceNetInfo *info)
{   
    CURL *curl;
    ServiceResponse *response = NULL;
    gboolean result = FALSE;

    if (service_net_check (info))
        return TRUE;

    info->task_type = SERVICE_TASK_LOGIN;
    curl = service_net_new_handle (info);
    service_net_perform_connecton (curl, info, &response);

    result = response->status && g_strcmp0 (info->username, response->username) == 0;
    service_response_free (response);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_logout (ServiceNetInfo *info)
{
    CURL *curl;
    ServiceResponse *response = NULL;
    gboolean result = FALSE;

    if (!service_net_check (info))
        return TRUE;
        
    info->task_type = SERVICE_TASK_LOGOUT;
    curl = service_net_new_handle (info);
    service_net_perform_connecton (curl, info, &response);

    result = response->status;

    service_response_free (response);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_check (ServiceNetInfo *info)
{
    CURL *curl;
    ServiceResponse *response = NULL;
    gboolean result = FALSE;

    info->task_type = SERVICE_TASK_CHECK;
    curl = service_net_new_handle (info);
    service_net_perform_connecton (curl, info, &response);

    result = response->status && g_strcmp0 (info->username, response->username) == 0;
    service_response_free (response);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_upload (ServiceNetInfo *info)
{
    CURL *curl;
    ServiceResponse *response = NULL;
    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;
    gchar *basename, *mime = NULL;
    gint64 size;
    gint width, height;
    gboolean result = FALSE;

    if (!info->filename || strlen (info->filename) == 0) {
        service_net_set_fake_message (info, "Filename is not specified");
        info->task_stage = SERVICE_STAGE_STOP;
        service_net_send_back_message (info, NULL);
        return FALSE;

    } else if (info->username && info->password && !service_net_login (info)) {
        return FALSE;

    } else if (!service_imaging_geometry (info->filename, &width, &height, &size, &mime, NULL)) {
        gchar* message;
        message = g_strdup_printf ("File '%s' is not validated of the size or/and mime type", info->filename);
        service_net_set_fake_message (info, message);
        info->task_stage = SERVICE_STAGE_STOP;
        service_net_send_back_message (info, NULL);

        g_free (message);
        
        return FALSE;
    }

    info->task_type = SERVICE_TASK_UPLOAD;
    basename = g_path_get_basename (info->filename);

    curl_formadd (&post, &last, CURLFORM_CONTENTTYPE, mime, CURLFORM_END);
    curl_formadd (&post, &last, CURLFORM_FILENAME, basename, CURLFORM_END);
    curl_formadd (&post, &last, CURLFORM_CONTENTSLENGTH, size, CURLFORM_END);
    curl_formadd (&post, &last, CURLFORM_COPYNAME, "UFileManager[picture]",
                                CURLFORM_COPYCONTENTS, basename,
                                CURLFORM_END);
    curl_formadd (&post, &last, CURLFORM_COPYNAME, "UFileManager[picture]",
                                CURLFORM_FILE, info->filename,
                                CURLFORM_END);

    curl = service_net_new_handle (info);
    curl_easy_setopt (curl, CURLOPT_HTTPPOST, post);
    service_net_perform_connecton (curl, info, &response);

    result = response->status;

    service_response_free (response);
    g_free (basename);
//    g_free (mime);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
typedef struct {
    int file;
} UserPtr;
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static size_t download_write_function (char *buffer, size_t size, size_t n_items, void *userp)
{
    UserPtr *ptr = (UserPtr *) userp;
    size_t writelen;

    writelen = write (ptr->file, buffer, n_items);

    return writelen;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_download (ServiceNetInfo *info)
{
    CURL *curl;
    ServiceResponse *response = NULL;
    gboolean result = FALSE;
    UserPtr ptr = {0};
    gchar *temp_filename, *filename;

    if (info->filename == NULL)
    {
        service_net_set_fake_message (info, "URL of image to download is not specified");
        info->task_stage = SERVICE_STAGE_STOP;
        service_net_send_back_message (info, NULL);
        return FALSE;
    }

    temp_filename = g_strdup ("/tmp/itmages-service-loaded-XXXXXX");
    ptr.file = g_mkstemp (temp_filename);

    info->curl_write_function = NULL;
    info->task_type = SERVICE_TASK_DOWNLOAD;

    curl = service_net_new_handle (info);

    curl_easy_setopt (curl, CURLOPT_URL, info->filename);
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, download_write_function);
    curl_easy_setopt (curl, CURLOPT_WRITEDATA, &ptr);

    filename = info->filename;
    info->filename = temp_filename;
    
    service_net_perform_connecton (curl, info, &response);

    result = response->status;
    service_response_free (response);
    close (ptr.file);
    g_remove (temp_filename);

    info->filename = filename;
    g_free (temp_filename);
    
    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_short (ServiceNetInfo *info)
{
    CURL *curl;
    ServiceResponse *response = NULL;
    gboolean result = FALSE;

    if (info->id == NULL || info->key == NULL)
    {
        service_net_set_fake_message (info, "ID and/or KEY of the image is not specified");
        info->task_stage = SERVICE_STAGE_STOP;
        service_net_send_back_message (info, NULL);
        return FALSE;
    }

    info->task_type = SERVICE_TASK_SHORT;
    curl = service_net_new_handle (info);
    service_net_perform_connecton (curl, info, &response);
    result = response->status;
    service_response_free (response);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static ServiceResponse *_service_net_get (ServiceNetInfo *info)
{
    CURL *curl;
    ServiceResponse *response = NULL;

    info->task_type = SERVICE_TASK_GET;
    curl = service_net_new_handle (info);
    service_net_perform_connecton (curl, info, &response);

    return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_get (ServiceNetInfo *info)
{
    gboolean result = FALSE;

    if (info->username == NULL || info->password == NULL)
    {
        service_net_set_fake_message (info, "Unknown username or password");
        info->task_stage = SERVICE_STAGE_STOP;
        service_net_send_back_message (info, NULL);
        return FALSE;
    }

    if (service_net_login (info))
    {
        ServiceResponse *response = NULL;
        gint offset;

        offset = info->offset;

        if (info->offset < 0)
            info->offset = 0;
            
        while (response->status) {

printf ("%s\n", G_STRLOC);
            if (offset < 0)
                info->offset = response->n_items;

            if (response) {
                ServiceResponse *temp_response;

printf ("%s\n", G_STRLOC);
                temp_response = _service_net_get (info);
printf ("%s\n", G_STRLOC);

                if (temp_response->status) {
                    GSList *node;

                    for (node = temp_response->items; node; node = node->next)
                    {
                        ServiceImageInfo *info_src, *info_dest;

                        info_src = node->data;
                        info_dest = service_image_info_copy (info_src);
                        response->items = g_slist_append (response->items, info_dest);
                    }

                    response->n_items += temp_response->n_items;
                } else {
                    response->status = FALSE;
                }

printf ("%s\n", G_STRLOC);
                service_response_free (temp_response);
printf ("%s\n", G_STRLOC);
                
            } else {
printf ("%s\n", G_STRLOC);
                response = _service_net_get (info);
printf ("%s\n", G_STRLOC);
            }

            if (response->n_items >= response->total_n_items || offset >= 0)
                break;
        }

printf ("%s\n", G_STRLOC);
        service_net_send_back_message (info, &response);
printf ("%s\n", G_STRLOC);
    }
    
    return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_info (ServiceNetInfo *info)
{
    return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_status (ServiceNetInfo *info)
{
    return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_tag (ServiceNetInfo *info)
{
    return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_sync (ServiceNetInfo *info)
{
    return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean service_net_delete (ServiceNetInfo *info)
{
    CURL *curl;
    ServiceResponse *response = NULL;
    gboolean result = FALSE;
    
    if (info->id == NULL || info->key == NULL || info->username == NULL || info->password == NULL)
        return FALSE;

    if (service_net_login (info))
    {
        info->task_type = SERVICE_TASK_DELETE;
        curl = service_net_new_handle (info);
        service_net_perform_connecton (curl, info, &response);
        result = response->status;
        service_response_free (response);
    }

    return result;
    
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
