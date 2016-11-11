#pragma once

#include <iostream>
#include <string>
#include <sys/types.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-daemon.h>
#include <systemd/sd-event.h>

#include "slp_meta.hpp"

namespace slp
{
namespace udp
{
class Server
{
        uint16_t port;
        std::string address;
        using callback = int (*)(sd_event_source*, int, uint32_t, void*);
        callback callme;

    public:

        Server(): port(slp::PORT), address(slp::ADDRESS), callme(nullptr) {};

        Server(uint16_t port, std::string address, callback cb):
            port(port),
            address(address),
            callme(cb) {};

        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        Server(Server&&) = delete;
        Server& operator=(Server &&) = delete;


        void setPort(uint16_t port)
        {
            this->port = port;
        };
        void setAddress(std::string address)
        {
            this->address = address;
        };
        void setCallBack(Server::callback wakeme)
        {
            this->callme = wakeme;
        };

        int run();

};
}//namespce udp
}//namespace slp
