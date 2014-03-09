#ifndef CONFIG_H
#define CONFIG_H

#include "server.h"

#define DEFAULT_PORT 9090
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_USER "www"
#define DEFAULT_ROOT "www"

void config_init (int argc, char const *argv[], Server *srv);

#ifdef _DEBUG
void config_dump (Server *srv);
#endif

void config_load (char *filename, Server *srv);

#endif
