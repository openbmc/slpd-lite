#pragma once

#include <stdio.h>

#include <array>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "slp_service_info.hpp"

namespace slp
{

using buffer = std::vector<uint8_t>;

template<typename T>
using deleted_unique_ptr = std::unique_ptr<T,std::function<void(T*)>>;

namespace request //for all the request types
{

struct ServiceType
{

    uint16_t prListLen = 0;
    std::string prList;
    uint16_t namingAuthLen = 0;
    std::string namingAuth;
    uint16_t scopeListLen = 0;
    std::string scopeList;

};

struct Service
{
    uint16_t prListLen = 0;
    std::string prList;
    uint16_t srvTypeLen = 0;
    std::string srvType;
    uint16_t scopeListLen = 0;
    std::string scopeList;
    uint16_t predicateLen = 0;
    std::string predicate;
    uint16_t spistrLen = 0;
    std::string spistr;

};
}//namespace request

enum class FunctionType : uint8_t
{
    SRVRQST     = 0x01,
    SRVRPLY     = 0x02,
    ATTRRQST    = 0x06,
    ATTRRPLY    = 0x07,
    SRVTYPERQST = 0x09,
    SRVTYPERPLY = 0x0A,
    SAADV       = 0x0B,
};

enum class Error : uint8_t
{
    LANGUAGE_NOT_SUPPORTED = 0x01,
    PARSE_ERROR            = 0x02,
    INVALID_REGISTRATION   = 0x03,
    SCOPE_NOT_SUPPORTED    = 0x04,
    AUTHENTICATION_UNKNOWN = 0x05,
    AUTHENTICATION_ABSENT  = 0x06,
    AUTHENTICATION_FAILED  = 0x07,
    VER_NOT_SUPPORTED      = 0x09,
    INTERNAL_ERROR         = 0x0A,
    DA_BUSY_NOW            = 0x0B,
    OPTION_NOT_UNDERSTOOD  = 0x0C,
    INVALID_UPDATE         = 0x0D,
    MSG_NOT_SUPPORTED      = 0x0E,
};


struct Header
{
    uint8_t version = 0;
    uint8_t functionID = 0;
    std::array<uint8_t, 3> length;
    uint16_t flags = 0;
    std::array<uint8_t, 3> extOffset;
    uint16_t xid = 0;
    uint16_t langtagLen = 0;
    std::string langtag;

};

union Payload
{
    request::ServiceType srvtyperqst;
    request::Service srvrqst;

    Payload() {};
    ~Payload() {};
    Payload(Payload&&) = default;
    Payload& operator=(Payload &&) = default;
    Payload(const Payload&);
    Payload& operator=(const Payload&);

};



/** SLP wire protocol message management structures and prototypes */
struct Message
{
    Header header;
    Payload body;

    Message() {};
    ~Message() = default;
    Message(const Message&);
    Message& operator=(const Message&);
    Message(Message&&) = default;
    Message& operator=(Message &&) = default;
};

class Parser
{

    public:
        /* Not allowing it to instantiate */
        Parser() = delete;
        /** Parse a buffer and fill the header and the body of the message.
         *
         * @param[in] buffer - The buffer from which data should be parsed.
         *
         * @return Zero on success and parsed msg object,
         *         non-zero on failure and empty msg object.
         *
         */

        static std::tuple<int, Message> parseBuffer(const buffer& buf);

    private:
        /** Parse header data from the buffer.
         *
         * @param[in] buffer - The buffer from which data should be parsed.
         *
         * @return Zero on success and fills header object inside message,
         *         non-zero on failure and empty msg object.
         *
         * @internal
         */

        static std::tuple<int, Message> parseHeader(const buffer& buf);

        /** Parse a srvType request
         *
         * @param[in] buffer - The buffer from which data should be parsed.
         *
         * @return Zero on success,and fills the body object inside message.
         *         non-zero on failure and empty msg object.
         *
         * @internal
         */

        static std::tuple<int, Message> parseSrvTypeRqst(const buffer& buf);

        /** Parse a service request.
          *
          * @param[in] buffer - The buffer from which data should be parsed.
          *
          * @return Zero on success,and fills the body object inside message.
          *         non-zero on failure and empty msg object.
          *
          * @internal
          */

        static std::tuple<int, Message> parseSrvRqst(const buffer& buf);

};

class Handler
{

    public:

        /* Not allowing it to instantiate */
        Handler() = delete;

        using ServiceList = std::map<std::string, ConfData>;

        /** Handle the  request  message.
         *
         * @param[in] msg - The message to process.
         *
         * @return In case of success, the vector is populated with the data
         *         available on the socket and return code is 0.
         *         In case of error, nonzero code and vector is set to size 0.
         *
         */

        static std::tuple<int, buffer> processRequest(const Message& msg);

        /** Handle the error
         *
         * @param[in] msg - Req message.
         * @param[in] err - Error code.
         *
         * @return the vector populated with the error data
         */

        static buffer processError(const Message& req,
                                   const uint8_t err);
    private:

        /** Handle the  SrvRequest message.
         *
         * @param[in] msg - The message to process
         *
         * @return In case of success, the vector is populated with the data
         *         available on the socket and return code is 0.
         *         In case of error, nonzero code and vector is set to size 0.
         *
         * @internal
         */

        static std::tuple<int, buffer> processSrvRequest(const Message& msg);


        /** Handle the  SrvTypeRequest message.
         *
         * @param[in] msg - The message to process
         *
         * @return In case of success, the vector is populated with the data
         *         available on the socket and return code is 0.
         *         In case of error, nonzero code and vector is set to size 0.
         *
         * @internal
         *
         */

        static std::tuple<int, buffer> processSrvTypeRequest(const Message& msg);

        /**  Read the SLPinfo from the configuration.
         *
         * @param[in] filename - Name of the conf file
         *
         * @return the list of the services
         *
         * @internal
         *
         */
        static ServiceList readSLPServiceInfo(const std::string& filename);

        /**  Get all the interface address
         *
         * @return the list of the interface address.
         *
         * @internal
         *
         */

        static std::list<std::string> getIntfAddrs();

        /** Fill the buffer with the header data from the request object
         *
         * @param[in] req - Header data will be copied from
         *
         * @return the vector is populated with the data
         *
         * @internal
         */
        static buffer prepareHeader(const Message& req);


};
}//namespce slp
