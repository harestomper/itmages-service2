/*
 * nw-xml.c
 * 
 * Copyright 2013 Voldemar Khramtsov <harestomper@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */


#include <stdio.h>
#include <time.h>
#include <glib-object.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "nw-xml.h"
#include "nw-main.h"

#ifndef strptime
extern char *strptime (const char *s, const char *format, struct tm *tm);
#endif

/*
Error message in the response:

<response status="fail" user="USERNAME">
    <error code="123">Error message</error>
</response>


Response to commands 'login', 'check', 'logout':
    <response status="ok" user="username">
    </response>

Response to command 'delete':
    ><response status="ok" user="elstop">
    </response>

    <?xml version="1.0" encoding="UTF-8"?><response status="fail" user="elstop">
        <error code="200">Error while delete. Possible "id" or "key" incorrect</error>
    </response>

Response to command 'upload':
    <response status="ok" user="elstop">
        <item ... the same as in the 'info' command ...  />
    </response>

Response to command 'get':
    <response status="ok" user="username">
      <items count="10" perPage="10" itemsTotal="1073">
        <item ... the same as in the 'info' command ...  />
        ....
        <item ... the same as in the 'info' command ...  />
      </items>
    </response>

Response to command 'info':
    <response status="ok" user="Guest">
        <item
            id="1234567"
            views="1"
            size="859039"
            width="3264"
            height="1836"
            created="2012-06-11 16:14:14"
            album="0"
            server="storage4"
            policy="rwxr--r--"
            turi="i/12/0611/s_1234567890_1234567_qwertyui.jpeg"
            iuri="i/12/0611/h_1234567890_1234567_qwertyui.jpeg"
            username="username"
            iname="mypicture.jpg"
            hash="c69d7ae1e80f521a3a2f833b43d1024e"
            key="qwertyuiop"
            mime="image/jpeg"
            status="active"  />
    </response>

Response to command 'short':
    <response status="ok" user="Guest">
        <short id="1234567" url="http://itmag.es/qwert" />
    </response>

*/
struct NwXmlResult
{
    NwDataItem **list;
    guint        len;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void nw_xml_result_append (NwXmlResult *res, NwDataItem *item)
{
    res->list = realloc (res->list, sizeof (*res->list) * (res->len + 2));
    res->list [res->len] = item;
    res->list [res->len + 1] = NULL;
    res->len++;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void nw_get_prop (xmlNodePtr      corrent,
                         NwDataItem*     owner,
                         const gchar*    prop_name,
                         NwDataItemType  expected_type)
{
    NwDataItem *prop;
    xmlChar *value;

    if ((value = xmlGetProp (current, prop_name)) == NULL
                                && expected_type != NW_DATA_ITEM_TYPE_NONE)
        return;
        
    prop = nw_data_item_add (&owner, expected_type, prop_name);

    switch (expected_type) {
        case NW_DATA_ITEM_TYPE_INT: 
            prop->val.i64 = strtol (value, NULL, 10);
            break;

        case NW_DATA_ITEM_TYPE_STR:
            prop->val.str = g_strdup (value);
            break;

        case NW_DATA_ITEM_TYPE_BOOL:
            prop->i64 = (g_strcmp0 (value, NW_RESPONSE_OK)
                     || g_strcmp0 (value, "true") == 0
                     || g_strcmp0 (value, "True") == 0
                     || strtol (value, NULL, 10) !=0) ? 1 : 0;
            break;

        case NW_DATA_ITEM_TYPE_DATE_TIME: {
            gchar *p = (gchar*) value, *end;
            struct tm tm_info = {0,};

            end = strptime (p, TIME_FORMAT, &tm_info);

            if (end != NULL)
                prop->val.i64 = mktime (&tm_info);
            else
                prop->val.i64 = -1;
            
            break;
        }

        case NW_DATA_ITEM_TYPE_NONE:
        default:
            break;
    }

    xmlFree (value);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void nw_xml_parser_worker (xmlNodePtr current, NwXmlResult *res)
{
    while (current) {
        NwDataItem *owner = NULL;
        
        if (current->children)
            nw_xml_parser_worker (current->children, res);

        if (xmlStrcmp (current->name, NW_XML_TAG_RESULT) == 0) {
            owner = nw_data_item_new (NW_DATA_ITEM_TYPE_NONE, NW_XML_TAG_RESULT);
            nw_get_prop (current, owner, NW_XML_KEY_STATUS, NW_DATA_ITEM_TYPE_BOOL);
            nw_get_prop (current, owner, NW_XML_KEY_USER, NW_DATA_ITEM_TYPE_STR);

        } else if (xmlStrcmp (current->name, NW_XML_TAG_ERROR)) == 0) {
            owner = nw_data_item_new (NW_DATA_ITEM_TYPE_STR, NW_XML_TAG_ERROR);
            nw_get_prop (current, owner, NW_XML_KEY_CODE, NW_DATA_ITEM_TYPE_INT);
            owner->val.str = xmlNodeGetContent (current);

        } else if (xmlStrcmp (current->name, NW_XML_TAG_ITEMS)) == 0) {
            owner = nw_data_item_new (NW_DATA_ITEM_TYPE_NONE, NW_XML_TAG_ITEMS);
            nw_get_prop (current, owner, NW_XML_KEY_N_PER_PAGE, NW_DATA_ITEM_TYPE_INT);
            nw_get_prop (current, owner, NW_XML_KEY_N_ITEMS, NW_DATA_ITEM_TYPE_INT);
            nw_get_prop (current, owner, NW_XML_KEY_N_ITEMS_TOTAL, NW_DATA_ITEM_TYPE_INT);
             
        } else if (xmlStrcmp (current->name, NW_XML_TAG_ITEM)) == 0) {
            owner = nw_data_item_new (NW_DATA_ITEM_TYPE_NONE, NW_XML_TAG_ITEM);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_ID, NW_DATA_ITEM_TYPE_STR);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_KEY, NW_DATA_ITEM_TYPE_STR);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_THUMB, NW_DATA_ITEM_TYPE_STR);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_ORIG, NW_DATA_ITEM_TYPE_STR);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_SERVER, NW_DATA_ITEM_TYPE_STR);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_NAME, NW_DATA_ITEM_TYPE_STR);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_HASH, NW_DATA_ITEM_TYPE_STR);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_MIME, NW_DATA_ITEM_TYPE_STR);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_SIZE, NW_DATA_ITEM_TYPE_INT);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_WIDTH, NW_DATA_ITEM_TYPE_INT);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_HEIGHT, NW_DATA_ITEM_TYPE_INT);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_ALBUM, NW_DATA_ITEM_TYPE_INT);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_VIEWS, NW_DATA_ITEM_TYPE_INT);
            nw_get_prop (current, owner, NW_XML_KEY_ITEM_DATE, NW_DATA_ITEM_TYPE_DATE_TIME);

        } else if (xmlStrcmp (current->name, NW_XML_TAG_SHORT)) == 0) {
            owner = nw_data_item_new (NW_DATA_ITEM_TYPE_NONE, NW_XML_TAG_SHORT);
            nw_get_prop (current, owner, NW_XML_KEY_SHORT, NW_DATA_ITEM_TYPE_STR);
        }

        if (owner)
            nw_xml_result_append (res, owner);

        current = current->next;
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
NwDataItem** nw_xml_parse (const gchar *contents, guint size, guint *n_elem)
{
    xmlNodePtr current;
    xmlDocPtr document;
    NwXmlResult result = {NULL, 0};

    if ((document = xmlParseMemory (contents, (int) size)) == NULL) {
        g_warning ("%s: %s: Failed to parse contents\n", G_STRLOC, G_STRFUNC);
        return NULL;
    }

    current = xmlDocGetRootElement (doc);
    nw_xml_parser_worker (current, &result);

    if (n_elem)
        *n_elem = result->len;

    xmlFreeDoc (document);

    return result->list;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
