#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <bstring.h>

typedef struct Request {
    bstring request_method;
    bstring version;
    bstring uri;
    bstring path;
    bstring query_string;
    bstring fragment;
    bstring host;
    bstring host_name;
    bstring pattern;
    int status_code;
    int response_size;
} Request;

Request *requestNew(char *rbuf, size_t count);

#endif