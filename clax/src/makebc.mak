#
# clax version 0.10 makefile (Borland C - DOS)
#
# i tested clax with Borland C 3.1 only, but it should
# compile without a problem on any higher version.
#

CC      = bcc
CCOPT   = -c -ml -I\programs\tc\include
DEBUG   =
DEFINES =
INCLUDE = 
LIB     = tlib
LIBOPT  =
LIBNAME = clax.lib
OBJS    = camera.obj clax.obj file_3ds.obj matrix.obj quatern.obj \
	  spline.obj vector.obj

all: $(OBJS) 
	if exist $(LIBNAME) del $(LIBNAME)
	for %i in ($(OBJS)) do $(LIB) $(LIBOPT) $(LIBNAME) +%i
	if exist clax.bak del clax.bak

clean: 
	for %i in ($(OBJS)) do if exist %i del %i

cleanall:
	for %i in ($(OBJS)) do if exist %i del %i
	if exist $(LIBNAME) del $(LIBNAME)

.c.obj:
	$(CC) $(CCOPT) $(DEBUG) $(DEFINES) $(INCLUDE) $<
