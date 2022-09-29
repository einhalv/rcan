#ifndef _RCAN_H_
#define _RCAN_H_ 1
#include <netinet/in.h>

/* parameters */
#define NUM_CANS 3
#define DEFAULT_REMOTE_PORT 23
#define PROTOCOL_NAME_LENGTH 32
#define IF_NAME_LENGTH 64
#define MAXHOSTNAMELENGTH 128
#define MAXFRAMELENGTH 128


/* data types */
typedef struct {
  char can_format[PROTOCOL_NAME_LENGTH];
  int bus_active[NUM_CANS];
  char bus_if_name[NUM_CANS][IF_NAME_LENGTH];
  int bus_socket_desc[NUM_CANS];
  char *host; 
  int port;
  int host_socket_desc;
  struct sockaddr_storage host_addr;
  size_t host_addrlen;
} setup_t;

/* prototypes */
void init_setup(setup_t *s);
int remote_address(setup_t *s);
void *downstream(void *p);
void *upstream(void *p);
int alldigits(char *p);
int splitaddrport(char *dest, const char *src);

#endif
