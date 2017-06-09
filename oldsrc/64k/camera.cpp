/*CAMERA.C********************************************************************

   Clax: Portable keyframing library
         see inclosed LICENSE.TXT for licensing terms.

         for documentation, refer to CLAX.TXT

   author           : Adept
   file created     : 16/04/97
   file description : camera math

   revision history :
     v0.10 (16/04/97) Adept: Initial version.
           (17/06/97) Borzom: Added lens to fov conversion.
           (20/06/97: Borzom: Camera pivot inside matrix.

   notes            :

*****************************************************************************/

#include <math.h>
#include <stdio.h>
#include "clax.h"
#include "claxi.h"

void cam_lens_fov (float lens, float *fov)
{
/*
  cam_lens_fov: converts 3dstudio lens to fov.
*/
  int16  i;
  struct {
    float lens, fov;
  } lens_table[] = {
    {15.0f,  115.0f}, {20.0f, 94.28571f}, {24.0f, 84.0f}, {28.0f,  76.36364f},
    {35.0f,  63.0f},  {50.0f, 46.0f},     {85.0f, 28.0f}, {135.0f, 18.0f},
    {200.0f, 12.0f}
  };

  for (i = 0; i < 9; i++) if (lens == lens_table[i].lens) {
    *fov = lens_table[i].fov;
    return;
  }
  *fov = 15.0f / lens * 160.0f; /* hello adept :) */
}


void
cam_update( c_CAMERA *cam )
{
/*
  cam_update: updates the camera matrix.
*/
  c_VECTOR c, pivot;
  c_MATRIX mat;
  float    tanfov, focus;
  float    ax, ay, az;
  float    sinx, siny, sinz,
           cosx, cosy, cosz;

  mat_identity( mat );
  vec_negate( &cam->pos, &pivot );
  vec_sub( &cam->target, &cam->pos, &c );
  focus = vec_length( &c );

  tanfov = 1.0f / (float)tan( (cam->fov / 2.0f) / 180.0f * (float)M_PI );

  cam->viewAspectratio = cam->xSize / (cam->ySize / cam->pixelHeight);
  cam->xOffset = cam->xSize / 2.0f;
  cam->yOffset = cam->ySize / 2.0f;
  cam->xScale = cam->xOffset * tanfov;
  cam->yScale = cam->yOffset * tanfov * cam->viewAspectratio;
  cam->xClip = tanfov;
  cam->yClip = tanfov * cam->viewAspectratio; 

/*  printf( "\n[Camera]\n" );
  printf( " FOV:     %f\n", cam->fov );
  printf( " tanfov:  %f\n", tanfov );
  printf( " xClip:   %f\n", cam->xClip );
  printf( " yClip:   %f\n", cam->yClip );
  printf( " xSize:   %f\n", cam->xSize );
  printf( " ySize:   %f\n", cam->ySize );
  printf( " viewAsp: %f\n", cam->viewAspectratio );
  fflush( stdout );*/

  ax = -(float)atan2( c.x, c.z );
  ay = (float)asin( c.y / focus );
  az = -cam->roll * (float)M_PI / 180.0f;

  sinx = (float)sin( ax ); cosx = (float)cos( ax ); siny = (float)sin( ay );
  cosy = (float)cos( ay ); sinz = (float)sin( az ); cosz = (float)cos( az );

  mat[X][X] =  sinx * siny * sinz + cosx * cosz;
  mat[X][Y] =  cosy * sinz;
  mat[X][Z] =  sinx * cosz - cosx * siny * sinz;
  mat[Y][X] =  sinx * siny * cosz - cosx * sinz;
  mat[Y][Y] =  cosy * cosz;
  mat[Y][Z] = -cosx * siny * cosz - sinx * sinz;
  mat[Z][X] = -sinx * cosy;
  mat[Z][Y] =  siny;
  mat[Z][Z] =  cosx * cosy;

  mat_pretrans( &pivot, mat, cam->matrix );
}
