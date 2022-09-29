####################################################################
#	
# Makefile for rcan 
# (C) 2022 Einar Halvorsen
# 
#####################################################################
# location to install binary
INSBIN=${HOME}/bin 
####################################################################
## Profiling 
#PROF= -g -pg 
PROF= 
## C-compiler
CC=gcc  ${PROF}
CINC= ./
CFLAGS = -ffloat-store -Wall -Wstrict-prototypes -Wmissing-prototypes -Wformat -O2 -funroll-loops -fexpensive-optimizations -ansi -pedantic -D_GNU_SOURCE -I${CINC}
## Linker
LD=gcc
LDDIR = 
LIBS = -lc -lpthread

OBJS=rcan.o init_setup.o splitaddrport.o remote_address.o downstream.o upstream.o downstream_gvret.o gvret_frame.o downstream_crtd.o crtd_frame.o alldigits.o

.SUFFIXES:
.SUFFIXES: .c .o 

.c.o:; 	$(CC) $(CFLAGS) -c -o $@ $*.c 

rcan:  $(OBJS)
	$(LD) -o rcan $(OBJS) $(LIBS)

clean: 
	rm -f  $(OBJS)

veryclean: clean 
	rm -f rcan *~ 
