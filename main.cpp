#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#define EINTRWRAP(var, op)                      \
    do {                                        \
        var = op;                               \
    } while (var == -1 && errno == EINTR);

int main(int argc, char **argv)
{
    short port = 8888;
    for (int i=1; i<argc; ++i) {
        if ((!strcmp(argv[i], "--port") || !strcmp(argv[i], "-p")) && i + 1 < argc) {
            port = atoi(argv[++i]);
        }
    }
    const int fd = ::socket(AF_INET6, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("::socket failed %d", errno);
        return 1;
    }

    int reuseaddr = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int))) {
        printf("::setsockopt failed %d", errno);
        int r;
        EINTRWRAP(r, ::close(fd));
        return 1;
    }

    sockaddr_in6 address;
    ::memset(&address, 0, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(port);
    address.sin6_addr = in6addr_any;
    if (::bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address))) {
        printf("::bind failed %d", errno);
        int r;
        EINTRWRAP(r, ::close(fd));
        return 1;
    }
    enum { Backlog = 128 }; // ### configurable?
    if (::listen(fd, Backlog)) {
        int r;
        EINTRWRAP(r, ::close(fd));
        printf("::listen failed %d", errno);
        return 1;
    }
    while (true) {
        fd_set r;
        FD_ZERO(&r);
        FD_SET(fd, &r);
        int ret = select(fd + 1, &r, 0, 0, 0);
        printf("GOT A DUDE %d\n", ret);
        sleep(1);
    }
}
