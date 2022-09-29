#include <stdlib.h>
#include <string.h>
#include "rcan.h"

int splitaddrport(char *dest, const char *src) {
  char *p;
  int port=-1;
  
  p = strrchr(src, ':');
  if (p) {
    if (*(p+1)=='\0') {
      strcpy(dest, src);
      dest[strlen(src)-1] = '\0';
    }
    else if (src[0]=='[' && *(p-1) == ']') { /* ip6 with port */
      if (alldigits(p+1)) {
        memcpy(dest, src+1, p-src-2);
        *(dest+(p-src-2))='\0';
        port = atoi(p+1);
      }
      else {
        *dest = '\0';
      }
    }
    else {
      if (memchr(src, ':', p-src)) { /* bare ip6 */
        strcpy(dest, src);
      }
      else { /* ip4 with port */
        if (alldigits(p+1)) {
          memcpy(dest, src, p-src);
          *(dest+(p-src))='\0';
          port = atoi(p+1);
        }
        else {
          *dest = '\0';
        }
      }
    }
  }
  else {
    strcpy(dest,src);
  }
  return port;
}
