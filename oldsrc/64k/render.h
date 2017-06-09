
#ifndef __MOE_RENDER_HH__
#define __MOE_RENDER_HH__

#include "clax.h"

#define  MAXFACES   10000

typedef unsigned long  ulong;


struct c_DVERTEX {
    long  x,y,z;        // Screen position in 16:16 bit fixed point
    long  i;			// Color intensity in 16:16 bit fixed point
	long  u,v;		    // texture coordinates in 16:16
};

struct c_DFACE {
  c_DVERTEX  v[3];
  long       didx, dudx, dvdx;	//intensity gradient 
  int		 intensity; // lambert intesity
  int		 id;  //material id
  bool		 affine; // use affine mapping flag
};

struct c_DSORT {
  unsigned long     z;
  c_DFACE*  face;
};
 

struct c_VIEWPORT {
  int    iWidth;
  int    iHeight;
  c_DSORT*  sortList;
  c_DSORT*  tmpList;
  c_DFACE*  faceList;
  int*  outBuffer;     // color
  char*  nzBuffer;      // combined normal-z-buffer
  char*  trBuffer;      // trace buffer
};


void  drawTriangle( c_VERTEX* v0, c_VERTEX* v1, c_VERTEX* v2, int id);
void  flushPolys();
int   viewportOpen( int width, int height, int maxFaces, c_VIEWPORT* vp );
void  viewportClose( c_VIEWPORT* vp );
void  viewportSetactive( c_VIEWPORT* vp );
void  viewportGetactive( c_VIEWPORT* vp );


#endif