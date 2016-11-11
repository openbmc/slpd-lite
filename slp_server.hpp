#pragma once
#include <iostream>
#include <string>
#include <sys/types.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-daemon.h>
#include <systemd/sd-event.h>

namespace SLP
{
    namespace UDP
    {
        class Server
        {
            uint16_t port;
            std::string address;
            using CALLBACK = int (*) ( sd_event_source*, int, uint32_t, void*);
            CALLBACK callme;

            public:

            Server():port(427),address("0.0.0.0"),callme(nullptr) {};

            Server(uint16_t port,std::string address,CALLBACK callback):
                port(port),
                address(address),
                callme(callback) {};

            Server(const Server&) = delete;
            Server& operator=(const Server&) = delete;
            Server(Server&&) = delete;
            Server& operator=(Server&&) = delete;


            void setPort(uint16_t port) {
                this->port = port;
            };
            void setAddress(std::string address) {
                this->address = address;
            };
            void setCallBack(Server::CALLBACK wakeme) {
                this->callme = wakeme;
            };

            int run();

        };
    }
}
