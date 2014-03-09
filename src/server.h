#ifndef SERVER_H
#define SERVER_H

#include <stdarg.h>
#include <stdbool.h>
#include <ev.h>
#include <bstring.h>

#include "protocol.h"

#define MAX_BUFF 2048
#define VERSION "0.0"

typedef struct Server {
    unsigned short port;
    unsigned int listen_fd;
    bstring user;
    bstring document_root;
    bstring chroot;
    bstring access_log;
    bstring error_log;
    bstring pid_file;
    bstring default_hostname;
    unsigned short verbosity;
    bool daemonize;
} Server;

typedef struct Client {
    ev_io ev_write;
	ev_io ev_read;
    int fd;
    int retry;
    int remote_port;
    bstring remote_addr;
    bstring input_header;
    bstring input_body;
    size_t input_pos;
    Request *req;
} Client;

#endif
