#include "slp.hpp"

#include <string.h>

#include <string>
#include <algorithm>

#include "endian.hpp"
#include "slp_meta.hpp"

std::tuple<int, slp::Message> slp::Parser::parseHeader(const buffer& buff)
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
    slp::Message req;
    int rc = slp::SUCCESS;

    std::copy_n(buff.data(),
                slp::header::SIZE_VERSION,
                &req.header.version);

    std::copy_n(buff.data() + slp::header::FUNCTION_OFFSET,
                slp::header::SIZE_VERSION,
                &req.header.functionID);

    std::copy_n(buff.data() + slp::header::LENGTH_OFFSET,
                slp::header::SIZE_LENGTH,
                req.header.length.data());

    std::copy_n(buff.data() + slp::header::FLAGS_OFFSET,
                slp::header::SIZE_FLAGS,
                (uint8_t*)&req.header.flags);

    req.header.flags = endian::from_network<uint16_t>(req.header.flags);

    std::copy_n(buff.data() + slp::header::EXT_OFFSET,
                slp::header::SIZE_OFFSET,
                req.header.extOffset.data());

    std::copy_n(buff.data() + slp::header::XID_OFFSET,
                slp::header::SIZE_XID,
                (uint8_t*)&req.header.xid);

    req.header.xid = endian::from_network<uint16_t>(req.header.xid);

    std::copy_n(buff.data() + slp::header::LANG_LEN_OFFSET,
                slp::header::SIZE_LANG,
                &req.header.langtagLen);

    std::copy_n(buff.data() + slp::header::LANG_OFFSET,
                req.header.langtagLen,
                (uint8_t*)req.header.langtag.c_str());

    req.header.langtagLen = endian::from_network<uint16_t>
                            (req.header.langtagLen);

    /* check for the validity of the function */
    if (req.header.functionID < static_cast<uint8_t>
        (FunctionType::SLP_FUNCT_SRVRQST)
        || req.header.functionID > static_cast<uint8_t>(FunctionType::SLP_FUNCT_SAADV))
    {
        rc = static_cast<int>(slp::Error::PARSE_ERROR);
    }

    return std::make_tuple(rc, std::move(req));
}


std::tuple<int, slp::Message> slp::Parser::parseSrvTypeRqst(const buffer& buff)
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

    slp::Message req;
    /* Enforce SLPv2 service type request size limits. */
    if (buff.size() < slp::request::MIN_SRVTYPE_LEN)
    {
        return std::make_tuple((int)slp::Error::PARSE_ERROR,
                               std::move(req));
    }

    /* Parse the PRList. */
    std::copy_n(buff.data() + slp::request::PR_LEN_OFFSET,
                slp::request::SIZE_PRLIST,
                (uint8_t*)&req.body.srvtyperqst.prListLen);

    std::copy_n(buff.data() + slp::request::PR_OFFSET,
                req.body.srvtyperqst.prListLen,
                (uint8_t*)req.body.srvtyperqst.prList.c_str());

    uint8_t pos = slp::request::PR_OFFSET +
                  req.body.srvtyperqst.prListLen;

    req.body.srvtyperqst.prListLen = endian::from_network<uint16_t>
                                     (req.body.srvtyperqst.prListLen);


    /* Parse the Naming Authority. */
    std::copy_n(buff.data() + pos,
                slp::request::SIZE_NAMING,
                (uint8_t*)&req.body.srvtyperqst.namingAuthLen);

    pos += slp::request::SIZE_NAMING;

    if (req.body.srvtyperqst.namingAuthLen == 0
        || req.body.srvtyperqst.namingAuthLen == 0xffff)
    {
        req.body.srvtyperqst.namingAuth = "";
    }
    else
    {
        std::copy_n(buff.data() + pos,
                    req.body.srvtyperqst.namingAuthLen,
                    (uint8_t*)req.body.srvtyperqst.namingAuth.c_str());

    }

    pos += req.body.srvtyperqst.namingAuthLen;

    req.body.srvtyperqst.namingAuthLen = endian::from_network<uint16_t>
                                         (req.body.srvtyperqst.namingAuthLen);
    /* Parse the <scope-list>. */
    std::copy_n(buff.data() + pos,
                slp::request::SIZE_SCOPE,
                (uint8_t*)&req.body.srvtyperqst.scopeListLen);

    pos += slp::request::SIZE_SCOPE;

    std::copy_n(buff.data() + pos,
                req.body.srvtyperqst.scopeListLen,
                (uint8_t*)req.body.srvtyperqst.scopeList.c_str());

    req.body.srvtyperqst.scopeListLen = endian::from_network<uint16_t>
                                        (req.body.srvtyperqst.scopeListLen);

    return std::make_tuple((int)slp::SUCCESS,
                           std::move(req));
}

std::tuple<int, slp::Message> slp::Parser::parseSrvRqst(const buffer& buff)
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

    slp::Message req;
    /* Enforce v2 service request size limits. */
    if (buff.size() < slp::request::MIN_SRV_LEN)
    {
        return std::make_tuple((int)slp::Error::PARSE_ERROR,
                               std::move(req));
    }

    /* 1) Parse the PRList. */
    std::copy_n(buff.data() + slp::request::PR_LEN_OFFSET,
                slp::request::SIZE_PRLIST,
                (uint8_t*)&req.body.srvrqst.prListLen);

    auto pos = slp::request::PR_LEN_OFFSET + slp::request::SIZE_PRLIST;

    std::copy_n(buff.data() + pos,
                req.body.srvrqst.prListLen,
                (uint8_t*)req.body.srvrqst.prList.c_str());


    pos += req.body.srvtyperqst.prListLen;

    req.body.srvrqst.prListLen = endian::from_network<uint16_t>
                                 (req.body.srvrqst.prListLen);

    /* 2) Parse the <service-type> string. */
    std::copy_n(buff.data() + pos,
                slp::request::SIZE_SERVICE_TYPE,
                (uint8_t*)&req.body.srvrqst.srvTypeLen);


    pos += slp::request::SIZE_SERVICE_TYPE;
    std::copy_n(buff.data() + pos,
                req.body.srvrqst.srvTypeLen,
                (uint8_t*)req.body.srvrqst.srvType.c_str());

    pos += req.body.srvrqst.srvTypeLen;

    req.body.srvrqst.srvTypeLen = endian::from_network<uint16_t>
                                  (req.body.srvrqst.srvTypeLen);
    /* 3) Parse the <scope-list> string. */
    std::copy_n(buff.data() + pos,
                slp::request::SIZE_SCOPE,
                (uint8_t*)&req.body.srvrqst.scopeListLen);

    pos += slp::request::SIZE_SCOPE;
    std::copy_n(buff.data() + pos,
                req.body.srvrqst.scopeListLen,
                (uint8_t*)req.body.srvrqst.scopeList.c_str());

    pos += req.body.srvrqst.scopeListLen;

    req.body.srvrqst.scopeListLen = endian::from_network<uint16_t>
                                    (req.body.srvrqst.scopeListLen);
    /* 4) Parse the <predicate> string. */
    std::copy_n(buff.data() + pos,
                slp::request::SIZE_PREDICATE,
                (uint8_t*)&req.body.srvrqst.predicateLen);

    pos += slp::request::SIZE_PREDICATE;

    std::copy_n(buff.data() + pos,
                req.body.srvrqst.predicateLen,
                (uint8_t*)req.body.srvrqst.predicate.c_str());

    pos += req.body.srvrqst.predicateLen;

    req.body.srvrqst.predicateLen = endian::from_network<uint16_t>
                                    (req.body.srvrqst.predicateLen);
    /* 5) Parse the <SLP SPI> string. */
    std::copy_n(buff.data() + pos,
                slp::request::SIZE_SLPI,
                (uint8_t*)&req.body.srvrqst.spistrLen);

    pos += slp::request::SIZE_SLPI;
    req.body.srvrqst.spistr = (const char*)(buff.data() + pos);

    req.body.srvrqst.spistrLen = endian::from_network<uint16_t>
                                 (req.body.srvrqst.spistrLen);

    return std::make_tuple((int)slp::SUCCESS,
                           std::move(req));
}

std::tuple<int, slp::Message> slp::Parser::parseBuffer(const buffer& buff)
{
    slp::Message req;
    int rc = slp::SUCCESS;
    /* parse the header first */
    std::tie(rc, req) = slp::Parser::parseHeader(buff);
    if (!rc)
    {
        /* switch on the function id to parse the body */
        switch (req.header.functionID)
        {
            case (uint8_t)FunctionType::SLP_FUNCT_SRVTYPERQST:
                std::tie(rc, req) = slp::Parser::parseSrvTypeRqst(buff);
                break;
            case (uint8_t)FunctionType::SLP_FUNCT_SRVRQST:
                std::tie(rc, req) = slp::Parser::parseSrvRqst(buff);
                break;
            default:
                rc = (int)slp::Error::MSG_NOT_SUPPORTED;
        }
    }
    return std::make_tuple(rc, std::move(req));
}
