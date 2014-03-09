#ifndef UNIX_H
#define UNIX_H

#include <unistd.h>

#include "bstring.h"

bstring get_cwd (void);
int uchroot (bstring path);
int droppriv (bstring path);
int daemonize(void);

#endif
