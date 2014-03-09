#ifndef _SENDFILE_H
#define _SENDFILE_H

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/uio.h>
#else
    #include <sys/sendfile.h>
#endif

int mysendfile(int out_fd, int in_fd, off_t offset, size_t count);
int mmap_stream(int out_fd, int in_fd, off_t offset, size_t count);
ssize_t sys_sendfile(int ofd, int ifd, off_t offset, size_t size);

#endif
