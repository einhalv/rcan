#ifndef _GVRET_H_
#define _GVRET_H_ 1
#include <stdint.h>
#include <linux/can.h>

typedef struct{
  int major;
  int minor;
  uint8_t data[28];
  int numbytes;
  int discarded;
} gvret_state_t;

int gvret_frame(struct can_frame *f, int bus, uint8_t  *d);
void state_reset_gvret(gvret_state_t *p);
void state_init_gvret(void *p);
int state_next_gvret(void *p, uint8_t b);
int extract_can_gvret(struct can_frame *f, void  *s);

#endif
