#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <bstring.h>

#include "log.h"
#include "server.h"
#include "util.h"

Server *srv;

#define LOG_DEBUG(FMT, ...) LOG(LogDebugLevel, "DEBUG", FMT, ##__VA_ARGS__)
#define LOG_INFO(FMT, ...) LOG(LogInfoLevel, "INFO", FMT, ##__VA_ARGS__)
#define LOG_ERROR(FMT, ...) LOG(LogErrorLevel, "ERROR", FMT, ##__VA_ARGS__)
#define LOG(LOG_LEVEL, LEVEL_NAME, FMT, ...) { \
    if (LOG_LEVEL >= log_level) {\
        fprintf(stderr, "%s:%d:%s\t%s\t" FMT "\n", __FILE__, __LINE__, __func__, LEVEL_NAME, ##__VA_ARGS__);\
    }\
}


void server_log (int level, const char *fmt, ...) 
{
    va_list ap;
    FILE *fp;
    
    fp = stderr;

#ifndef _DEBUG
    if (level == DEBUG) {
        return;
    }
#endif
    
    va_start (ap, fmt);
    
    if (level == INFO) {
        fprintf(fp, COLOR_BLUE "\r==> " COLOR_RESET);
    } else if (level == WARNING) {
        fprintf(fp, COLOR_YELLOW "\r==> " COLOR_RESET);
    } else if (level == ERROR) {
        fprintf(fp, COLOR_RED "\r==> " COLOR_RESET);
    } else {
        fprintf(fp, COLOR_CYAN "\r==> " COLOR_RESET);
    }
    
    vfprintf(fp, fmt, ap);
    fprintf(fp, "\n");
    fflush(fp);
    va_end(ap);
}

static void format_log (int level, const char *fmt, ...) 
{
    va_list ap;
    FILE *fp;

    fp = (srv->access_log == NULL) ? stdout : fopen(bdata(srv->access_log), "a");
    
    if (!fp)
        return;

    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    fprintf(fp,"\n");
    fflush(fp);
    va_end(ap);

    if (srv->access_log) 
        fclose(fp);
}

// http://en.wikipedia.org/wiki/Common_Log_Format
void access_log (Client *cli)
{
    const char *LOG_TIME = "[%d/%b/%Y %H:%M:%S]";
    time_t now = time(NULL);
    bstring logdate = bStrfTime(LOG_TIME, gmtime(&now));
	
    format_log(0, "%s - - %s \"%s %s HTTP/%s\"",
        bdata(cli->remote_addr),
        bdata(logdate),
        bdata(cli->req->request_method),
        bdata(cli->req->path),
        bdata(cli->req->version)
    );
}
