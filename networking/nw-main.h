


#ifndef __NWMAIN__
#define __NWMAIN__


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

#define NW_KEY_STATUS           "status"
#defien NW_KEY_USER             "user"
#define NW_KEY_PAGES            "perPage"
#define NW_KEY_ITEMS            "count"
#define NW_KEY_TOTAL_ITEMS      "itemsTotal"
#define NW_KEY_SHORT            "url"

#define NW_KEY_ITEM_ID          "id"
#define NW_KEY_ITEM_KEY         "key"
#define NW_KEY_ITEM_THUMB       "turi"
#define NW_KEY_ITEM_ORIG        "iuri"
#define NW_KEY_ITEM_SERVER      "server"
#define NW_KEY_ITEM_NAME        "iname"
#define NW_KEY_ITEM_HASH        "hash"
#define NW_KEY_ITEM_MIME        "mime"
#define NW_KEY_ITEM_SIZE        "size"
#define NW_KEY_ITEM_WIDTH       "width"
#define NW_KEY_ITEM_HEIGHT      "height"
#define NW_KEY_ITEM_ALBUM       "album"
#define NW_KEY_ITEM_VIEWS       "views"
#define NW_KEY_ITEM_DATE        "created"


#define NW_RESPONSE_GUEST       "Guest"
#define NW_RESPONSE_OK          "ok"


#endif /* __NWMAIN__ */
