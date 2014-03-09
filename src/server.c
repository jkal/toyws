//
//  server.c
//

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <signal.h>
#include <ev.h>
#include <eio.h>

#include <bstring.h>
#include <syslog.h>

#include "server.h"
#include "net.h"
#include "config.h"
#include "sendfile.h"
#include "dir.h"
#include "protocol.h"
#include "log.h"
#include "unix.h"
#include "util.h"

extern Server *srv;
struct ev_loop *loop;

ev_async eio_want_poll_notifier;
ev_async eio_done_poll_notifier;

static void on_write(struct ev_loop *loop, struct ev_io *w, int revents)
{
    Client *cli = ((Client *) (((char*) w) - offsetof(Client, ev_write)));

	if (revents & EV_WRITE) {
        serve_client(cli);
	    ev_io_stop(EV_A_ w);
	}

    access_log(cli);

    close(cli->fd);
    free(cli->req);
    free(cli);
}

static void on_read(struct ev_loop *loop, struct ev_io *w, int revents)
{
    Client *cli = ((Client*) (((char*) w) - offsetof(Client, ev_read)));
	int r = 0;
	char rbuff[MAX_BUFF];

	if (revents & EV_READ) {
		r = read(cli->fd, rbuff, MAX_BUFF);
        cli->req = requestNew(rbuff, strlen(rbuff));

        if (r < 0) {
		    server_log(ERROR, "An error occured while reading client data. r = %d", r);
            return;
        }
	}

    ev_io_stop(loop, &cli->ev_read);

    // Is the request valid?
    if (cli->req->request_method) {
        ev_io_init(&cli->ev_write, on_write, cli->fd, EV_WRITE);
        ev_io_start(loop, &cli->ev_write);
    } else {
        server_log(ERROR, "Invalid request: %s", cli->req->request_method);
        close(cli->fd);
        // TODO: Anything else here?
    }
}

static void on_accept (struct ev_loop *loop, struct ev_io *w, int revents)
{
    int client_fd, cport;
    char cip[16];
    Client *client;

    client_fd = saccept(w->fd, cip, &cport);

	client = calloc(1, sizeof(Client));
	client->fd = client_fd;
    client->remote_addr = bfromcstr(cip);
    client->remote_port = cport;

    setnonblock(client->fd);
    setnodelay(client->fd);

	ev_io_init(&client->ev_read, on_read, client->fd, EV_READ);
	ev_io_start(loop, &client->ev_read);
}

void server_destroy(Server *srv)
{
    if (srv) {
        close(srv->listen_fd);
        free(srv);
    }
}

static void on_sigint (struct ev_loop *loop, struct ev_signal *w, int revents)
{
    server_log(INFO, "Bye.");
    ev_unloop(loop, EVUNLOOP_ALL);
    server_destroy(srv);
    exit(EXIT_SUCCESS);
}

static void on_ev_syserr (const char* msg)
{
    server_log(ERROR, "%s", msg);
}


static void WantPollNotifier (struct ev_async *watcher, int revents) {
    printf("want poll notifier\n");
    fflush(stdout);

    if (eio_poll() == -1) {
        printf("eio_poller start\n");
        //ev_idle_start(EV_DEFAULT_UC_ &eio_poller);
        ev_async_send(loop, &eio_want_poll_notifier);
    }
}

static void DonePollNotifier(struct ev_async *watcher, int revents) {

  printf("done poll notifier\n");
  fflush(stdout);
  if (eio_poll() != -1) {
    printf("eio_poller stop\n");
    //ev_idle_stop(EV_DEFAULT_UC_ &eio_poller);
      ev_async_send(loop, &eio_done_poll_notifier);
  }
}

static void EIOWantPoll(void) {
  // Signal the main thread that eio_poll need to be processed.
  ev_async_send(loop, &eio_want_poll_notifier);
}


static void EIODonePoll(void) {
  // Signal the main thread that we should stop calling eio_poll().
  // from the idle watcher.
  ev_async_send(loop, &eio_done_poll_notifier);
}


int main (int argc, char const *argv[])
{
    static ev_io ev_accept;
    struct ev_signal signal_watcher;

    loop = ev_default_loop(EVBACKEND_KQUEUE);
    if (!loop) {
        server_log(ERROR, "could not initialize the default libev loop");
    }

    ev_set_syserr_cb(&on_ev_syserr);

    signal(SIGPIPE, SIG_IGN);

    srv = (Server*) malloc(sizeof(Server));

    config_init(argc, argv, srv);

    srv->listen_fd = tcp_server_init(srv->port, bdata(srv->default_hostname));

#ifdef _DEBUG
    config_dump(srv);
#endif

	ev_signal_init(&signal_watcher, on_sigint, SIGINT);
	ev_signal_start(loop, &signal_watcher);

	ev_io_init(&ev_accept, on_accept, srv->listen_fd, EV_READ);
	ev_io_start(loop, &ev_accept);

    ev_async_init(&eio_want_poll_notifier, WantPollNotifier);
    ev_async_start(loop, &eio_want_poll_notifier);

    ev_async_init(&eio_done_poll_notifier, DonePollNotifier);
    ev_async_start(loop, &eio_done_poll_notifier);

    eio_init(EIOWantPoll, EIODonePoll);

    server_log(INFO, "Server started");
    server_log(INFO, "Listening on 127.0.0.1:%d with PID %d, CTRL+C to stop", srv->port, getpid());

	ev_loop(loop, 0);

	close(srv->listen_fd);
    free(srv);
    return EXIT_SUCCESS;
}