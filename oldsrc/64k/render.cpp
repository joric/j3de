#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "clax.h"
#include "render.h"
#include "fatmap.h"

int*   outBuffer;
char*   nzBuffer;
char*   trBuffer;

int     bufferWidth, bufferHeight;

c_DSORT*  sortList;
c_DSORT*  tmpList;
c_DFACE*  faceList;
int       nFaces;

#define ByteOf(x) (((x) >> bitsOffset) & 0xff)

static
void
radix( short bitsOffset, ulong N, c_DSORT *source, c_DSORT *dest )
{
  // suppressed the need for index as it is reported in count
  ulong    count[256];

  // added temp variables to simplify writing, understanding and compiler optimization job
  // most of them will be allocated as registers
  ulong    *cp;
  c_DSORT  *sp;
  ulong    s, c, i;

  // faster than MemSet
  cp = count;
  for (i = 256; i > 0; --i, ++cp)
    *cp = 0;

  // count occurences of every byte value
  sp = source;
  for (i = N; i > 0; --i, ++sp) {
    cp = count + ByteOf( sp->z );
    (*cp)++;
  }

  // transform count into index by summing elements and storing into same array
  s = 0;
  cp = count;
  for (i = 256; i > 0; --i, ++cp) {
    c = *cp;
    *cp = s;
    s += c;
  }

  // fill dest with the right values in the right place
  sp = source;
  for (i = N; i > 0; --i, ++sp) {
    cp = count + ByteOf( sp->z );
    dest[*cp].z    = sp->z;
    dest[*cp].face = sp->face;
    (*cp)++;
  }
}

void
sortPolys()
{
  radix( 0, nFaces, sortList, tmpList );
  radix( 8, nFaces, tmpList, sortList );
  radix( 16, nFaces, sortList, tmpList );
  radix( 24, nFaces, tmpList, sortList );
}


float sqr(float x)
{
	return x*x;
}

void
drawTriangle( c_VERTEX* v0, c_VERTEX* v1, c_VERTEX* v2, int id)
{

    long didx, dudx, dvdx;
	float a_u, b_u, c_u;
	float a_v, b_v, c_v;

	float zk=64.0;

    double denom = (v0->sx - v2->sx) * (v1->sy - v2->sy) - 
				   (v1->sx - v2->sx) * (v0->sy - v2->sy);

    if( denom == 0.0 ) return;

	denom= (float)ZCOR/denom;	

    didx = (long)(((v0->sz*zk - v2->sz*zk) * (v1->sy - v2->sy) -  
				(v1->sz*zk - v2->sz*zk) * (v0->sy - v2->sy)) * denom);

	float a=(sqr(v1->sx - v0->sx) + sqr(v1->sy - v0->sy));
	float b=(sqr(v2->sx - v1->sx) + sqr(v2->sy - v1->sy));
	float c=(sqr(v0->sx - v2->sx) + sqr(v0->sy - v2->sy));	
	int lim=sqr(32);

	USE_AFFINE_MAPPING=(a<lim && b<lim && c<lim);
				
if (!USE_AFFINE_MAPPING)
{
	a_u = v0->u * v0->sz * zk;
	a_v = v0->v * v0->sz * zk;
	b_u = v1->u * v1->sz * zk;
	b_v = v1->v * v1->sz * zk;
	c_u = v2->u * v2->sz * zk;
	c_v = v2->v * v2->sz * zk;
}
else
{
	a_u = v0->u;
	a_v = v0->v;
	b_u = v1->u;
	b_v = v1->v;
	c_u = v2->u;
	c_v = v2->v;
}

    dudx = (long)(((a_u - c_u) * (v1->sy - v2->sy) -  
				   (b_u - c_u) * (v0->sy - v2->sy)) * denom);
	
    dvdx = (long)(((a_v - c_v) * (v1->sy - v2->sy) -  
				   (b_v - c_v) * (v0->sy - v2->sy)) * denom);

    c_DVERTEX*  v;

//----vertex1----
    v = &faceList[nFaces].v[0];
	
	v->x = (long)(v0->sx * 65536.0f);
	v->y = (long)(v0->sy * 65536.0f);

	v->i = (long)(v0->sz * ZCOR * zk);
	v->u = (long)(a_u * ZCOR);
	v->v = (long)(a_v * ZCOR);
//----vertex2----
    v = &faceList[nFaces].v[1];
	
	v->x = (long)(v1->sx * 65536.0f);
	v->y = (long)(v1->sy * 65536.0f);

	v->i = (long)(v1->sz * ZCOR * zk);

	v->u = (long)(b_u * ZCOR);
	v->v = (long)(b_v * ZCOR);
//----vertex3----
    v = &faceList[nFaces].v[2];

	v->x = (long)(v2->sx * 65536.0f);
	v->y = (long)(v2->sy * 65536.0f);

	v->i = (long)(v2->sz * ZCOR * zk);
	v->u = (long)(c_u * ZCOR);
	v->v = (long)(c_v * ZCOR);
//---------------

    faceList[nFaces].didx = didx;
	faceList[nFaces].dudx = dudx;
	faceList[nFaces].dvdx = dvdx;
	faceList[nFaces].id = id;
	faceList[nFaces].affine = USE_AFFINE_MAPPING;
	faceList[nFaces].intensity = m_intensity;

		
    // farthest point
    long  z = (*((long *)&v0->sz));
    if (*((long *)&v1->sz) < z )
      z = *((long *)&v1->sz);
    if (*((long *)&v2->sz) < z )
      z = *((long *)&v2->sz);

    sortList[nFaces].z = z; //sort backwards :-?
    sortList[nFaces].face = &faceList[nFaces];

    // add new poly
    if (nFaces<MAXFACES) nFaces++;
	
}

void
flushPolys()
{
  int        i;
  c_DFACE*   face;

  sortPolys(); //GPF!!!???
  Vertex v[3];

  for ( i = 0; i < nFaces; i++ ) {
    face = sortList[i].face;

	vfaces++; //global rendered faces counter
	if (face->affine) afaces++;

		textureoffset=0;
		int mat=face->id;

		USE_AFFINE_MAPPING=face->affine;

		textureoffset=0;
		//if (mat>=0 && mat<=9) textureoffset=256*256*mat;
		switch(mat)
		{
		case 2:textureoffset=256*128+128; break;//walls
		case 7:textureoffset=256*128;break; //column
		case 1:textureoffset=256*128+128;break; //lab
		case 5:textureoffset=128;break; //windows
		case 8:textureoffset=256*256;break; //meta
		}

		m_intensity=face->intensity;

		DrawPkTexturedPoly( face->v, 3, face->didx, face->dudx, face->dvdx );
  }  
	nFaces = 0;
}


//
//   VIEWPORT
//

int
viewportOpen( int width, int height, int maxFaces, c_VIEWPORT* vp )
{

  vp->iWidth = width;
  vp->iHeight = height;

  if ( maxFaces == -1 )
    maxFaces = MAXFACES;

  vp->outBuffer = new int [vp->iWidth * vp->iHeight];
  if ( !vp->outBuffer ) {
    return clax_err_nomem;
  }

  vp->nzBuffer = new char [vp->iWidth * vp->iHeight * 2];
  if ( !vp->nzBuffer ) {
    delete [] vp->outBuffer;
    return clax_err_nomem;
  }

  vp->trBuffer = new char [vp->iWidth * vp->iHeight];
  if ( !vp->trBuffer ) {
    delete [] vp->outBuffer;
    delete [] vp->nzBuffer;
    return clax_err_nomem;
  }

  vp->sortList = new c_DSORT [maxFaces];
  if ( !vp->sortList ) {
    delete [] vp->outBuffer;
    delete [] vp->nzBuffer;
    delete [] vp->trBuffer;
    return clax_err_nomem;
  }

  vp->tmpList = new c_DSORT [maxFaces];
  if ( !vp->tmpList ) {
    delete [] vp->outBuffer;
    delete [] vp->nzBuffer;
    delete [] vp->trBuffer;
    delete [] vp->sortList;
    return clax_err_nomem;
  }

  vp->faceList = new c_DFACE [maxFaces];
  if ( !vp->tmpList ) {
    delete [] vp->outBuffer;
    delete [] vp->nzBuffer;
    delete [] vp->trBuffer;
    delete [] vp->sortList;
    delete [] vp->tmpList;
    return clax_err_nomem;
  }

  return clax_err_ok;
}


void
viewportClose( c_VIEWPORT* vp )
{
  if ( vp->outBuffer )
    delete [] vp->outBuffer;
  if ( vp->nzBuffer )
    delete [] vp->nzBuffer;
  if ( vp->trBuffer )
    delete [] vp->trBuffer;
  if ( vp->sortList )
    delete [] vp->sortList;
  if ( vp->tmpList )
    delete [] vp->tmpList;
  if ( vp->faceList )
    delete [] vp->faceList;
}

void
viewportSetactive( c_VIEWPORT* vp )
{  
  outBuffer = vp->outBuffer;
  nzBuffer = vp->nzBuffer;
  trBuffer = vp->trBuffer;

  bufferWidth = vp->iWidth;
  bufferHeight = vp->iHeight;

  sortList = vp->sortList;
  tmpList = vp->tmpList;
  faceList = vp->faceList;
  
  nFaces = 0;
}

void
viewportGetactive( c_VIEWPORT* vp )
{  
  vp->outBuffer = outBuffer;
  vp->nzBuffer = nzBuffer;
  vp->trBuffer = trBuffer;

  vp->iWidth = bufferWidth;
  vp->iHeight = bufferHeight;

  vp->sortList = sortList;
  vp->tmpList = tmpList;
  vp->faceList = faceList;
}
