#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <pthread.h>
#include <unistd.h>
#include "rcan.h"

int alldigits(char *p);

/* global data */
char progname[] = "rcan";
setup_t setup;

int main(int argc, char *argv[]) {
  static char host[MAXHOSTNAMELENGTH]="";
  int port;
  char *usage = "host protocol bus0 if_name0 [bus1 if_name1 [...]]";
  int n, bus, s;
  struct sockaddr_can canaddr;
  pthread_t thread_upstream, thread_downstream;
  int rc, rc_up, rc_down, cerr_up, cerr_down;
  
  init_setup(&setup);
  
  /*********************************************************************/
  /*                 parse command line arguments                      */
  if (argc < 4 || !(argc % 2)) {
    printf("%s: error: incorrect number of arguments\n", progname);
    printf("Usage: %s %s\n", progname, usage);
    return 1;
  }
  port = splitaddrport(host, argv[1]);
  if (port >= 0) {
    setup.port = port;
  }
  if (host[0]) {
    setup.host = host;
  }
  else {
    printf("%s: error: incomprehensible host specification: %s\n",
           progname, argv[1]);
    return 1;
  }
  
  if (snprintf(setup.can_format, PROTOCOL_NAME_LENGTH, "%s", argv[2])
      < strlen(argv[2]) ) {
    printf("%s: error: invalid protocol name\n", progname);
    printf("Usage: %s %s\n", progname, usage);
    return 1;    
  }
  for (n=3; n < argc-1; n += 2) {
    if (alldigits(argv[n])) {
      bus = atoi(argv[n]);
    }
    else {
      printf("%s: error: argument \"%s\" is not a nonnegative integer\n",
             progname, argv[n]);
      printf("Usage: %s %s\n", progname, usage);
      return 1;
    }
    if (bus >=0 && bus <NUM_CANS) {
      setup.bus_active[bus] = 1;
    }
    else {
      printf("%s: error: bus number %d is outside range [0,...,%d]\n",
             progname, bus, NUM_CANS-1);
      printf("Usage: %s %s\n", progname, usage);
      return 1;      
    }
    if (strlen(argv[n+1]) < IF_NAME_LENGTH) { 
      strcpy(setup.bus_if_name[bus], argv[n+1]);
    }
    else {
      printf("%s: error: interface name for bus number %d is too long:"
             " \"%s\"\n", progname, bus, argv[n+1]);
    }
  }

  /*********************************************************************/
  /*                      create socket to host                        */
  printf("%s: creating sockets\n", progname);
  setup.host_socket_desc = socket(AF_INET, SOCK_STREAM, 0);  
  if(setup.host_socket_desc < 0){
    printf("%s: error: Unable to create host socket\n", progname);
    return 1;
  }
  if (remote_address(&setup)) { /* get address info */
    printf("%s: error: failed obtain host address\n", progname);
    return 1;    
  }  
  if (connect(setup.host_socket_desc,
              (struct sockaddr*)&setup.host_addr, setup.host_addrlen) < 0)
    {
      printf("%s: error: unable to connect to host\n", progname);
      return 1;
    }

  /*********************************************************************/
  /*                           create CAN sockets                      */
  for (n=0; n<NUM_CANS; n++) {
    if (!setup.bus_active[n])
      continue;
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
      printf("%s: error: Failed to create %s socket\n",
             progname, setup.bus_if_name[n]);
      return 1;
    }
    setup.bus_socket_desc[n] = s;

    printf("%s: binding socket to %s\n", progname, setup.bus_if_name[n]); 
    memset(&canaddr, 0, sizeof(canaddr));
    canaddr.can_family = AF_CAN;
    canaddr.can_ifindex = if_nametoindex(setup.bus_if_name[n]);
    if ( (rc=bind(s, (struct sockaddr *)&canaddr, sizeof(canaddr))) < 0) {
      printf("%s: error: unable to bind to %s: exit code %d\n",
             progname, setup.bus_if_name[n], rc);
      return 1;
    }
  }

  /*********************************************************************/
  /*                  spawn threads/subprocesses                       */ 
  if (pthread_create(&thread_downstream, NULL, &downstream, &setup)) {
    printf("%s: error: failed to start downstream thread\n", progname);
    return 1;
  }
  if (pthread_create(&thread_upstream, NULL, &upstream, &setup)) {
    printf("%s: error: failed to start upstream thread\n", progname);
    return 1;
  }

  while (1) { /* wait until one thread exits */
    rc_up = pthread_tryjoin_np(thread_upstream, (void *) &cerr_up);
    rc_down = pthread_tryjoin_np(thread_downstream, (void *) &cerr_down);
    /* printf("%s: upstream thread exited with code %d\n", progname, cerr); */
    if (rc_up && rc_down) { /* both threads run */
      sleep(15); /* could use sigwait ? */
    }
    else { /* at least one thread exited */ 
      break;
    }
  }
  /* cancel remaining thread if any */
  if (rc_up && !pthread_cancel(thread_upstream)) {
    rc_up = pthread_join(thread_upstream, (void *) &cerr_up);
  }
  if (rc_down && !pthread_cancel(thread_downstream)) {
    rc_down = pthread_join(thread_downstream, (void *) &cerr_down);    
  } /* bad to overwrite ? */
 

  /* exit program */
  pthread_exit(NULL);
  return 0;
}
