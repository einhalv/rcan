#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <linux/can.h>
#include "rcan.h"
#include "gvret.h"
#include "crtd.h"

void *upstream(void *p) {
  setup_t *s;
  int ii;
  int host_desc;
  int can_desc[NUM_CANS];
  int can_busno[NUM_CANS];
  int num_active;
  struct epoll_event ev, events[NUM_CANS];
  int epollfd, nfds;
  int bus, sock, nread;
  uint n, nd;
  struct can_frame frame;
  uint8_t data[MAXFRAMELENGTH];
  int (*upstream_frame) (struct can_frame *, int, uint8_t  *);

  s = (setup_t *) p;
  if (!strcmp(s->can_format, "gvretb")) {
      upstream_frame = gvret_frame;
    }
  else if (!strcmp(s->can_format, "crtd")) {
      upstream_frame = crtd_frame;
    }
  else {
    pthread_exit((void *) 99); 
  }
  host_desc = s->host_socket_desc;
  num_active=0;
  for (ii=0; ii<NUM_CANS; ii++)
    if (s->bus_active[ii]) {
      can_busno[num_active] = ii;
      can_desc[num_active] = s->bus_socket_desc[ii];
      num_active++;
    }

  epollfd=epoll_create1(0);
  if (epollfd == -1) {
    pthread_exit((void *) 1); 
  }

  for (ii=0; ii<num_active; ii++) {
    ev.events = EPOLLIN;
    ev.data.u32 = ii;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, can_desc[ii], &ev) == -1) {
      pthread_exit((void *) 2);
    }
  }

  while (1) {
    nfds = epoll_wait(epollfd, events, num_active, -1);
    if (nfds == -1) {
      pthread_exit((void *) 3);
    }
    for (ii=0; ii<nfds; ii++) {
      n = events[ii].data.u32;
      sock = can_desc[n];
      bus = can_busno[n];
      nread = read(sock, &frame, sizeof(struct can_frame));
      if (nread < 0) {
        pthread_exit((void *) 4);
      }
      else {
        nd=upstream_frame(&frame, bus, data);
        if (write(host_desc, data, nd)==-1) {
          pthread_exit((void *) 5);
        }
      }
    }
  }
  pthread_exit((void *)6); /* insane bug */
}
