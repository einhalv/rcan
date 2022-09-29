#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include "rcan.h"

extern char progname[];

int remote_address(setup_t *s) {
  struct addrinfo hints, *p;
  struct sockaddr_in ipv4;
  struct sockaddr_in6 ipv6;
  
  int rc;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  if ((rc = getaddrinfo(s->host, NULL, &hints, &p)) != 0) {
    printf("%s: getaddrinfo: error: %s\n", progname, gai_strerror(rc));
    return 1;
  }

  /* pick the first address */
  if (p->ai_family == AF_INET) { /* IPv4 */
    memcpy(&ipv4, p->ai_addr, sizeof(struct sockaddr_in));
    ipv4.sin_port = htons(s->port);
    memcpy(&(s->host_addr), &ipv4, sizeof(struct sockaddr_in));
    s->host_addrlen = sizeof(struct sockaddr_in);
  }
  else { /* IPv6 */
    memcpy(&ipv6, p->ai_addr, sizeof(struct sockaddr_in6));
    ipv6.sin6_port = htons(s->port);
    memcpy(&(s->host_addr), &ipv6, sizeof(struct sockaddr_in6));
    s->host_addrlen = sizeof(struct sockaddr_in6);
  }
    
  freeaddrinfo(p);
  return 0;
}
