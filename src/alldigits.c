#include <ctype.h>
#include <string.h>
#include "rcan.h"

int alldigits(char *p);

int alldigits(char *p) {
  int a=1;
  
  for (; *p; p++) {
    a = a && isdigit(*p);
  }
  return a;
}
