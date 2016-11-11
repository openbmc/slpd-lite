#pragma once
#include <stdio.h>
#include <vector>
#include <array>
#include <memory>
#include <string>
#include <map>
#include "slp_service_info.hpp"

namespace SLP
{
    namespace REQUEST_TYPE //for all the request types
    {
        /** SLPSrvTypeRqst structure and associated functions */
        struct SrvType
        {

            uint16_t prlistlen;
            const char * prlist;
            uint16_t namingauthlen;
            const char *namingauth;
            uint16_t scopelistlen;
            const char *scopelist;

            SrvType() :
                prlistlen(0),
                prlist(nullptr),
                namingauthlen(0),
                namingauth(nullptr),
                scopelistlen(0),
                scopelist(nullptr) {}

            ~SrvType() = default;
            SrvType(const SrvType&) = delete;
            SrvType& operator=(const SrvType&) = delete;
            SrvType(SrvType&&) = delete;
            SrvType& operator=(SrvType&&) = delete;


        };
    }

    enum class FunctionType : uint8_t
    {
        SLP_FUNCT_SRVRQST     = 0x01,
        SLP_FUNCT_SRVRPLY     = 0x02,
        SLP_FUNCT_ATTRRQST    = 0x06,
        SLP_FUNCT_ATTRRPLY    = 0x07,
        SLP_FUNCT_SRVTYPERQST = 0x09,
        SLP_FUNCT_SRVTYPERPLY = 0x0A,
        SLP_FUNCT_SAADV       = 0x0B,
    };

    enum class Error : uint8_t
    {
        LANGUAGE_NOT_SUPPORTED = 0x1,
        PARSE_ERROR,
        INVALID_REGISTRATION,
        SCOPE_NOT_SUPPORTED,
        AUTHENTICATION_UNKNOWN,
        AUTHENTICATION_ABSENT,
        AUTHENTICATION_FAILED,
        VER_NOT_SUPPORTED = 0x9,
        INTERNAL_ERROR,
        DA_BUSY_NOW,
        OPTION_NOT_UNDERSTOOD,
        INVALID_UPDATE,
        MSG_NOT_SUPPORTED,
    };


    struct Header
    {
        uint8_t version;
        uint8_t functionid;
        std::array<uint8_t,3> length;
        uint16_t flags;
        std::array<uint8_t,3> extoffset;
        uint16_t xid;
        uint16_t langtaglen;
        const char * langtag;

        Header() :
            version(0),
            functionid(0),
            flags(0),
            xid(0),
            langtaglen(0),
            langtag(nullptr)
        {

            length.fill(0);
            extoffset.fill(0);
        }

        ~Header() = default;
        Header(const Header&) = default;
        Header& operator=(const Header&) = default;
        Header(Header&&) = default;
        Header& operator=(Header&&) = default;


    };

    /** SLP wire protocol message management structures and prototypes */
    struct Message
    {
        Header header;
        union _body
        {
            REQUEST_TYPE::SrvType srvtyperqst;
            _body() {};
            ~_body() {};

        } body;

        Message() = default;
        ~Message() = default;
        Message(const Message&) = default;
        Message& operator=(const Message&) = default;
        Message(Message&&) = default;
        Message& operator=(Message&&) = default;
    };

    class Parser
    {
        private:
            /** Parse header data from the buffer.
             *
             * @param[in] buffer - The buffer from which data should be parsed.
             * @param[out] msg -   The msg obj into which buffer should be parsed
             *
             *
             * @return Zero on success, or a non-zero error code.
             *
             * @internal
             */

            static uint8_t parseHeader(const std::vector<uint8_t>& buffer,
                    SLP::Message& msg);

            /** This function will parse the request payload for a service type request.
             *
             * @param[in] buffer - The buffer from which data should be parsed.
             * @param[out] msg - The msg obj  into which  buffer should be parsed.
             *
             * @return Zero on success, or a non-zero error code.
             *
             * @internal
             */

            static uint8_t parseSrvTypeRqst(const std::vector<uint8_t>& buffer,
                    SLP::Message& msg);

        public:
            /** Parse a buffer and fill the header and the body of the message.
             *
             * @param[in] buffer - The buffer from which data should be parsed.
             * @param[out] msg - The msg into which buffer should be parsed.
             *
             * @return Zero on success, or a non-zero error code.
             *
             */

            static uint8_t parseBuffer(const std::vector<uint8_t>& buffer,
                    SLP::Message & msg);
    };

    class Handler
    {

        public:

            using SlpServiceList = std::map<std::string,SlpInfo>;

            /** Handle the  request  message.
             *
             * @param[in] msg - The message to process
             * @param[out] buff - The response buffer to fill.
             *
             * @return Zero on success, or a non-zero error code.
             *
             */

            static uint8_t processRequest(const Message& msg,
                    std::vector<uint8_t>& buff);

            /** Handle the error
             *
             * @param[in] msg - Req message
             * @param[out] buff - The response buffer to fill.
             * @param[in] err - Error code
             *
             */

            static void processError(const Message& req,
                    std::vector<uint8_t>& buff,
                    uint8_t err);
        private:

            /** Handle the  SrvTypeRequest message.
             *
             * @param[in] msg - The message to process
             * @param[out] buff - The response buffer to fill.
             *
             * @return Zero on success, or a non-zero error code.
             *
             */

            static uint8_t processSrvTypeRequest(const Message& msg,
                    std::vector<uint8_t>& buff);

            /**  Read the SLPinfo from the configuration.
             *
             * @param[in] filename - Name of the conf file
             * @param[out] svcList - Service List
             *
             * @return Zero on success, or a non-zero error code.
             *
             */
            static uint8_t readSLPServiceInfo(const std::string& filename,
                    SlpServiceList& svcList);

            /** Fill the buffer with the header data from the req object
             *
             * @param[in] req - Header data will be copied from
             * @param[out] buff - data will be copied to.
             *
             */
            static void prepareHeader(const Message& req,
                    std::vector<uint8_t>& buff);

            static SlpServiceList slpSvcLst;

    };

}


