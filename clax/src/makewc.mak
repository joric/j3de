#
# clax version 0.10 makefile (WATCOM C - DOS)
#
# clax compiles under both watcom 16 and 32 bit versions.
# if you want to compile for 16 bit, just change the CC
# variable to wcc.
#

CC      = wcc386
#CC     = wcc
CCOPT   = -otexan -s -fp5 -fpi87 -5
#DEBUG   = 
DEBUG  = /d2
DEFINES =
INCLUDE =
LIB     = wlib
LIBOPT  = /b
LIBNAME = clax.lib
OBJS    = camera.obj clax.obj file_3ds.obj matrix.obj quatern.obj &
	  spline.obj vector.obj file_clx.obj

all: $(OBJS) .symbolic
	if exist $(LIBNAME) del $(LIBNAME)
	for %i in ($(OBJS)) do $(LIB) $(LIBOPT) $(LIBNAME) +%i

clean: .symbolic
	for %i in ($(OBJS)) do if exist %i del %i

cleanall: .symbolic
	for %i in ($(OBJS)) do if exist %i del %i
	if exist $(LIBNAME) del $(LIBNAME)

.c.obj:
	$(CC) $(CCOPT) $(DEBUG) $(DEFINES) $(INCLUDE) $<
