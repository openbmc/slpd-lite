#pragma once

namespace SLP
{
    constexpr auto CONF_FILE = "/etc/slp_lite.conf";
    constexpr size_t VERSION = 2;
    constexpr auto SUCCESS = 0;
    namespace HEADER
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
    namespace REQUEST
    {
        constexpr size_t SRVTYPE_LEN = 22;
        constexpr size_t SIZE_ERROR = 2;
        constexpr size_t SIZE_SERVICE = 2;

        constexpr size_t ERROR_OFFSET = 16;
        constexpr size_t SERVICELEN_OFFSET = 18;
        constexpr size_t SERVICE_OFFSET = 20;

    }
}
