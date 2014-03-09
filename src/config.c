#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <bstring.h>

#include "config.h"
#include "server.h"
#include "util.h"
#include "log.h"
#include "unix.h"

#ifdef _DEBUG
void config_dump (Server *srv)
{
    server_log(INFO, "Server configuration:");
    printf("default_host: %s\n", bdata(srv->default_hostname));
    printf("port: %d\n", srv->port);
    printf("document_root: %s\n", bdata(srv->document_root));
    printf("user: %s\n", bdata(srv->user));
    printf("chroot: %s\n", bdata(srv->chroot));
    printf("access_log: %s\n", bdata(srv->access_log));
    printf("error_log: %s\n", bdata(srv->error_log));
    printf("pid_file: %s\n", bdata(srv->pid_file));
    printf("daemonize: %s\n", srv->daemonize ? "True" : "False");
    fflush(stdout);
}
#endif

void config_init (int argc, char const *argv[], Server *srv)
{
    int c = 0;

    srv->port = DEFAULT_PORT;
    srv->default_hostname = bfromcstr(DEFAULT_HOST);
    srv->document_root = get_cwd();
    srv->user = bfromcstr(DEFAULT_USER);

    while ((c = getopt(argc, (char* const*) argv, "iHhR:p:l:L:u:n:")) != -1) {
        switch (c) {
		case 'r':
            srv->document_root = bfromcstr(optarg);
			break;
		case 'u':
            srv->user = bfromcstr(optarg);
			break;
		case 'p':
            srv->port = strtoul(optarg, NULL, 10);
			break;
		case 'l':
            srv->access_log = bfromcstr(optarg);
			break;
		case 'h':
		default:
		    usage();
            exit(EXIT_FAILURE);
		}
	}
}
