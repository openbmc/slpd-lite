#include "slp_server.hpp"

#include <memory>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sock_channel.hpp"

/** General udp server which waits for the POLLIN event
    on the port and calls the call back once it gets the event.
    usage would be create the server with the port and the call back
    and call the run method.
 */
int slp::udp::Server::run()
{
    struct sockaddr_in in {};

    sd_event_source* eventSource = nullptr;
    sd_event* event = nullptr;

    slp::deleted_unique_ptr<sd_event> eventPtr(event, [](sd_event * event)
    {
        event = sd_event_unref(event);
    });

    slp::deleted_unique_ptr<sd_event_source> eventSourcePtr(eventSource, [](
                sd_event_source * event)
    {
        event = sd_event_source_unref(event);
    });

    int fd = -1, r;
    sigset_t ss;

    r = sd_event_default(&event);
    if (r < 0)
    {
        goto finish;
    }
    eventPtr.reset(event);
    event = nullptr;

    if (sigemptyset(&ss) < 0 || sigaddset(&ss, SIGTERM) < 0 ||
        sigaddset(&ss, SIGINT) < 0)
    {
        r = -errno;
        goto finish;
    }
    /* Block SIGTERM first, so that the event loop can handle it */
    if (sigprocmask(SIG_BLOCK, &ss, NULL) < 0)
    {
        r = -errno;
        goto finish;
    }

    /* Let's make use of the default handler and "floating" reference features of sd_event_add_signal() */
    r = sd_event_add_signal(eventPtr.get(), NULL, SIGTERM, NULL, NULL);
    if (r < 0)
    {
        goto finish;
    }

    r = sd_event_add_signal(eventPtr.get(), NULL, SIGINT, NULL, NULL);
    if (r < 0)
    {
        goto finish;
    }

    fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (fd < 0)
    {
        r = -errno;
        goto finish;
    }

    in.sin_family = AF_INET;
    in.sin_port = htons(this->port);
    in.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr*)&in, sizeof(in)) < 0)
    {
        r = -errno;
        goto finish;
    }

    r = sd_event_add_io(eventPtr.get(), &eventSource, fd, EPOLLIN, this->callme,
                        NULL);
    if (r < 0)
    {
        goto finish;
    }
    eventSourcePtr.reset(eventSource);
    eventSource = nullptr;

    r = sd_event_loop(eventPtr.get());

finish:

    if (fd >= 0)
    {
        (void) close(fd);
    }

    if (r < 0)
    {
        fprintf(stderr, "Failure: %s\n", strerror(-r));
    }

    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
