#include "slp_server.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sock_channel.hpp"

int SLP::UDP::Server::run()
{
    struct sockaddr_in in {};

    sd_event_source* event_source = nullptr;
    sd_event* event = nullptr;
    int fd = -1, r;
    sigset_t ss;

    r = sd_event_default(&event);
    if (r < 0)
    {
        goto finish;
    }

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
    r = sd_event_add_signal(event, NULL, SIGTERM, NULL, NULL);
    if (r < 0)
    {
        goto finish;
    }

    r = sd_event_add_signal(event, NULL, SIGINT, NULL, NULL);
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

    r = sd_event_add_io(event, &event_source, fd, EPOLLIN,this->callme, NULL);
    if (r < 0)
    {
        goto finish;
    }

    r = sd_event_loop(event);

finish:
    event_source = sd_event_source_unref(event_source);
    event = sd_event_unref(event);

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
