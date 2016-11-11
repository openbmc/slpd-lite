#pragma once

namespace slp
{
    constexpr auto CONF_FILE = "/etc/slp_lite.conf";
    constexpr size_t VERSION_2 = 2;
    constexpr auto SUCCESS = 0;
    constexpr auto PORT = 427;
    constexpr auto ADDRESS = "0.0.0.0";
    constexpr auto TIMEOUT = 30;
    namespace header
    {
        constexpr size_t SIZE_VERSION = 1;
        constexpr size_t SIZE_LENGTH = 3;
        constexpr size_t SIZE_FLAGS = 2;
        constexpr size_t SIZE_OFFSET = 3;
        constexpr size_t SIZE_XID = 2;
        constexpr size_t SIZE_LANG = 2;

        constexpr size_t FUNCTION_OFFSET = 1;
        constexpr size_t LENGTH_OFFSET = 2;
        constexpr size_t FLAGS_OFFSET = 5;
        constexpr size_t EXT_OFFSET = 7;
        constexpr size_t XID_OFFSET = 10;
        constexpr size_t LANG_LEN_OFFSET = 12;
        constexpr size_t LANG_OFFSET = 14;

        constexpr size_t MIN_LEN = 14;
    }

    namespace response
    {
        constexpr size_t SIZE_ERROR = 2;
        constexpr size_t SIZE_SERVICE = 2;
        constexpr size_t SIZE_URL_COUNT = 2;
        constexpr size_t SIZE_URL_ENTRY = 6;
        constexpr size_t SIZE_RESERVED = 1;
        constexpr size_t SIZE_LIFETIME = 2;
        constexpr size_t SIZE_URLLENGTH = 2;
        constexpr size_t SIZE_AUTH = 1;

        constexpr size_t ERROR_OFFSET = 16;
        constexpr size_t SERVICE_LEN_OFFSET = 18;
        constexpr size_t SERVICE_OFFSET = 20;
        constexpr size_t URL_ENTRY_OFFSET = 18;

    }

    namespace request
    {
        constexpr size_t MIN_SRVTYPE_LEN = 22;
        constexpr size_t MIN_SRV_LEN = 26;

        constexpr size_t SIZE_PRLIST = 2;
        constexpr size_t SIZE_NAMING = 2;
        constexpr size_t SIZE_SCOPE = 2;
        constexpr size_t SIZE_SERVICE_TYPE = 2;
        constexpr size_t SIZE_PREDICATE = 2;
        constexpr size_t SIZE_SLPI = 2;

        constexpr size_t PR_LEN_OFFSET = 16;
        constexpr size_t PR_OFFSET = 18;
        constexpr size_t SERVICE_OFFSET = 20;

    }
    
}
