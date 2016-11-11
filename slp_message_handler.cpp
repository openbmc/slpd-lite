#include "slp.hpp"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <string.h>

#include <algorithm>

#include "endian.hpp"
#include "slp_meta.hpp"

slp::Handler::SlpServiceList slp::Handler::slpSvcLst;


slp::buffer slp::Handler::prepareHeader(const slp::Message& req)
{
    slp::buffer buff;

    uint8_t length =  slp::header::MIN_LEN +   /* 14 bytes for header     */
                      req.header.langtagLen +  /* Actual length of lang tag */
                      slp::response::SIZE_ERROR; /*  2 bytes for error code */

    buff.resize(length);

    buff[0] = req.header.version;

    //will increment the function id from 1 as reply
    buff[slp::header::FUNCTION_OFFSET] = req.header.functionID + 1;

    std::copy_n(&length, slp::header::SIZE_VERSION,
                buff.data() +
                slp::header::LENGTH_OFFSET + 2);

    uint16_t flags = endian::to_network<uint16_t>(req.header.flags);

    std::copy_n((uint8_t*)&flags, slp::header::SIZE_FLAGS,
                buff.data() +
                slp::header::FLAGS_OFFSET);

    std::copy_n(req.header.extOffset.data(), slp::header::SIZE_OFFSET,
                buff.data() +
                slp::header::EXT_OFFSET);

    uint16_t xid = endian::to_network<uint16_t>(req.header.xid);
    std::copy_n((uint8_t*)&xid, slp::header::SIZE_XID,
                buff.data() +
                slp::header::XID_OFFSET);

    uint16_t langtagLen = endian::to_network<uint16_t>(req.header.langtagLen);
    std::copy_n((uint8_t*)&langtagLen, slp::header::SIZE_LANG,
                buff.data() +
                slp::header::LANG_LEN_OFFSET);

    std::copy_n((uint8_t*)req.header.langtag.c_str(), slp::header::SIZE_LANG,
                buff.data() +
                slp::header::LANG_OFFSET);
    return buff;

}


slp::buffer slp::Handler::processError(const slp::Message& req,
                                       uint8_t err)
{
    slp::buffer buff;
    buff = prepareHeader(req);

    std::copy_n(&err, slp::response::SIZE_ERROR,
                buff.data() +
                slp::response::ERROR_OFFSET);
    return buff;

}

std::tuple<int, slp::buffer> slp::Handler::processSrvTypeRequest(
    const slp::Message& req)
{

    /*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |      Service Location header (function = SrvTypeRply = 10)    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           Error Code          |    length of <srvType-list>   |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       <srvtype--list>                         \
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */

    slp::buffer buff;

    //read the slp service info from conf and create the service type string
    SlpServiceList svcList = readSLPServiceInfo(slp::CONF_FILE);
    if (svcList.size() <= 0)
    {
        buff.resize(0);
        return std::make_tuple((int)slp::Error::INTERNAL_ERROR, buff);
    }

    std::string service;
    for_each(svcList.begin(), svcList.end(),
             [&service](const std::pair<std::string, slp::ConfData>& svc)
    {
        service += svc.first + ",";
    });

    //removing last comma
    if (service.size() > 0)
    {
        service = service.erase(service.length() - 1, 1);
    }

    buff = prepareHeader(req);

    /* Need to modify the length and the function type field of the header
     * as it is dependent on the handler of the service */

    std::cout << "service=" << service.c_str() << "\n";

    uint8_t length =  slp::header::MIN_LEN +         /* 14 bytes for header */
                      req.header.langtagLen +        /* length of lang tag */
                      slp::response::SIZE_ERROR +    /* 2 byte err code */
                      slp::response::SIZE_SERVICE +  /* 2 byte srvtype len */
                      service.length();


    buff.resize(length);

    std::copy_n(&length, slp::header::SIZE_VERSION,
                buff.data() +
                slp::header::LENGTH_OFFSET + 2);

    /* error code is already set to 0 moving to service type len */

    uint16_t serviceTypeLen = service.length();
    serviceTypeLen = endian::to_network<uint16_t>(serviceTypeLen);

    std::copy_n((uint8_t*)&serviceTypeLen, slp::response::SIZE_SERVICE,
                buff.data() +
                slp::response::SERVICE_LEN_OFFSET);

    /* service type data */
    std::copy_n((uint8_t*)service.c_str(), service.length(),
                (buff.data() +
                 slp::response::SERVICE_OFFSET));

    return std::make_tuple(slp::SUCCESS, buff);
}

std::tuple<int, slp::buffer> slp::Handler::processSrvRequest(
    const slp::Message& req)
{

    /*
          Service Reply
          0                   1                   2                   3
          0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         |        Service Location header (function = SrvRply = 2)       |
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         |        Error Code             |        URL Entry count        |
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         |       <URL Entry 1>          ...       <URL Entry N>          \
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

         URL Entry
          0                   1                   2                   3
          0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         |   Reserved    |          Lifetime             |   URL Length  |
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         |URL len, contd.|            URL (variable length)              \
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         |# of URL auths |            Auth. blocks (if any)              \
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */

    slp::buffer buff;
    //Get all the services which are registered
    SlpServiceList svcList = readSLPServiceInfo(slp::CONF_FILE);
    if (svcList.size() <= 0)
    {
        buff.resize(0);
        return std::make_tuple((int)slp::Error::INTERNAL_ERROR, buff);
    }

    //return error if serice type doesn't match
    std::string svcName = req.body.srvrqst.srvType;
    if (svcList.find(svcName) == svcList.end())
    {
        buff.resize(0);
        return std::make_tuple((int)slp::Error::INTERNAL_ERROR, buff);
    }
    //Get all the interface address
    std::list <std::string>ifaddrList = getIntfAddrs();
    if (ifaddrList.size() <= 0)
    {
        buff.resize(0);
        return std::make_tuple((int)slp::Error::INTERNAL_ERROR, buff);
    }

    buff = prepareHeader(req);
    //Calculate the length and resize the buffer
    uint8_t length =  slp::header::MIN_LEN +         /* 14 bytes for header */
                      req.header.langtagLen +        /* length of lang tag */
                      slp::response::SIZE_ERROR +    /* 2 bytes error code */
                      slp::response::SIZE_URL_COUNT; /* 2 bytes srvtype len */

    buff.resize(length);

    //Populate the url count
    uint16_t urlCount = ifaddrList.size();
    urlCount = endian::to_network<uint16_t>(urlCount);

    std::copy_n((uint8_t*)&urlCount, slp::response::SIZE_URL_COUNT,
                buff.data() +
                slp::response::URL_ENTRY_OFFSET);

    //Find the service
    slp::ConfData svc = svcList[svcName];
    //Populate the URL Entrys
    auto pos = slp::response::URL_ENTRY_OFFSET + slp::response::SIZE_URL_COUNT;
    for (auto & addr : ifaddrList)
    {
        std::stringstream url;
        url << svc.name << ":" << svc.type << "//" << addr << "," << svc.port;

        buff.resize(buff.size() +
                    slp::response::SIZE_URL_ENTRY +
                    url.str().length());

        uint8_t reserved = 0;
        uint16_t auth = 0;
        uint16_t lifetime = 5;//lifetime is 5 sec
        uint16_t urlLength = url.str().length();

        std::copy_n((uint8_t*)&reserved, slp::response::SIZE_RESERVED,
                    buff.data() + pos);

        pos += slp::response::SIZE_RESERVED;

        lifetime = endian::to_network<uint16_t>(lifetime);

        std::copy_n((uint8_t*)&lifetime, slp::response::SIZE_LIFETIME,
                    buff.data() + pos);

        pos += slp::response::SIZE_LIFETIME;

        urlLength = endian::to_network<uint16_t>(urlLength);
        std::copy_n((uint8_t*)&urlLength, slp::response::SIZE_URLLENGTH,
                    buff.data() + pos);
        pos += slp::response::SIZE_URLLENGTH;

        std::copy_n((uint8_t*)url.str().c_str(), url.str().length(),
                    buff.data() + pos);
        pos += url.str().length();

        std::copy_n((uint8_t*)&auth, slp::response::SIZE_AUTH,
                    buff.data() + pos);
        pos += slp::response::SIZE_AUTH;
    }
    uint8_t packetLength = buff.size();
    std::copy_n((uint8_t*)&packetLength, slp::header::SIZE_VERSION,
                buff.data() +
                slp::header::LENGTH_OFFSET + 2);

    return std::make_tuple((int)slp::SUCCESS, buff);
}

std::list<std::string> slp::Handler::getIntfAddrs()
{
    std::list<std::string> addrList;
    addrList.clear();

    struct ifaddrs* ifaddr, *ifa;
    // attempt to fill struct with ifaddrs
    if (getifaddrs(&ifaddr) == -1)
    {
        return addrList;
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        // walk interfaces
        if (ifa->ifa_addr == nullptr)
        {
            continue;
        }

        // get only INET interfaces not ipv6
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            // if loopback, or not running ignore
            if ((strcmp("lo", ifa->ifa_name) == 0) ||
                !(ifa->ifa_flags & IFF_RUNNING))
            {
                continue;
            }

            char tmp[INET_ADDRSTRLEN] = { 0 };

            inet_ntop(AF_INET,
                      &(((struct sockaddr_in*)(ifa->ifa_addr))->sin_addr),
                      tmp,
                      sizeof(tmp));
            addrList.emplace_back(std::string(tmp));
        }
    }
    //free pointers
    freeifaddrs(ifaddr);
    //print the list
    std::for_each(addrList.begin(), addrList.end(),
                  [](std::string & intf)
    {
        std::cout << "Interface Addresses-->" << intf.c_str() << "\n";
    });

    return addrList;
}


std::tuple<int, slp::buffer> slp::Handler::processRequest(
    const slp::Message& msg)
{
    int rc = slp::SUCCESS;
    slp::buffer resp(0);
    switch (msg.header.functionID)
    {
        case (uint8_t)FunctionType::SLP_FUNCT_SRVTYPERQST:
            std::tie(rc, resp) = Handler::processSrvTypeRequest(msg);
            break;
        case (uint8_t)FunctionType::SLP_FUNCT_SRVRQST:
            std::tie(rc, resp) = Handler::processSrvRequest(msg);
            break;
        default:
            rc = (uint8_t)slp::Error::MSG_NOT_SUPPORTED;
    }
    return std::make_tuple(rc, resp);
}

slp::Handler::SlpServiceList  slp::Handler::readSLPServiceInfo(
    const std::string& filename)
{
    /*Conf File format would be
      ServiceName serviceType Port */

    SlpServiceList svcLst;

    std::ifstream readFile(filename);
    slp::ConfData service;
    //Read all the service from the file
    while (readFile >> service)
    {
        std::string tmp = std::string("service:") + service.name;
        service.name = tmp;
        svcLst.emplace(service.name, service);
    }

    return svcLst;
}
