#include <sys/socket.h>
#include <pthread.h>
#include <stdint.h>
#include <linux/can.h>
#include <string.h>
#include <unistd.h>
#include "rcan.h"
#include "downstream.h"

void *downstream(void *p) {
  union state gu;
  void *g;
  uint8_t buf[BUFLEN];
  ssize_t nread;
  setup_t *s;
  int ii, rc;
  int host_desc;
  int can_desc[] = {-1, -1, -1, -1, -1, -1, -1, -1,
                    -1, -1, -1, -1. -1, -1, -1, -1};
  struct can_frame frame;
  int busno, nbytes;
  int (*extract_can)(struct can_frame *, void *);
  int (*state_next)(void *, uint8_t);
  
  s = (setup_t *) p;
  if (!strcmp(s->can_format, "gvretb")) {
    extract_can = extract_can_gvret;
    state_next = state_next_gvret;
    g = &(gu.gvret);
    state_init_gvret(g);
  }
  else if (!strcmp(s->can_format, "crtd")) {
    extract_can = extract_can_crtd;
    state_next = state_next_crtd;
    g = &(gu.crtd);
    state_init_crtd(g);    
  }
  else {
    pthread_exit((void *) 99); 
  }
  host_desc = s->host_socket_desc;
  for (ii=0; ii<NUM_CANS; ii++)
    if (s->bus_active[ii]) 
       can_desc[ii] = s->bus_socket_desc[ii];
  
  while (1) {
    nread = recv(host_desc, buf, BUFLEN, 0);
    if (!nread) { /* connection closed */
      pthread_exit((void *) 1);
    }
    else if (nread < 0) { /* error */
      pthread_exit((void *)2);
    }
    else { /* parse */
      for (ii=0; ii<nread; ii++) {
        rc = state_next(g, buf[ii]);
        if ( rc && (busno=extract_can(&frame, g)) ) {
          --busno;
          if (can_desc[busno]>=0) {
            nbytes = write(s->bus_socket_desc[busno], &frame,
                           sizeof(struct can_frame));
            if (nbytes < 0) { /* error */
              pthread_exit((void *)3);
            }
          }
        }
      } /* end for */
    }  
  }
  
  pthread_exit((void *)4); /* bug */
}
