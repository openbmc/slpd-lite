#include "slp_server.hpp"
#include "sock_channel.hpp"
#include "slp.hpp"
#include "slp_meta.hpp"
#include <algorithm>

/* Call Back for the sd event loop */
static int requestHandler(sd_event_source* es, int fd, uint32_t revents,
                          void* userdata)
{
    uint8_t rc = SLP::SUCCESS;
    SocketChannel channel(fd);
    std::vector<uint8_t>recvBuff;
    SLP::Message req;
    std::vector<uint8_t> resp;

    if (channel.read(recvBuff) != SLP::SUCCESS)
    {
        std::cout << "Unable to read the data" << std::endl;
        return -errno;
    }
    switch (recvBuff[0])
    {
        case SLP::VERSION_2:
            {
                std::cout << "Request" << std::endl;
                //print the buffer
                std::for_each(recvBuff.begin(), recvBuff.end(),
                        [](uint8_t& ch) {
                        std::cout << std::hex << ch << ' ' ;
                        });

                //Parse the buffer and construct the req object
                rc = SLP::Parser::parseBuffer(recvBuff, req);
                if ( !rc )
                {
                    //Passing the req object to handler to serve it
                    rc = SLP::Handler::processRequest(req, resp);
                }
            }
            break;
        default:
            rc = static_cast<uint8_t>(SLP::Error::VER_NOT_SUPPORTED);
            break;
    }

    //if there was error during Parsing of request
    //or processing of request then handle the error.
    if( rc )
    {
        std::cout << "Failure occured rc=" << rc << std::endl;
        SLP::Handler::processError(req, resp, rc);
    }

    //print and send the response
    std::cout << "Response" << std::endl;
    std::for_each(resp.begin(), resp.end(),
            [](uint8_t& ch) {
            std::cout << std::hex << ch << ' ' ;
            });

    channel.write(resp);
    return SLP::SUCCESS;
}


int main(int argc, char *argv[])
{
    SLP::UDP::Server svr(slp::PORT,slp::ADDRESS,requestHandler);
    return svr.run();
}
