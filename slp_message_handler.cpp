#include "slp.hpp"
#include "slp_meta.hpp"
#include <string.h>
#include <algorithm>
#include "endian.hpp"

SLP::Handler::SlpServiceList SLP::Handler::slpSvcLst;


void SLP::Handler::prepareHeader(const SLP::Message& req,
                                 std::vector<uint8_t>& buff)
{

    uint8_t length =  SLP::HEADER::MIN_LEN +   /* 14 bytes for header     */
                      req.header.langtaglen +  /* Actual length of lang tag */
                      SLP::REQUEST::SIZE_ERROR; /*  2 bytes for error code */

    buff.resize(length);

    buff[0]= req.header.version;

    buff[SLP::HEADER::FUNCTION_OFFSET]=req.header.functionid;

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

    prepareHeader(req,buff);
    buff[SLP::HEADER::FUNCTION_OFFSET] = req.header.functionid+1; //reply message

    std::copy_n(&err, SLP::REQUEST::SIZE_ERROR,
                buff.data() +
                SLP::REQUEST::ERROR_OFFSET);

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
    if ( readSLPServiceInfo(SLP::CONF_FILE, svcList) != SLP::SUCCESS)
    {
        return (uint8_t)SLP::Error::INTERNAL_ERROR;
    }

    std::string service;
    for ( auto & svc : svcList )
    {
        service += "service:"+svc.first+",";
        std::cout << svc.first << std::endl;
    }
    //removing last comma
    service = service.erase(service.length()-1, 1);

    prepareHeader(req, buff);

    /* Need to modify the length and the function type field of the header
     * as it is dependent on the handler of the service */

    std::cout << "service=" << service.c_str() << std::endl;

    uint8_t length =  SLP::HEADER::MIN_LEN +   /* 14 bytes for header     */
                      req.header.langtaglen +  /* Actual length of lang tag */
                      SLP::REQUEST::SIZE_ERROR + /*  2 bytes for error code */
                      SLP::REQUEST::SIZE_SERVICE +  /*  2 bytes for srvtype len */
                      service.length();


    buff.resize(length);

    /* function type */

    buff[SLP::HEADER::FUNCTION_OFFSET] = (uint8_t)SLP::FunctionType::SLP_FUNCT_SRVTYPERPLY;

    std::copy_n((uint8_t*)&length, SLP::HEADER::SIZE_VERSION,
                buff.data() +
                SLP::HEADER::LENGTH_OFFSET + 2);

    /* error code is already set to 0 moving to service type len */

    uint16_t serviceTypeLen = service.length();
    serviceTypeLen = (endian::to_network<uint16_t>(serviceTypeLen));

    std::copy_n((uint8_t*)&serviceTypeLen, SLP::REQUEST::SIZE_SERVICE,
                buff.data() +
                SLP::REQUEST::SERVICELEN_OFFSET);

    /* service type data */
    std::copy_n((uint8_t*)service.c_str(), service.length(),
                (buff.data() +
                 SLP::REQUEST::SERVICE_OFFSET));

    return SLP::SUCCESS;
}


uint8_t SLP::Handler::processRequest(const SLP::Message& msg,
                                     std::vector<uint8_t>& resp)
{
    uint8_t rc = SLP::SUCCESS;
    switch( msg.header.functionid)
    {
    case (uint8_t)FunctionType::SLP_FUNCT_SRVTYPERQST:
        rc = Handler::processSrvTypeRequest(msg,resp);
        break;
    }
    return rc;
}

uint8_t SLP::Handler::readSLPServiceInfo(const std::string& filename,
        SlpServiceList& svcList)
{
    //Conf File format would be
    //ServiceName serviceType Port
    if ( ! (slpSvcLst.size()) )
    {
        std::ifstream readFile(filename);
        SlpInfo service;
        while(readFile >> service)
        {
            slpSvcLst.emplace(service.name,service);
        }
    }
    svcList = slpSvcLst;
    return svcList.size() > 0 ? 0:1;
}
