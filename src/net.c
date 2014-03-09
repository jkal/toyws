//
//  net.c
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#include "net.h"
#include "log.h"

int setnonblock (int fd)
{
    int flags;
    
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        server_log(ERROR, "fcntl(F_GETFL): %s\n", strerror(errno));
        return NET_ERR;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        server_log(ERROR, "fcntl(F_SETFL,O_NONBLOCK): %s\n", strerror(errno));
        return NET_ERR;
    }
    return NET_OK;
}

int setnodelay (int fd)
{
    int yes = 1;
    
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1) {
        server_log(ERROR, "setsockopt TCP_NODELAY: %s\n", strerror(errno));
        return NET_ERR;
    }
    return NET_OK;
}

int setsndbuffer (int fd, int buffsize)
{
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(buffsize)) == -1) {
        server_log(ERROR, "setsockopt SO_SNDBUF: %s\n", strerror(errno));
        return NET_ERR;
    }
    return NET_OK;
}

int setkeepalive (int fd)
{
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1) {
        server_log(ERROR, "setsockopt SO_KEEPALIVE: %s\n", strerror(errno));
        return NET_ERR;
    }
    return NET_OK;
}

int sread (int fd, char *buf, int count)
{
    int nread, totlen = 0;
    while (totlen != count) {
        nread = read(fd, buf, count - totlen);
        if (nread == 0) return totlen;
        if (nread == -1) return -1;
        totlen += nread;
        buf += nread;
    }
    return totlen;
}

int swrite (int fd, char *buf, int count)
{
    int nwritten, totlen = 0;
    while(totlen != count) {
        nwritten = write(fd,buf,count-totlen);
        if (nwritten == 0) return totlen;
        if (nwritten == -1) return -1;
        totlen += nwritten;
        buf += nwritten;
    }
    return totlen;
}

int tcp_server_init (int port, char *bindaddr)
{
    int s, on = 1;
    struct sockaddr_in sa;
    
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        server_log(ERROR, "socket: %s\n", strerror(errno));
        return NET_ERR;
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        server_log(ERROR, "setsockopt SO_REUSEADDR: %s\n", strerror(errno));
        close(s);
        return NET_ERR;
    }
    memset(&sa,0,sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bindaddr) {
        if (inet_aton(bindaddr, &sa.sin_addr) == 0) {
            server_log(ERROR, "Invalid bind address\n");
            close(s);
            return NET_ERR;
        }
    }
    if (bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        server_log(ERROR, "bind: %s\n", strerror(errno));
        close(s);
        return NET_ERR;
    }
    if (listen(s, 511) == -1) { /* the magic 511 constant is from nginx */
        server_log(ERROR, "listen: %s\n", strerror(errno));
        close(s);
        return NET_ERR;
    }
    return s;
}

int saccept (int serversock, char *ip, int *port)
{
    int fd;
    struct sockaddr_in sa;
    unsigned int sa_len;

    while (1) {
        sa_len = sizeof(sa);
        fd = accept(serversock, (struct sockaddr*)&sa, &sa_len);
        if (fd == -1) {
            if (errno == EINTR)
                continue;
            else {
                server_log(ERROR, "accept: %s\n", strerror(errno));
                return NET_ERR;
            }
        }
        break;
    }
    if (ip) strcpy(ip,inet_ntoa(sa.sin_addr));
    if (port) *port = ntohs(sa.sin_port);
    return fd;
}
