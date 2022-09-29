#ifndef _CRTD_H_
#define _CRTD_H_ 1
#include <stdint.h>
#include <linux/can.h>

#define CRTD_STAMP_LENGTH 18

typedef struct{
  int major;
  int read;
  char intstr[CRTD_STAMP_LENGTH];
  int ns;
  int stamp1;
  int stamp2;
  int bus;
  int extended;
  unsigned int id;
  uint8_t bytes[8];
  int nb;
  int discarded;
} crtd_state_t;

void state_reset_crtd(crtd_state_t *p);
void state_init_crtd(void *p);
void state_else_crtd(crtd_state_t *p, uint8_t b);
int state_next_crtd(void *p0, uint8_t b);
int extract_can_crtd(struct can_frame *f, void  *s);
int crtd_frame(struct can_frame *f, int bus, uint8_t  *d);

#endif
