#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <limits.h>
#include <stdlib.h>

#include "unix.h"
#include "log.h"
#include "util.h"


bstring get_cwd(void)
{
    char *wd = calloc(PATH_MAX + 1, 1);
    bstring dir = NULL;

    if (!getcwd(wd, PATH_MAX-1)) {
        server_log(ERROR, "could not get current working directory.");   
        free(wd);
        return NULL;
    }
    
    wd[PATH_MAX] = '\0';

    dir = bfromcstr(wd);

    free(wd);
    return dir;
}

int uchroot(bstring path)
{
    int rc = 0;
    const char *to_dir = bdata(path);

    check(to_dir && blength(path) > 0, "invalid or empty chroot path");

    rc = chroot(to_dir);
    if (rc != 0) {
        server_log(ERROR, "failed to chroot to %s", bdata(path));
    }

    rc = chdir("/");
    if (rc != 0) {
        server_log(ERROR, "failed to chdir to /.");
    }

    return 0;
}

int droppriv(bstring path)
{
    const char *from_dir = bdata(path);
    struct stat sb;

    check(from_dir && blength(path), "path can't be empty.");

    int rc = stat(from_dir, &sb);
    check(rc == 0, "stat failed: %s", bdata(path));

    rc = setregid(sb.st_gid, sb.st_gid);
    check(rc == 0 && getgid() == sb.st_gid && getegid() == sb.st_gid, "failed to change gid");

    rc = setreuid(sb.st_uid, sb.st_uid);
    check(rc == 0 && getuid() == sb.st_uid && geteuid() == sb.st_uid, "failed to change uid");

    return 0;
}

int daemonize(void)
{
    int rc = daemon(0, 1);
    if (rc != 0) {
        server_log(ERROR, "daemon() failed.");
        exit(-1);
    }

    return 0;
}
