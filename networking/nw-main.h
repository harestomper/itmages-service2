


#ifndef __NW_MAIN_H__
#define __NW_MAIN_H__

#define NW_URL_API              "http://itmages.ru/api/v2"
#define NW_USER_AGENT           "ITmages GNU/Linux network client v0.0.1"
#define NW_API_LOGIN            "login"
#define NW_API_LOGOUT           "logout"
#define NW_API_CHECK            "check"
#define NW_API_UPLOAD           "add"
#define NW_API_SHORTING         "short"
#define NW_API_DELETE           "delete"
#define NW_API_INFO             "info"
#define NW_API_INFO_FULL        "get"

#define NW_API_DATA_KEY         "UFileManager[picture]"
#define NW_API_USER_KEY         "username"
#define NW_API_PASS_KEY         "password"
#define NW_API_POST_FORMAT      NW_API_USER_KEY"=%s&"NW_API_PASS_KEY"%s"
#define NW_API_TIME_FORMAT      "%Y-%m-%d %H:%M:%S"    /* 2011-08-31 19:05:25 */

#define NW_XML_TAG_ITEM         "item"
#define NW_XML_TAG_RESULT       "response"
#define NW_XML_TAG_ITEMS        "items"
#define NW_XML_TAG_SHORT        "short"
#define NW_XML_TAG_ERROR        "error"

#define NW_XML_KEY_STATUS           "status"
#define NW_XML_KEY_USER             "user"
#define NW_XML_KEY_CODE             "code"
#define NW_XML_KEY_N_PER_PAGE       "perPage"
#define NW_XML_KEY_N_ITEMS          "count"
#define NW_XML_KEY_N_ITEMS_TOTAL    "itemsTotal"
#define NW_XML_KEY_SHORT            "url"

#define NW_XML_KEY_ITEM_ID          "id"
#define NW_XML_KEY_ITEM_KEY         "key"
#define NW_XML_KEY_ITEM_THUMB       "turi"
#define NW_XML_KEY_ITEM_ORIG        "iuri"
#define NW_XML_KEY_ITEM_SERVER      "server"
#define NW_XML_KEY_ITEM_NAME        "iname"
#define NW_XML_KEY_ITEM_HASH        "hash"
#define NW_XML_KEY_ITEM_MIME        "mime"
#define NW_XML_KEY_ITEM_SIZE        "size"
#define NW_XML_KEY_ITEM_WIDTH       "width"
#define NW_XML_KEY_ITEM_HEIGHT      "height"
#define NW_XML_KEY_ITEM_ALBUM       "album"
#define NW_XML_KEY_ITEM_VIEWS       "views"
#define NW_XML_KEY_ITEM_DATE        "created"


#define NW_RESPONSE_GUEST       "Guest"
#define NW_RESPONSE_OK          "ok"

#endif /* __NW_MAIN_H__ */
