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
    {15.0,  115.0}, {20.0, 94.28571}, {24.0, 84.0}, {28.0,  76.36364},
    {35.0,  63.0},  {50.0, 46.0},     {85.0, 28.0}, {135.0, 18.0},
    {200.0, 12.0}
  };

  for (i = 0; i < 9; i++) if (lens == lens_table[i].lens) {
    *fov = lens_table[i].fov;
    return;
  }
  *fov = 15.0 / lens * 160; /* hello adept :) */
}

void cam_update (c_CAMERA *cam)
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

  mat_identity (mat);
  vec_negate (&cam->pos, &pivot);
  vec_sub (&cam->target, &cam->pos, &c);
  focus = vec_length (&c);
  tanfov = tan ((cam->fov / 2) / 180 * M_PI);
  cam->perspX = cam->sizeX / 2 / tanfov;
  cam->perspY = cam->sizeY / 2 / cam->aspectratio / tanfov;

  ax = -atan2 (c.x, c.z);
  ay = asin (c.y / focus);
  az = -cam->roll * M_PI / 180;

  sinx = sin (ax); cosx = cos (ax); siny = sin (ay);
  cosy = cos (ay); sinz = sin (az); cosz = cos (az);

  mat[X][X] =  sinx * siny * sinz + cosx * cosz;
  mat[X][Y] =  cosy * sinz;
  mat[X][Z] =  sinx * cosz - cosx * siny * sinz;
  mat[Y][X] =  sinx * siny * cosz - cosx * sinz;
  mat[Y][Y] =  cosy * cosz;
  mat[Y][Z] = -cosx * siny * cosz - sinx * sinz;
  mat[Z][X] = -sinx * cosy;
  mat[Z][Y] =  siny;
  mat[Z][Z] =  cosx * cosy;

  mat_pretrans (&pivot, mat, cam->matrix);
}
