#include <algorithm>

#include "slp.hpp"
#include "slp_meta.hpp"
#include "slp_server.hpp"
#include "sock_channel.hpp"

/* Call Back for the sd event loop */
static int requestHandler(sd_event_source* es, int fd, uint32_t revents,
                          void* userdata)
{
    int rc = slp::SUCCESS;
    timeval tv{slp::TIMEOUT,0};
    udpsocket::Channel channel(fd,tv);
    std::vector<uint8_t>recvBuff;
    slp::Message req;
    std::vector<uint8_t> resp;
    // Read the packet
    std::tie(rc, recvBuff) = channel.read();

    if (rc < 0)
    {
        std::cerr << "E> Error in Read : " << std::hex << rc << "\n";
        return rc;
    }

    switch (recvBuff[0])
    {
        case slp::VERSION_2:
            {
                std::cout << "Request" << std::endl;
                //print the buffer
                std::for_each(recvBuff.begin(), recvBuff.end(),
                        [](uint8_t& ch) {
                        std::cout << std::hex << ch << ' ' ;
                        });

                //Parse the buffer and construct the req object
                std::tie(rc,req) = slp::Parser::parseBuffer(recvBuff);
                if ( !rc )
                {
                    //Passing the req object to handler to serve it
                    std::tie(rc,resp) = slp::Handler::processRequest(req);
                }
            }
            break;
        default:
            rc = static_cast<uint8_t>(slp::Error::VER_NOT_SUPPORTED);
            break;
    }

    //if there was error during Parsing of request
    //or processing of request then handle the error.
    if( rc )
    {
        std::cout << "Failure occured rc=" << rc << std::endl;
        resp = slp::Handler::processError(req, rc);
    }

    //print and send the response
    std::cout << "Response" << std::endl;
    std::for_each(resp.begin(), resp.end(),
            [](uint8_t& ch) {
            std::cout << std::hex << ch << ' ' ;
            });

    channel.write(resp);
    return slp::SUCCESS;
}


int main(int argc, char *argv[])
{
    slp::udp::Server svr(slp::PORT,slp::ADDRESS,requestHandler);
    return svr.run();
}
