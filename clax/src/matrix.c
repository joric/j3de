/*MATRIX.C********************************************************************

   Clax: Portable keyframing library
         see inclosed LICENSE.TXT for licensing terms.

         for documentation, refer to CLAX.TXT

   author           : Borzom
   file created     : 17/04/97
   file description : matrix math

   revision history :
     v0.10 (17/04/97) Borzom: Initial version.

   notes            :
     ( xx , xy , xz , xw ) clax 3x4 matrix structure.
     ( yx , yy , yz , yw )
     ( zx , zy , zz , zw )

*****************************************************************************/

#include <math.h>
#include <stdio.h>
#include "clax.h"
#include "claxi.h"

static c_MATRIX Tidentity = {
  {1, 0, 0, 0},
  {0, 1, 0, 0},
  {0, 0, 1, 0},
};

void mat_zero (c_MATRIX out)
{
/*
  mat_zero: clear matrix.
*/
  int16 i, j;

  for (i = 0; i < 3; i++)
    for (j = 0; j < 4; j++)
      out[i][j] = 0.0;
}

void mat_identity (c_MATRIX out)
{
/*
  mat_idenity: make identity matrix.
*/
  int16 i, j;

  for (i = 0; i < 3; i++)
    for (j = 0; j < 4; j++)
      out[i][j] = Tidentity[i][j];
}

void mat_copy (c_MATRIX a, c_MATRIX out)
{
/*
  mat_copy: matrix copy.
*/
  int16 i, j;

  for (i = 0; i < 3; i++)
    for (j = 0; j < 4; j++)
      out[i][j] = a[i][j];
}

void mat_print (c_MATRIX a)
{
/*
  mat_print: print matrix on stdout.
*/
  printf ("xx: %9.3f xy: %9.3f xz: %9.3f xw: %9.3f\n",
          a[X][X], a[X][Y], a[X][Z], a[X][W]);
  printf ("yx: %9.3f yy: %9.3f yz: %9.3f yw: %9.3f\n",
          a[Y][X], a[Y][Y], a[Y][Z], a[Y][W]);
  printf ("zx: %9.3f zy: %9.3f zz: %9.3f zw: %9.3f\n",
          a[Z][X], a[Z][Y], a[Z][Z], a[Z][W]);
}

void mat_add (c_MATRIX a, c_MATRIX b, c_MATRIX out)
{
/*
  mat_add: matrix addition.
*/
  int16    i, j;
  c_MATRIX temp;
  for (i = 0; i < 3; i++)
    for (j = 0; j < 4; j++)
      temp[i][j] = a[i][j]+b[i][j];
  mat_copy (temp, out);
}

void mat_sub (c_MATRIX a, c_MATRIX b, c_MATRIX out)
{
/*
  mat_sub: matrix substraction.
*/
  int16    i, j;
  c_MATRIX temp;

  for (i = 0; i < 3; i++)
    for (j = 0; j < 4; j++)
      temp[i][j] = a[i][j] - b[i][j];
  mat_copy (temp, out);
}

void mat_mul (c_MATRIX a, c_MATRIX b, c_MATRIX out)
{
/*
  mat_mul: matrix multiplication.
*/
  c_MATRIX temp;

  temp[X][X]=a[X][X]*b[X][X]+a[X][Y]*b[Y][X]+a[X][Z]*b[Z][X];
  temp[X][Y]=a[X][X]*b[X][Y]+a[X][Y]*b[Y][Y]+a[X][Z]*b[Z][Y];
  temp[X][Z]=a[X][X]*b[X][Z]+a[X][Y]*b[Y][Z]+a[X][Z]*b[Z][Z];
  temp[X][W]=a[X][X]*b[X][W]+a[X][Y]*b[Y][W]+a[X][Z]*b[Z][W]+a[X][W];
  temp[Y][X]=a[Y][X]*b[X][X]+a[Y][Y]*b[Y][X]+a[Y][Z]*b[Z][X];
  temp[Y][Y]=a[Y][X]*b[X][Y]+a[Y][Y]*b[Y][Y]+a[Y][Z]*b[Z][Y];
  temp[Y][Z]=a[Y][X]*b[X][Z]+a[Y][Y]*b[Y][Z]+a[Y][Z]*b[Z][Z];
  temp[Y][W]=a[Y][X]*b[X][W]+a[Y][Y]*b[Y][W]+a[Y][Z]*b[Z][W]+a[Y][W];
  temp[Z][X]=a[Z][X]*b[X][X]+a[Z][Y]*b[Y][X]+a[Z][Z]*b[Z][X];
  temp[Z][Y]=a[Z][X]*b[X][Y]+a[Z][Y]*b[Y][Y]+a[Z][Z]*b[Z][Y];
  temp[Z][Z]=a[Z][X]*b[X][Z]+a[Z][Y]*b[Y][Z]+a[Z][Z]*b[Z][Z];
  temp[Z][W]=a[Z][X]*b[X][W]+a[Z][Y]*b[Y][W]+a[Z][Z]*b[Z][W]+a[Z][W];
  mat_copy (temp, out);
}

void mat_transpose (c_MATRIX a, c_MATRIX out)
{
/*
  mat_transpose: transpose matrix.
*/
  int      i, j;
  c_MATRIX temp;

  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      temp[i][j] = a[j][i];
  for (i = 0; i < 3; i++)
    temp[i][W] = a[i][W];
  mat_copy (temp, out);
}

int32 mat_inverse (c_MATRIX a, c_MATRIX out)
{
/*
  mat_inverse: inverse matrix calculation (non-singular).
*/
  int16    i, j;
  float    scale;
  c_MATRIX temp;

  for (i = 0; i < 3; i++) {
    scale = a[i][X]*a[i][X] + a[i][Y]*a[i][Y] + a[i][Z]*a[i][Z];
    if (scale == 0.0) {
      mat_identity (out);
      return clax_err_singular;
    }
    scale = 1.0 / scale;
    for (j = 0; j < 3; j++) temp[i][j] = a[j][i] * scale;
    temp[i][W] = -(temp[i][X]*a[i][W] + temp[i][Y]*a[Y][W] +
                   temp[i][Z]*a[Z][W]);
  }
  mat_copy (temp, out);
  return clax_err_ok;
}

int32 mat_invscale (c_MATRIX a, c_MATRIX out)
{
/*
  mat_invscale: inverse matrix scale.
*/
  int16    i, j;
  float    scale;
  c_MATRIX temp;

  for (i = 0; i < 3; i++) {
    scale = a[i][X]*a[i][X] + a[i][Y]*a[i][Y] + a[i][Z]*a[i][Z];
    if (scale == 0.0) {
      mat_identity (out);
      return clax_err_singular;
    }
    scale = 1.0 / scale;
    for (j = 0; j < 3; j++) temp[i][j] = a[i][j] * scale;
    temp[i][W] = a[i][W];
  }
  mat_copy (temp, out);
  return clax_err_ok;
}

int32 mat_normalize (c_MATRIX a, c_MATRIX out)
{
/*
  mat_normalize: normalize matrix.
*/
  int16    i, j;
  float    len;
  c_MATRIX temp;

  for (i = 0; i < 3; i++) {
    len = sqrt (a[i][X]*a[i][X] + a[i][Y]*a[i][Y] + a[i][Z]*a[i][Z]);
    if (len != 0.0) len = 1.0 / len; else len = 1.0;
    for (j = 0; j < 3; j++) temp[i][j] = a[i][j] * len;
    temp[i][W] = a[i][W];
  }
  mat_copy (temp, out);
  return clax_err_ok;
}

void mat_toeuler (c_MATRIX mat, c_VECTOR *out)
{
/*
  mat_toeuler: convert rotation matrix to euler angles.
*/
   float siny, cosy;

   siny = mat[Z][X];
   cosy = sqrt (1.0 - siny*siny);
   if (mat[X][X] < 0.0 && mat[Z][Z] < 0.0) cosy = -cosy;
   if (cosy != 0.0) {
     out->x = atan2 (mat[Z][Y] / cosy, mat[Z][Z] / cosy);
     out->y = atan2 (siny, cosy);
     out->z = atan2 (mat[Y][X] / cosy, mat[X][X] / cosy);
   } else {
     out->x = atan2 (-mat[Y][Z], mat[Y][Y]);
     out->y = asin (siny);
     out->z = 0.0;
   }
}

void mat_pretrans (c_VECTOR *v, c_MATRIX mat, c_MATRIX out)
{
/*
  mat_pretrans: create a pre-translation matrix.
*/
  c_MATRIX temp;
  int      i, j;

  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      temp[i][j] = mat[i][j];
  temp[X][W] = mat[X][X]*v->x + mat[X][Y]*v->y + mat[X][Z]*v->z + mat[X][W];
  temp[Y][W] = mat[Y][X]*v->x + mat[Y][Y]*v->y + mat[Y][Z]*v->z + mat[Y][W];
  temp[Z][W] = mat[Z][X]*v->x + mat[Z][Y]*v->y + mat[Z][Z]*v->z + mat[Z][W];
  mat_copy (temp, out);
}

void mat_settrans (c_VECTOR *v, c_MATRIX out)
{
/*
  mat_settrans: create a translation matrix.
*/
  mat_identity (out);
  out[X][W] = v->x;
  out[Y][W] = v->y;
  out[Z][W] = v->z;
}

void mat_setscale (c_VECTOR *v, c_MATRIX out)
{
/*
  mat_setscale: create a scale matrix.
*/
  mat_identity (out);
  out[X][X] = v->x;
  out[Y][Y] = v->y;
  out[Z][Z] = v->z;
}

void mat_rotateX (float ang, c_MATRIX out)
{
/*
  mat_rotateX: create rotation matrix around X axis.
*/
  float sinx, cosx;

  mat_identity (out);
  sinx = sin (ang);
  cosx = cos (ang);
  out[Y][Y] =  cosx;
  out[Y][Z] =  sinx;
  out[Z][Y] = -sinx;
  out[Z][Z] =  cosx;
}

void mat_rotateY (float ang, c_MATRIX out)
{
/*
  mat_rotateY: create rotation matrix around Y axis.
*/
  float siny, cosy;

  mat_identity (out);
  siny = sin (ang);
  cosy = cos (ang);
  out[X][X] =  cosy;
  out[X][Z] = -siny;
  out[Z][X] =  siny;
  out[Z][Z] =  cosy;
}

void mat_rotateZ (float ang, c_MATRIX out)
{
/*
  mat_rotateZ: create rotation matrix around Z axis.
*/
  float sinz, cosz;

  mat_identity (out);
  sinz = sin (ang);
  cosz = cos (ang);
  out[X][X] =  cosz;
  out[X][Y] =  sinz;
  out[Y][X] = -sinz;
  out[Y][Y] =  cosz;
}

void mat_mulvec (c_MATRIX a, c_VECTOR *b, c_VECTOR *out)
{
/*
  mat_mulvec: multiply a vector by matrix (out = [a]*b)
*/
  c_VECTOR temp;

  temp.x = b->x*a[X][X] + b->y*a[X][Y] + b->z*a[X][Z] + a[X][W];
  temp.y = b->x*a[Y][X] + b->y*a[Y][Y] + b->z*a[Y][Z] + a[Y][W];
  temp.z = b->x*a[Z][X] + b->y*a[Z][Y] + b->z*a[Z][Z] + a[Z][W];
  vec_copy (&temp, out);
}

void mat_mulnorm (c_MATRIX a, c_VECTOR *b, c_VECTOR *out)
{
/*
  mat_mulnorm: multiply a normal by matrix (out = [a]*b)
*/
  c_VECTOR temp;

  temp.x = b->x*a[X][X] + b->y*a[X][Y] + b->z*a[X][Z];
  temp.y = b->x*a[Y][X] + b->y*a[Y][Y] + b->z*a[Y][Z];
  temp.z = b->x*a[Z][X] + b->y*a[Z][Y] + b->z*a[Z][Z];
  vec_copy (&temp, out);
}
