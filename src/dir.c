#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>

#include <bstring.h>
#include <eio.h>

#include "dir.h"
#include "strmap.h"
#include "log.h"
#include "net.h"
#include "sendfile.h"
#include "server.h"

Server *srv;

const char *HTTP_404 = "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/plain\r\n"
    "Connection: close\r\n"
    "Content-Length: 9\r\n"
    "Server: " "toyws" VERSION
    "\r\n\r\n"
    "Not Found";

const char *RESPONSE_FORMAT = "HTTP/1.1 200 OK\r\n"
    "Date: %s\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %d\r\n"
    "Last-Modified: %s\r\n"
    "ETag: %s\r\n"
    "Server: " "toyws" VERSION
    "\r\n\r\n";

const char *RFC_822_TIME = "%a, %d %b %Y %H:%M:%S GMT";


int serve_client (Client *cli)
{
    File *fr = NULL;
    bstring filepath;

    filepath = bstrcpy(srv->document_root);

    bconcat(filepath, cli->req->path);

    fr = find_file(filepath, bfromcstr("null"));

    if (biseqcstr(cli->req->request_method, "HEAD")) {
        swrite(cli->fd, bdata(fr->header), blength(fr->header));
        goto cleanup;
    }

    if (fr) {
        swrite(cli->fd, bdata(fr->header), blength(fr->header));
        eio_sendfile_sync(cli->fd, fr->fd, 0, fr->size);
    } else {
        swrite(cli->fd, (char*) HTTP_404, strlen(HTTP_404));
    }

    goto cleanup;

cleanup:
    destroy_file(fr);
    return 0;
}

File* find_file(const_bstring path, const_bstring default_type)
{
    File *fr = malloc(sizeof(File));
    const char *p = bdata(path);

    int rc = stat(p, &fr->sb);
    if (!(rc == 0)) {
        server_log(ERROR, "File stat failed: %s", bdata(path));
        return NULL;
    }

    if (S_ISDIR(fr->sb.st_mode)) {
        fr->full_path = path;
        fr->is_dir = 1;
        // FIXME
        return fr;
    }
  	
    fr->fd = open(p, O_RDONLY);
    if (fr->fd < 0) {
        server_log(ERROR, "Failed to open file but stat worked: %s", bdata(path));
    }

    fr->size = fr->sb.st_size;
    fr->loaded = time(NULL);
    fr->last_mod = bStrfTime(RFC_822_TIME, gmtime(&fr->sb.st_mtime));
    if (!fr->last_mod) {
        server_log(ERROR, "Failed to format last modified time.");
    }

    fr->content_type = get_mime(path, bfromcstr("mime/default"));
    fr->full_path = path;

    time_t now = time(NULL);

    fr->date = bStrfTime(RFC_822_TIME, gmtime(&now));
    fr->etag = bformat("%x-%x", fr->sb.st_mtime, fr->sb.st_size);
    fr->header = bformat(RESPONSE_FORMAT,
        bdata(fr->date),
        bdata(fr->content_type),
        fr->sb.st_size,
        bdata(fr->last_mod),
        bdata(fr->etag));

    return fr;
}

void destroy_file (File *file)
{
    if (file) {
        if (!file->is_dir) {
            close(file->fd);
            bdestroy(file->date);
            bdestroy(file->last_mod);
            bdestroy(file->header);
            bdestroy(file->etag);
        }

        bdestroy(file->full_path);
        free(file);
    }
}


bstring get_mime(const_bstring path, bstring default_type)
{
    bstring ext;
    int res, k;
    char type[128];

    StrMap *ctypes = NULL;

    ctypes = strmap_new(10);
    if (ctypes == NULL) {
        server_log(ERROR, "Failed to allocate strmap");
    }

    strmap_put(ctypes, ".txt",  "text/plain");
    strmap_put(ctypes, ".html", "text/html");
    strmap_put(ctypes, ".htm",  "text/html");
    strmap_put(ctypes, ".css",  "text/css");
    strmap_put(ctypes, ".js",   "application/x-javascript");
    strmap_put(ctypes, ".jpeg", "image/jpeg");
    strmap_put(ctypes, ".jpg",  "image/jpeg");
    strmap_put(ctypes, ".gif",  "image/gif");
    strmap_put(ctypes, ".png",  "image/png");
    strmap_put(ctypes, ".ico",  "image/x-icon");

    k = bstrrchr(path, '.');
    ext = bmidstr(path, k, blength(path));

    res = strmap_get(ctypes, bdata(ext), type, sizeof(type));
    if (res == 0) {
        server_log(WARNING, "Content-Type for \"%s\" not found. Using default type.", ext);
        return default_type;
    }

    strmap_delete(ctypes);

    return bfromcstr(type);
}
