#ifndef LOG_H
#define LOG_H

#include "server.h"

#define DEBUG 0
#define INFO 1
#define NOTICE 2
#define WARNING 3
#define ERROR 4

void server_log(int level, const char *fmt, ...);
void access_log (Client *cli);

#endif