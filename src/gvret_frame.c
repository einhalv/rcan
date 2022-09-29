#include <stdint.h>
#include <string.h>
#include <linux/can.h>
#include "gvret.h"

int gvret_frame(struct can_frame *f, int bus, uint8_t  *d) {
  uint32_t id;
  uint8_t dlc;
  
  id = f->can_id;
  dlc = f->can_dlc;
  d[0] = 0xf1;
  d[1] = 0;
  d[2] = id % 256;
  id /= 256;
  d[3] = id % 256;
  id /= 256;
  d[4] = id % 256;
  id /= 256;
  d[5] = id % 256;
  d[6] = bus & 0xff;
  d[7] = dlc;
  memcpy(d+8, f->data, dlc);
  return 8 + dlc;
}
