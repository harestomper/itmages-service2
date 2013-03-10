/*
 *      service-xml-utils.c
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


#include "service-xml-utils.h"

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define KEY_ITEMS     "items"
#define KEY_ITEM      "item"
#define KEY_ERROR     "error"
#define KEY_RESP      "response"
#define PROP_USER     "user"
#define PROP_STATUS   "status"
#define PROP_ID       "id"
#define PROP_KEY      "key"
#define PROP_FULL     "iuri"
#define PROP_SMALL    "turi"
#define PROP_SERVER   "server"
#define PROP_SHORT    "short"
#define PROP_FNAME    "iname"
#define PROP_MIME     "mime"
#define PROP_WIDTH    "width"
#define PROP_HEIGHT   "height"
#define PROP_SIZE     "size"
#define PROP_CREATE   "created"
#define PROP_HASH     "hash"
#define PROP_ALBUM    "album"
#define PROP_URL      "url"
#define PROP_UNAME    "username"
#define PROP_COUNT    "count"
#define PROP_NITEMS   "itemsTotal"
#define PROP_PAGE     "perPage"
#define STATUS_OK     "ok"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define getProp(n, k) (gchar*) (xmlGetProp ((n), (xmlChar*) (k)))
#define strEqual(s1, s2) (gboolean) (xmlStrEqual ((xmlChar*) (s1), (xmlChar*) (s2)))
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//G_DEFINE_BOXED_TYPE (ServiceResponse, service_response, service_response_copy, service_response_free)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static  ServiceImageInfo*    service_xml_get_image_info    (xmlNode *node);
static  gint64               service_xml_parse_int_prop    (xmlNode *node, const gchar* key);
static  void                 service_xml_parse_items        (xmlNode *node, ServiceResponse *response);
static  gint64               service_xml_parse_created     (xmlNode *node);
static  gboolean             _service_xml_get_response     (xmlDoc *doc, xmlNode *a_node, ServiceResponse *response);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gint64 service_xml_parse_created (xmlNode *node)
{
    struct tm t;
    gint64 result = 0;
    gchar *string;

    string = (gchar*) getProp (node, (const xmlChar*) PROP_CREATE);
    
    if (string == NULL)
        return 0;

    if (strptime (string, "%Y-%m-%d %H:%M:%S", &t))
        result = (gint64) mktime (&t);

    xmlFree (string);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ServiceResponse* service_xml_get_response (const gchar* xml_data)
{
    xmlDoc          *doc = NULL;
    xmlNode         *root_element = NULL;
    ServiceResponse *response = NULL;

    doc = xmlReadDoc ((xmlChar*) xml_data, NULL, NULL, 0);

    if (doc == NULL) {
        g_warning ("ERROR: Failed to read the memory");
    } else {

        response = service_response_new ();
        root_element = xmlDocGetRootElement (doc);

        if (!_service_xml_get_response (doc, root_element, response))
        {
            service_response_free (response);
            response = NULL;
            g_warning ("ERROR: Response unknown");
        }

        xmlFreeDoc (doc);
    }

    xmlCleanupParser ();

    return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gint64 service_xml_parse_int_prop (xmlNode *node, const gchar* key)
{
    gchar *temp;
    gint64 value;

    temp = getProp (node, key);
    value = strtol (temp ? temp : "0", NULL, 10);
    xmlFree (temp);

    return value;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static ServiceImageInfo* service_xml_get_image_info (xmlNode *node)
{
    ServiceImageInfo *result = NULL;

    if (strEqual (node->name, "item") == 0)
        return NULL;

    result = service_image_info_new ();
    result->id = getProp (node, PROP_ID);
    result->key = getProp (node, PROP_KEY);
    result->full = getProp (node, PROP_FULL);
    result->small = getProp (node, PROP_SMALL);
    result->server = getProp (node, PROP_SERVER);
    result->username = getProp (node, PROP_UNAME);
    result->hash = getProp (node, PROP_HASH);
    result->filename = getProp (node, PROP_FNAME);
    result->mime = getProp (node, PROP_MIME);
    result->album_ex = (gint) service_xml_parse_int_prop (node, PROP_ALBUM);
    result->width = (gint) service_xml_parse_int_prop (node, PROP_WIDTH);
    result->height = (gint) service_xml_parse_int_prop (node, PROP_HEIGHT);
    result->size = service_xml_parse_int_prop (node, PROP_SIZE);
    result->created = service_xml_parse_created (node);

    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void  service_xml_parse_items (xmlNode *node, ServiceResponse *response)
{
    int n = 0;

    response->n_items = (int) service_xml_parse_int_prop (node, PROP_COUNT);
    response->total_per_page = service_xml_parse_int_prop (node, PROP_PAGE);
    response->total_n_items = service_xml_parse_int_prop (node, PROP_NITEMS);

    node = node->xmlChildrenNode;

    while (node != NULL)
    {
        if (strEqual (node->name, KEY_ITEM))
        {
            ServiceImageInfo *item;

            item = service_xml_get_image_info (node);

            if (item != NULL)
            {
                response->items = g_slist_append (response->items, item);
                n++;
            }
        }
        node = node->next;
    }

    response->n_items = n;

    if (n == 0)
        response->items = NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean _service_xml_get_response (xmlDoc *doc, xmlNode *a_node, ServiceResponse *response)
{
    xmlNode *cur_node = NULL;
 
    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
    {

        if (cur_node->type == XML_ELEMENT_NODE && strEqual (cur_node->name, KEY_RESP))
        {
            gchar *temp;

            temp = getProp (cur_node, PROP_STATUS),
            response->status = (temp != NULL && strEqual (temp, STATUS_OK)) ? TRUE : FALSE;
            response->username = getProp (cur_node, PROP_USER);
            response->short_id = getProp (cur_node, PROP_SHORT);
            response->id = getProp (cur_node, PROP_ID);
            cur_node = cur_node->xmlChildrenNode;

            while (cur_node != NULL)
            {

                if (strEqual (cur_node->name, KEY_ERROR)) {
                    response->error = (gchar*) xmlNodeListGetString (doc, cur_node->xmlChildrenNode, 1);

                } else if (strEqual (cur_node->name, KEY_ITEMS)) {
                    service_xml_parse_items (cur_node, response);

                } else if (strEqual (cur_node->name, PROP_SHORT)) {
                    response->short_id = getProp (cur_node, PROP_URL);
                    response->id = getProp (cur_node, PROP_ID);
                } else if (strEqual (cur_node->name, KEY_ITEM)) {
                    ServiceImageInfo *info;
                    info = service_xml_get_image_info (cur_node);

                    if (info != NULL)
                        response->items = g_slist_append (response->items, info);
                }

                cur_node = cur_node->next;
            }

            xmlFree (temp);
            return TRUE;
        }

        _service_xml_get_response (doc, cur_node->children, response);
    }

    return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

