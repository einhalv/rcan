#ifndef _DOWNSTREAM_H_
#define _DOWNSTREAM_H_
#include "gvret.h"
#include "crtd.h"

#define BUFLEN 1024

union state {
  gvret_state_t gvret;
  crtd_state_t crtd;
};

#endif
