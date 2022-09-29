#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <linux/can.h>
#include "crtd.h"

int extract_can_crtd(struct can_frame *f, void *p) {
  int bus, len;
  crtd_state_t *s;

  s = (crtd_state_t *) p; 
  f->can_id = s->id;
  len = s->nb;
  bus = s->bus;
  f->can_dlc = len;
  memcpy(f->data, s->bytes, len);
  state_reset_crtd(s);
  return (1+bus); /* return bus no. + 1 */
}

void state_reset_crtd(crtd_state_t *p) {  
  p->major = 0;
  p->read = 0;
  p->stamp1 = 0;
  p->stamp2 = 0;
  p->ns = 0;
  p->bus = 0;
  p->extended = 0;
  p->id = 0;
  p->nb = 0;
  memset(p->intstr, '\0', CRTD_STAMP_LENGTH);
}

void state_init_crtd(void *p) {
  state_reset_crtd((crtd_state_t *)p);
  ((crtd_state_t *)p)->discarded = 0;
}

void state_else_crtd(crtd_state_t *p, uint8_t b) {
  if (b == '\n') {
      p->discarded += p->read;
      state_reset_crtd(p);
  }
  else {
    p->major = -1;
  }
}

int state_next_crtd(void *p0, uint8_t b) {
  crtd_state_t *p;
  int res;
  unsigned long int ul;

  res = 0;
  p = (crtd_state_t *)p0;
  p->read++;
  switch(p->major) {
  case -1: /* discard */
    if (b == '\n') {
      p->discarded += p->read;
      state_reset_crtd(p);
    }
    break;
  case 0: /* init */
    if (isdigit(b)) {
      p->intstr[0] = b;
      p->ns = 1;
      p->major = 1;
    }
    else if (b != ' ') {
      state_else_crtd(p, b);
    }
    break;
  case 1: /* int1 */
    if (isdigit(b) && p->ns < 10) {
      p->intstr[p->ns] = b;
      p->ns++;
    }
    else if (b == '.') {
      p->stamp1 = atoi(p->intstr);
      p->ns = 0;
      memset(p->intstr, '\0', CRTD_STAMP_LENGTH);
      p->major = 3;
    }
    else if (b == 'r' || b == 'R') {
      p->bus = atoi(p->intstr) -1;
      p->ns = 0;
      memset(p->intstr, '\0', CRTD_STAMP_LENGTH);
      if (p->bus > 15) {
        state_else_crtd(p,b);        
      }
      else {
        p->major = 2;
      }
    }
    else if (b != ' ') {
      state_else_crtd(p, b);
    }
    break; 
  case 2: /* type0 */
    if (b == '1') {
      p->major = 4;
    }
    else if (b == '2') {
      p->major = 5;
      p->extended = 1;
    }
    else {
      state_else_crtd(p,b);        
    }
    break;
  case 3: /* int2 */
    if (isdigit(b) && p->ns<6) {
      p->intstr[p->ns] = b;
      p->ns++;
    }
    else if (b == ' ') {
      p->stamp2 = atoi(p->intstr);
      p->ns = 0;
      memset(p->intstr, '\0', CRTD_STAMP_LENGTH);
      p->major = 6;
    }
    else {
      state_else_crtd(p,b);        
    }
    break;
  case 4: /* type1 base */
    if (b == '1') {
      p->major = 7;
    }
    else {
      state_else_crtd(p,b);        
    }
    break;
  case 5: /* type2 extended */
    if (b == '9') {
      p->major = 7;
    }
    else {
      state_else_crtd(p,b);        
    }    
    break;
  case 6: /* space1 */
    if (b == 'r' || b == 'R') {
      p->major = 2;
    }
    else if (isdigit(b)) {
      p->intstr[0] = b;
      p->ns = 1;
      p->major = 12;
    }
    else if (b != ' ') {
      state_else_crtd(p,b);      
    }
    break;
  case 7: /* space2 */
    if (isxdigit(b)) {
      p->intstr[0] = b;
      p->ns = 1;
      p->major = 8;
    }
    else if (b != ' ') {
      state_else_crtd(p,b);      
    }
    break;
  case 8: /* id */
    if (isxdigit(b) && p->ns < 8) {
      p->intstr[p->ns] = b;
      p->ns++;
    }
    else if (b == ' ' || b == '\n') {
      ul = strtoul(p->intstr, NULL, 16);
      if (( ul > 0x1fffffff && p->extended) || ( ul>0x7ff && !p->extended)) {
        state_else_crtd(p, b);
      }
      else {
        p->id = (unsigned int)  ul;
        p->ns = 0;
        memset(p->intstr, '\0', CRTD_STAMP_LENGTH);
        if (b == '\n') { /* no data */
          res = 1;
          p->major = 0;
        }
        else {
          p->major = 9;
        } 
      }
    }
    else {
      state_else_crtd(p, b);
    }
    break;
  case 9: /* space3 */
    if (isxdigit(b)) {
      p->intstr[0] = b;
      p->ns = 1;
      p->major = 10;
    }
    else if (b == '\n') {
      res = 1;
      p->major = 0;
    }
    else if (b != ' ') {
      state_else_crtd(p, b);
    }
    break;
  case 10: /* byte */
    if (isxdigit(b) && p->ns<2) {
      p->intstr[p->nb] = b;
      p->ns++;
    }
    else if ( (b == '\n' || b == ' ') && p->nb < 8) {
      ul = strtoul(p->intstr, NULL, 16);
      p->bytes[p->nb] = (uint8_t) ul;
      p->nb++;
      p->ns = 0;
      memset(p->intstr, '\0', CRTD_STAMP_LENGTH);
      if (b == '\n') {
        p->major = 0;
        res = 1;
      }
      else {
        p->major = 9;
      }
    }
    else {
      state_else_crtd(p, b);
    }
    break;
  case 12: /* int3 */
    if (b == 'r' || b == 'R') {
      ul =  strtoul(p->intstr, NULL, 16);
      p->ns = 0;
      memset(p->intstr, '\0', CRTD_STAMP_LENGTH);
      if (ul <16) {
        p->bus = (int) ul -1;
        p->major = 2;
      }
      else {
        state_else_crtd(p, b);
      }
    }
    else if (isdigit(b) && p->ns < 2) {
      p->intstr[p->ns] = b;
      p->ns++;
    }
    else {
      state_else_crtd(p, b);      
    }
    break;
  }
  return res;
}
