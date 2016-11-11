#pragma once

#include <iostream>
#include <string>
#include <sys/types.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-daemon.h>
#include <systemd/sd-event.h>

#include "slp_meta.hpp"
#include "slp.hpp"

namespace slp
{

namespace udp
{
/** General udp server which waits for the POLLIN event
    on the port and calls the call back once it gets the event.
    usage would be create the server with the port and the call back
    and call the run method.
 */
class Server
{
    private:

        using callback = int (*)(sd_event_source*, int, uint32_t, void*);

    public:

        Server(): Server(slp::PORT, nullptr) {};

        Server(uint16_t port, callback cb):
            port(port),
            callme(cb) {};

        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        Server(Server&&) = default;
        Server& operator=(Server &&) = default;

        uint16_t port;
        callback callme;

        int run();

};
}//namespce udp
}//namespace slp
