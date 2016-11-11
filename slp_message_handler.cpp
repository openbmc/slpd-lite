#include "slp.hpp"
#include "slp_meta.hpp"
#include <string.h>
#include <algorithm>
#include "endian.hpp"
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>

SLP::Handler::SlpServiceList SLP::Handler::slpSvcLst;


void SLP::Handler::prepareHeader(const SLP::Message& req,
                                 std::vector<uint8_t>& buff)
{

    uint8_t length =  SLP::HEADER::MIN_LEN +   /* 14 bytes for header     */
                      req.header.langtaglen +  /* Actual length of lang tag */
                      SLP::RESPONSE::SIZE_ERROR; /*  2 bytes for error code */

    buff.resize(length);

    buff[0] = req.header.version;

    //will increment the function id from 1 as reply
    buff[SLP::HEADER::FUNCTION_OFFSET] = req.header.functionid + 1;

    std::copy_n((uint8_t*)&length, SLP::HEADER::SIZE_VERSION,
                buff.data() +
                SLP::HEADER::LENGTH_OFFSET + 2);

    uint16_t flags = (endian::to_network<uint16_t>(req.header.flags));

    std::copy_n((uint8_t*)&flags, SLP::HEADER::SIZE_FLAGS,
                buff.data() +
                SLP::HEADER::FLAGS_OFFSET);

    std::copy_n((uint8_t*)req.header.extoffset.data(), SLP::HEADER::SIZE_OFFSET,
                buff.data() +
                SLP::HEADER::EXT_OFFSET);

    uint16_t xid = (endian::to_network<uint16_t>(req.header.xid));
    std::copy_n((uint8_t*)&xid, SLP::HEADER::SIZE_XID,
                buff.data() +
                SLP::HEADER::XID_OFFSET);

    uint16_t langtaglen = (endian::to_network<uint16_t>(req.header.langtaglen));
    std::copy_n((uint8_t*)&langtaglen, SLP::HEADER::SIZE_LANG,
                buff.data() +
                SLP::HEADER::LANG_LEN_OFFSET);

    std::copy_n((uint8_t*)req.header.langtag, SLP::HEADER::SIZE_LANG,
                buff.data() +
                SLP::HEADER::LANG_OFFSET);

}


void SLP::Handler::processError(const SLP::Message& req,
                                std::vector<uint8_t>& buff,
                                uint8_t err)
{

    prepareHeader(req, buff);

    std::copy_n(&err, SLP::RESPONSE::SIZE_ERROR,
                buff.data() +
                SLP::RESPONSE::ERROR_OFFSET);

}

uint8_t SLP::Handler::processSrvTypeRequest(const SLP::Message& req,
        std::vector<uint8_t>& buff)
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

    //read the slp service info from conf and create the service type string
    SlpServiceList svcList;
    if (readSLPServiceInfo(SLP::CONF_FILE, svcList) != SLP::SUCCESS)
    {
        return (uint8_t)SLP::Error::INTERNAL_ERROR;
    }

    std::string service;
    for_each(svcList.begin(), svcList.end(),
             [&service](const std::pair<std::string, SLP::ConfData>& svc)
    {
        service += svc.first + ",";
        std::cout << svc.first << std::endl;
    });

    //removing last comma
    if (service.size() > 0)
    {
        service = service.erase(service.length() - 1, 1);
    }

    prepareHeader(req, buff);

    /* Need to modify the length and the function type field of the header
     * as it is dependent on the handler of the service */

    std::cout << "service=" << service.c_str() << std::endl;

    uint8_t length =  SLP::HEADER::MIN_LEN +   /* 14 bytes for header     */
                      req.header.langtaglen +  /* Actual length of lang tag */
                      SLP::RESPONSE::SIZE_ERROR + /*  2 bytes for error code */
                      SLP::RESPONSE::SIZE_SERVICE +  /*  2 bytes for srvtype len */
                      service.length();


    buff.resize(length);

    std::copy_n((uint8_t*)&length, SLP::HEADER::SIZE_VERSION,
                buff.data() +
                SLP::HEADER::LENGTH_OFFSET + 2);

    /* error code is already set to 0 moving to service type len */

    uint16_t serviceTypeLen = service.length();
    serviceTypeLen = (endian::to_network<uint16_t>(serviceTypeLen));

    std::copy_n((uint8_t*)&serviceTypeLen, SLP::RESPONSE::SIZE_SERVICE,
                buff.data() +
                SLP::RESPONSE::SERVICE_LEN_OFFSET);

    /* service type data */
    std::copy_n((uint8_t*)service.c_str(), service.length(),
                (buff.data() +
                 SLP::RESPONSE::SERVICE_OFFSET));

    return SLP::SUCCESS;
}

uint8_t SLP::Handler::processSrvRequest(const SLP::Message& req,
                                        std::vector<uint8_t>& buff)
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

    //Get all the services which are registered
    std::string svcName = req.body.srvrqst.srvtype;
    SlpServiceList svcList;
    if (readSLPServiceInfo(SLP::CONF_FILE, svcList))
    {
        return (uint8_t)SLP::Error::INTERNAL_ERROR;
    }

    //return error if serice type doesn't match
    if (svcList.find(svcName) == svcList.end())
    {
        return (uint8_t)SLP::Error::INTERNAL_ERROR;
    }
    //Get all the interface address
    std::list <std::string>ifaddrList;
    if (getIntfAddrs(ifaddrList))
    {
        return (uint8_t)SLP::Error::INTERNAL_ERROR;
    }

    prepareHeader(req, buff);
    //Calculate the length and resize the buffer
    uint8_t length =  SLP::HEADER::MIN_LEN +   /* 14 bytes for header     */
                      req.header.langtaglen +  /* Actual length of lang tag */
                      SLP::RESPONSE::SIZE_ERROR + /*  2 bytes for error code */
                      SLP::RESPONSE::SIZE_URL_COUNT;  /*  2 bytes for srvtype len */

    buff.resize(length);

    //Populate the url count
    uint16_t urlCount = ifaddrList.size();
    urlCount = (endian::to_network<uint16_t>(urlCount));

    std::copy_n((uint8_t*)&urlCount, SLP::RESPONSE::SIZE_URL_COUNT,
                buff.data() +
                SLP::RESPONSE::URL_ENTRY_OFFSET);

    //Find the service
    SLP::ConfData svc = svcList[svcName];
    //Populate the URL Entrys
    uint8_t pos = SLP::RESPONSE::URL_ENTRY_OFFSET + SLP::RESPONSE::SIZE_URL_COUNT;
    for (auto & addr : ifaddrList)
    {
        std::stringstream url;
        url << svc.name << ":" << svc.type << "//" << addr << "," << svc.port;

        buff.resize(buff.size() +
                    SLP::RESPONSE::SIZE_URL_ENTRY +
                    url.str().length());

        uint8_t reserved = 0;
        uint16_t auth = 0;
        uint16_t lifetime = 5;//lifetime is 5 sec
        uint16_t urlLength = url.str().length();

        std::copy_n((uint8_t*)&reserved, SLP::RESPONSE::SIZE_RESERVED,
                    buff.data() + pos);

        pos += SLP::RESPONSE::SIZE_RESERVED;

        lifetime = (endian::to_network<uint16_t>(lifetime));

        std::copy_n((uint8_t*)&lifetime, SLP::RESPONSE::SIZE_LIFETIME,
                    buff.data() + pos);

        pos += SLP::RESPONSE::SIZE_LIFETIME;

        urlLength = (endian::to_network<uint16_t>(urlLength));
        std::copy_n((uint8_t*)&urlLength, SLP::RESPONSE::SIZE_URLLENGTH,
                    buff.data() + pos);
        pos += SLP::RESPONSE::SIZE_URLLENGTH;

        std::copy_n((uint8_t*)url.str().c_str(), url.str().length(),
                    buff.data() + pos);
        pos += url.str().length();

        std::copy_n((uint8_t*)&auth, SLP::RESPONSE::SIZE_AUTH,
                    buff.data() + pos);
        pos += SLP::RESPONSE::SIZE_AUTH;
    }
    uint8_t packetLength = buff.size();
    std::copy_n((uint8_t*)&packetLength, SLP::HEADER::SIZE_VERSION,
                buff.data() +
                SLP::HEADER::LENGTH_OFFSET + 2);

    return SLP::SUCCESS;
}

uint8_t SLP::Handler::getIntfAddrs(std::list<std::string>& addrList)
{

    addrList.clear();

    struct ifaddrs* ifaddr, *ifa;
    // attempt to fill struct with ifaddrs
    if (getifaddrs(&ifaddr) == -1)
    {
        return 1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        // walk interfaces
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }

        // get only INET interfaces not ipv6
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            // if loopback, or not running ignore
            if ((strcmp("lo", ifa->ifa_name) == 0) ||
                !(ifa->ifa_flags & (IFF_RUNNING)))
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
        std::cout << "Interface Addresses-->" << intf.c_str() << std::endl;
    });

    return SLP::SUCCESS;
}


uint8_t SLP::Handler::processRequest(const SLP::Message& msg,
                                     std::vector<uint8_t>& resp)
{
    uint8_t rc = SLP::SUCCESS;
    switch (msg.header.functionid)
    {
        case (uint8_t)FunctionType::SLP_FUNCT_SRVTYPERQST:
            rc = Handler::processSrvTypeRequest(msg, resp);
            break;
        case (uint8_t)FunctionType::SLP_FUNCT_SRVRQST:
            rc = Handler::processSrvRequest(msg, resp);
            break;
        default:
            rc = (uint8_t)SLP::Error::MSG_NOT_SUPPORTED;
    }
    return rc;
}

uint8_t SLP::Handler::readSLPServiceInfo(const std::string& filename,
        SlpServiceList& svcList)
{
    //Conf File format would be
    //ServiceName serviceType Port
    if (!(slpSvcLst.size()))
    {
        std::ifstream readFile(filename);
        SLP::ConfData service;
        //Read all the service from the file
        while (readFile >> service)
        {
            std::string tmp = std::string("service:") + service.name;
            service.name = tmp;
            slpSvcLst.emplace(service.name, service);
        }
    }
    svcList = slpSvcLst;
    return svcList.size() > 0 ? 0 : 1;
}
