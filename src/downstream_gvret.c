#include <string.h>
#include "gvret.h"

const int gvret_numbytes[] = {12, 6, 4, 11, 0, 0, 12, 8, 0, 4, 0, 12};

int extract_can_gvret(struct can_frame *f, void *p) {
  int bus, len;
  gvret_state_t *s;

  s = (gvret_state_t *) p; 
  if ((s->data[0] != 0xf1) || (s->data[1] != 0) || (s->data[s->minor-1])) {
    /* discard data */
    s->discarded += s->minor;
    state_reset_gvret(s);
    return(0);
  }
  /* 
    timestamp = s->data[2] + (s->data[3] << 8) +
                (s->data[4] << 16) + (s->data[5] << 24);
  */  
  f->can_id = s->data[6] + (s->data[7] << 8)
    + (s->data[8] << 16) + (s->data[9] << 24);
  len = s->data[10] & 0xf;
  bus = s->data[10] >> 28;
  f->can_dlc = len;
  memcpy(f->data, s->data + 11, len);
  state_reset_gvret(s);
  return(1+bus); /* return bus no. + 1 */
}

void state_reset_gvret(gvret_state_t *p) {
  p->major = -2; /* -2, -1, 0-11 */
  p->minor = 0;
  p->numbytes = 0;
}

void state_init_gvret(void *p) {
  ((gvret_state_t *)p)->discarded = 0;
  state_reset_gvret((gvret_state_t *)p);
}

int state_next_gvret(void *p0, uint8_t b) {
  int result;
  int discarded;
  gvret_state_t *p;

  p = (gvret_state_t *) p0;
  result = 0;
  if (p->major == -2) {
    if (b == 0xf1) {
      p->major = -1;
      p->data[0] = b;
      p->minor = 1;
    }
    else {
      p->discarded += 1;
    }
  }
  else if ( p->major == -1) {
    if (b>=0 && b<12) {
      p->minor = 2;
      p->major = b;
      p->numbytes = gvret_numbytes[b];
      p->data[1] = b;
    }
    else {
      discarded = p->discarded + p->minor + 1;
      state_init_gvret(p);
      p->discarded = discarded;
    }
  }
  else {
    p->data[p->minor] = b;
    p->minor += 1;
  }
  if (p->minor == p->numbytes) {
    if ( (p->major==0 || p->major == 11) &&
         p->numbytes == gvret_numbytes[p->major]) {
      p->numbytes += p->data[10] & 0xf;
    }
    else {
      result = 1;
    }
  }
  return result;
}
