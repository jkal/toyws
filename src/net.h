#ifndef ANET_H
#define ANET_H

#define NET_OK 0
#define NET_ERR -1
#define NET_ERR_LEN 256


int setnonblock (int fd);
int setnodelay (int fd);
int setkeepalive (int fd);

int saccept (int serversock, char *ip, int *port);

int sread (int fd, char *buf, int count);
int swrite (int fd, char *buf, int count);

int tcp_server_init (int port, char *bindaddr);

#endif
