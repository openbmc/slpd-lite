#include "slp.hpp"
#include <string.h>
#include <string>
#include <algorithm>
#include "endian.hpp"
#include "slp_meta.hpp"


uint8_t SLP::Parser::parseHeader(const std::vector<uint8_t>& buffer,
                                 Message& req)
{
    /*  0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |    Version    |  Function-ID  |            Length             |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Length, contd.|O|F|R|       reserved          |Next Ext Offset|
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |  Next Extension Offset, contd.|              XID              |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |      Language Tag Length      |         Language Tag          \
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */


    memcpy(&(req.header.version),
           buffer.data(),
           SLP::HEADER::SIZE_VERSION);

    memcpy(&(req.header.functionid),
           buffer.data() + SLP::HEADER::FUNCTION_OFFSET,
           SLP::HEADER::SIZE_VERSION);

    memcpy((req.header.length.data()),
           buffer.data() + SLP::HEADER::LENGTH_OFFSET,
           SLP::HEADER::SIZE_LENGTH);

    memcpy(&(req.header.flags),
           buffer.data() + SLP::HEADER::FLAGS_OFFSET,
           SLP::HEADER::SIZE_FLAGS);

    req.header.flags = (endian::from_network<uint16_t>(req.header.flags));

    memcpy((req.header.extoffset.data()),
           buffer.data() + SLP::HEADER::EXT_OFFSET,
           SLP::HEADER::SIZE_OFFSET);

    memcpy(&(req.header.xid),
           buffer.data() + SLP::HEADER::XID_OFFSET,
           SLP::HEADER::SIZE_XID);

    req.header.xid = (endian::from_network<uint16_t>(req.header.xid));

    memcpy(&(req.header.langtaglen),
           (buffer.data() + SLP::HEADER::LANG_LEN_OFFSET),
           SLP::HEADER::SIZE_LANG);

    req.header.langtaglen = (endian::from_network<uint16_t>(req.header.langtaglen));

    req.header.langtag = (const char*)buffer.data() + SLP::HEADER::LANG_OFFSET;

    /* check for the validity of the function */
    if (req.header.functionid < (uint8_t)FunctionType::SLP_FUNCT_SRVRQST
        || req.header.functionid > (uint8_t)FunctionType::SLP_FUNCT_SAADV)
    {
        return (uint8_t)SLP::Error::PARSE_ERROR;
    }

    return SLP::SUCCESS;
}



uint8_t SLP::Parser::parseSrvTypeRqst(const std::vector<uint8_t>& buffer,
                                      Message& req)
{

    /*  0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |        length of PRList       |        <PRList> String        \
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |   length of Naming Authority  |   <Naming Authority String>   \
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |     length of <scope-list>    |      <scope-list> String      \
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

    /* Enforce SLPv2 service type request size limits. */
    if (buffer.size() < SLP::REQUEST::MIN_SRVTYPE_LEN)
    {
        return (uint8_t)SLP::Error::PARSE_ERROR;
    }

    /* Parse the PRList. */
    memcpy(&(req.body.srvtyperqst.prlistlen),
           buffer.data() + SLP::REQUEST::PR_LEN_OFFSET,
           SLP::REQUEST::SIZE_PRLIST);

    req.body.srvtyperqst.prlistlen = (endian::from_network<uint16_t>
                                      (req.body.srvtyperqst.prlistlen));

    req.body.srvtyperqst.prlist = (const char*)(buffer.data() +
                                  SLP::REQUEST::PR_OFFSET);

    uint8_t pos = SLP::REQUEST::PR_OFFSET +
                  req.body.srvtyperqst.prlistlen;

    /* Parse the Naming Authority. */
    memcpy(&(req.body.srvtyperqst.namingauthlen),
           buffer.data() + pos,
           SLP::REQUEST::SIZE_NAMING);

    req.body.srvtyperqst.namingauthlen = (endian::from_network<uint16_t>
                                          (req.body.srvtyperqst.namingauthlen));
    pos += SLP::REQUEST::SIZE_NAMING;

    if (req.body.srvtyperqst.namingauthlen == 0
        || req.body.srvtyperqst.namingauthlen == 0xffff)
    {
        req.body.srvtyperqst.namingauth = 0;
    }
    else
    {
        req.body.srvtyperqst.namingauth = (const char*)(buffer.data() + pos);
    }

    pos += req.body.srvtyperqst.namingauthlen;
    /* Parse the <scope-list>. */
    memcpy(&(req.body.srvtyperqst.scopelistlen),
           buffer.data() + pos,
           SLP::REQUEST::SIZE_SCOPE);

    req.body.srvtyperqst.scopelistlen = (endian::from_network<uint16_t>
                                         (req.body.srvtyperqst.scopelistlen));
    pos += SLP::REQUEST::SIZE_SCOPE;
    req.body.srvtyperqst.scopelist = (const char*)(buffer.data() + pos);

    return SLP::SUCCESS;
}

uint8_t SLP::Parser::parseSrvRqst(const std::vector<uint8_t>& buffer,
                                  Message& req)
{
    /*  0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |      length of <PRList>       |        <PRList> String        \
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |   length of <service-type>    |    <service-type> String      \
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |    length of <scope-list>     |     <scope-list> String       \
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |  length of predicate string   |  Service Request <predicate>  \
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |  length of <SLP SPI> string   |       <SLP SPI> String        \
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

    /* Enforce v2 service request size limits. */
    if (buffer.size() < SLP::REQUEST::MIN_SRV_LEN)
    {
        return (uint8_t)SLP::Error::PARSE_ERROR;
    }

    /* 1) Parse the PRList. */
    memcpy(&(req.body.srvrqst.prlistlen),
           buffer.data() + SLP::REQUEST::PR_LEN_OFFSET,
           SLP::REQUEST::SIZE_PRLIST);

    req.body.srvrqst.prlistlen = (endian::from_network<uint16_t>
                                  (req.body.srvrqst.prlistlen));

    uint8_t pos = SLP::REQUEST::PR_LEN_OFFSET + SLP::REQUEST::SIZE_PRLIST;

    req.body.srvrqst.prlist = (const char*)(buffer.data() + pos);

    pos += req.body.srvtyperqst.prlistlen;

    /* 2) Parse the <service-type> string. */
    memcpy(&(req.body.srvrqst.srvtypelen),
           buffer.data() + pos,
           SLP::REQUEST::SIZE_SERVICE_TYPE);

    req.body.srvrqst.srvtypelen = (endian::from_network<uint16_t>
                                   (req.body.srvrqst.srvtypelen));

    pos += SLP::REQUEST::SIZE_SERVICE_TYPE;
    req.body.srvrqst.srvtype = (const char*)(buffer.data() + pos);

    pos += req.body.srvrqst.srvtypelen;
    /* 3) Parse the <scope-list> string. */
    memcpy(&(req.body.srvrqst.scopelistlen),
           buffer.data() + pos,
           SLP::REQUEST::SIZE_SCOPE);

    req.body.srvrqst.scopelistlen = (endian::from_network<uint16_t>
                                     (req.body.srvrqst.scopelistlen));

    pos += SLP::REQUEST::SIZE_SCOPE;
    req.body.srvrqst.scopelist = (const char*)(buffer.data() + pos);

    pos += req.body.srvrqst.scopelistlen;
    /* 4) Parse the <predicate> string. */
    memcpy(&(req.body.srvrqst.srvtypelen),
           buffer.data() + pos,
           SLP::REQUEST::SIZE_PREDICATE);

    req.body.srvrqst.predicatelen = (endian::from_network<uint16_t>
                                     (req.body.srvrqst.predicatelen));

    pos += SLP::REQUEST::SIZE_PREDICATE;
    req.body.srvrqst.predicate = (const char*)(buffer.data() + pos);

    pos += req.body.srvrqst.predicatelen;
    /* 5) Parse the <SLP SPI> string. */
    memcpy(&(req.body.srvrqst.srvtypelen),
           buffer.data() + pos,
           SLP::REQUEST::SIZE_SLPI);

    req.body.srvrqst.spistrlen = (endian::from_network<uint16_t>
                                  (req.body.srvrqst.spistrlen));

    pos += SLP::REQUEST::SIZE_SLPI;
    req.body.srvrqst.spistr = (const char*)(buffer.data() + pos);

    return SLP::SUCCESS;
}


uint8_t SLP::Parser::parseBuffer(const std::vector<uint8_t>& buffer,
                                 Message& req)
{
    uint8_t rc = SLP::SUCCESS;
    /* parse the header first */
    rc = Parser::parseHeader(buffer, req);
    if (!rc)
    {
        /* switch on the function id to parse the body */
        switch (req.header.functionid)
        {
            case (uint8_t)FunctionType::SLP_FUNCT_SRVTYPERQST:
                rc = Parser::parseSrvTypeRqst(buffer, req);
                break;
            case (uint8_t)FunctionType::SLP_FUNCT_SRVRQST:
                rc = Parser::parseSrvRqst(buffer, req);
                break;
            default:
                rc = (uint8_t)SLP::Error::MSG_NOT_SUPPORTED;
        }
    }
    return rc;
}
