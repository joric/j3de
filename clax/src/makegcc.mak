#
# clax version 0.10 makefile (GNU C - UNIX/DOS)
#
# should compile on any unix system with GNU C.
# tested on linux 2.0.28 with GCC 2.7.2 ELF, and DJGPP 2b under DOS.
#

CC      = gcc
CCOPT   = -O2 -Wall
#DEBUG  =
DEBUG   = -g
DEFINES =
INCLUDE =
LIB     = ar
LIBOPT  = crv
LIBNAME = clax.a
OBJS    = camera.o clax.o file_3ds.o matrix.o quatern.o spline.o vector.o

all: $(OBJS)
	$(LIB) $(LIBOPT) $(LIBNAME) $(OBJS)

clean:
	rm -rf $(OBJS)

cleanall:
	rm -rf $(OBJS)
	rm -rf $(LIBNAME)

.c.o:
	$(CC) $(CCOPT) $(DEBUG) $(DEFINES) $(INCLUDE) -o $@ -c $<
