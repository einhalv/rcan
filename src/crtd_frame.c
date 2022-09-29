#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <linux/can.h>
#include "crtd.h"

int crtd_frame(struct can_frame *f, int bus, uint8_t  *d) {
  int n;
  char str[64], *s;

  s = (char *) d;
  if (f->can_id & 0x80000000) {
    sprintf(s, "0.000 %dT29 %08x ", bus+1, f->can_id & 0x1fffffff);
  }
  else { 
    sprintf(s, "0.000 %dT11 %03x ", bus+1, f->can_id & 0x1fffffff);
  }
  for (n=0; n<f->can_dlc; n++) {
    sprintf(str, "%02x ", f->data[n]);
    strcat(s, str);
  }
  n = strlen(s);
  s[n-1] = '\n';
  return strlen(s);
}
