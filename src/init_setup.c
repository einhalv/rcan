#include <stdio.h>
#include <string.h>
#include "rcan.h"

void init_setup(setup_t *s) {
  int n;
  for (n=0; n<NUM_CANS; n++) {
    s->bus_active[n] = 0;
    strcpy(s->bus_if_name[n],"\0");
  }
  s->host = NULL; 
  s->port = DEFAULT_REMOTE_PORT;
  s->host_socket_desc = -1;
}
