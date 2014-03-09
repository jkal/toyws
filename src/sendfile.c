#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <errno.h>


#include "sendfile.h"
#include "net.h"

ssize_t sys_sendfile(int ofd, int ifd, off_t offset, size_t size)
{
    off_t sent_bytes = size;
    int ret;

    ret = sendfile(ifd, ofd, offset, &sent_bytes, NULL, 0);
    printf("%d\n", ret);
    if (ret == -1) {
        if (errno == EAGAIN) {
            if (sent_bytes == 0) {
                // didn't send anything. return error with errno == EAGAIN.
                return -1;
            } else {
                // we sent some bytes, but they we would block.  treat this as success for now.
                offset += sent_bytes;
                return sent_bytes;
            }
        } else {
            return -1;
        }
    } else if (ret == 0) {
        offset += size;
        return size;
    } else {
        printf("don't know how to handle return %d from OS X sendfile", ret);
        return -1;
    }
}

int mysendfile(int out_fd, int in_fd, off_t offset, size_t count)
{
    off_t my_count = count;
    int rc;

#if defined(__APPLE__)
    rc = sendfile(in_fd, out_fd, offset, &my_count, NULL, 0);
    if (rc == -1) {
        if (errno == EAGAIN) {
            if (my_count == 0) {
                /* Didn't send anything. Return error with errno == EAGAIN. */
                return -1;
            } else {
                /* We sent some bytes, but they we would block.  Treat this as
                 * success for now. */
                offset += my_count;
                return my_count;
            }
        } else {
            /* some other error */
            return -1;
        }
    } else if (ret == 0) {
        offset += size;
        return size;
    } else {
        printf("don't know how to handle return %d from OS X sendfile", ret);
        return -1;
    }
#elif defined(__FreeBSD__)
    rc = sendfile(in_fd, out_fd, offset, count, NULL, &my_count, 0);
#elif defined(__linux__)
    rc = sendfile(out_fd, in_fd, &offset, count);
    if (rc == -1) {
        fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
        exit(1);
    }
    if (rc != stat_buf.st_size) {
        fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n", rc, (int)stat_buf.st_size);
        exit(1);
    }
#else
    printf(stderr, "sendfile() not supported.");
    return -1;
#endif

    if (rc == -1) {
        perror("sendfile");
        return -1;
    }

    return rc;
}

//
// mmap, for when the mapped file is large (and thus any wasted space
// is a small percentage of the total mapping)
//
int mmap_stream(int out_fd, int in_fd, off_t offset, size_t count)
{
    const int page_size = 0x1000;
    off_t off = offset;
    void *data;

    while (off < count) {
        data = mmap(NULL, page_size, PROT_READ, MAP_SHARED, in_fd, off);
        if (data == MAP_FAILED) {
            perror("mmap");
            return 1;
        }
        swrite(out_fd, (char*) data, page_size);
        if (munmap(data, page_size) == -1) {
            perror("munmap");
            return 1;
        }
        off += page_size;
    }
    return 0;
}
