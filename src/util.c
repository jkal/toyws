//
//  util.c
//  stuff that don't fit elsewhere
//

#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>

#include "util.h"
#include "log.h"

void usage (void)
{
    printf("Usage: [-R web-root] [-i] [-H] [-h] [-u user] [-l logfile] [-p port]\n\n"
	    "\t -r : document root\n"
	    "\t -h : help\n"
	    "\t -u : run as this user\n"
	    "\t -l : logfile, default\n"
	    "\t -p : port\n"
    );
}

int file_exists(char *filename)
{
	struct stat st;
 
  	if (stat(filename, &st) || is_directory(filename)) return 0;

    return 1;
}
