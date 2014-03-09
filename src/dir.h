#ifndef DIR_H
#define DIR_H

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
#else
    #include <sys/sendfile.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bstring.h>

#include "server.h"

typedef struct File {
    int is_dir;
    int fd;
    time_t loaded;
    bstring date;
    bstring last_mod;
    bstring content_type;
    bstring header;
    bstring request_path;
    bstring full_path;
    bstring etag;
    struct stat sb;
    off_t size;
} File;

typedef struct Dir {
    bstring base;
    bstring normalized_base;
    bstring index_file;
    bstring default_ctype;
} Dir;

File * find_file(const_bstring path, const_bstring default_type);
void destroy_file(File *file);
bstring get_mime (const_bstring path, bstring default_type);
int serve_client(Client *cli);

#endif
