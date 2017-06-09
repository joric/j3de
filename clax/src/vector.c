/*VECTOR.C********************************************************************

   Clax: Portable keyframing library
         see inclosed LICENSE.TXT for licensing terms.

         for documentation, refer to CLAX.TXT

   author           : Borzom
   file created     : 29/04/97
   file description : vector math

   revision history :
     v0.10 (29/04/97) Borzom: Initial version.

   notes            :

*****************************************************************************/

#include <math.h>
#include <stdio.h>
#include "clax.h"
#include "claxi.h"

void vec_make (float x, float y, float z, c_VECTOR *out)
{
/*
  vec_make: create vector (out = [x,y,z]).
*/
  out->x = x;
  out->y = y;
  out->z = z;
}

void vec_zero (c_VECTOR *out)
{
/*
  vec_zero: zero vector (out = [0,0,0]).
*/
  out->x = 0.0;
  out->y = 0.0;
  out->z = 0.0;
}

void vec_copy (c_VECTOR *a, c_VECTOR *out)
{
/*
  vec_copy: vector copy (out = a).
*/
  out->x = a->x;
  out->y = a->y;
  out->z = a->z;
}

void vec_print (c_VECTOR *a)
{
/*
  vec_print: print vector on stdout.
*/
  printf ("x: %9.3f y: %9.3f z: %9.3f\n", a->x, a->y, a->z);
}

void vec_add (c_VECTOR *a, c_VECTOR *b, c_VECTOR *out)
{
/*
  vec_add: vector addition (out = a+b).
*/
  c_VECTOR temp;

  temp.x = a->x + b->x;
  temp.y = a->y + b->y;
  temp.z = a->z + b->z;
  vec_copy (&temp, out);
}

void vec_sub (c_VECTOR *a, c_VECTOR *b, c_VECTOR *out)
{
/*
  vec_sub: vector substraction (out = a-b).
*/
  c_VECTOR temp;

  temp.x = a->x - b->x;
  temp.y = a->y - b->y;
  temp.z = a->z - b->z;
  vec_copy (&temp, out);
}

void vec_mul (c_VECTOR *a, c_VECTOR *b, c_VECTOR *out)
{
/*
  vec_mul: vector multiplication (out = a*b).
*/
  c_VECTOR temp;

  temp.x = a->x * b->x;
  temp.y = a->y * b->y;
  temp.z = a->z * b->z;
  vec_copy (&temp, out);
}

void vec_scale (c_VECTOR *a, float s, c_VECTOR *out)
{
/*
  vec_scale: vector multiplication by a scalar.
*/
  c_VECTOR temp;

  temp.x = a->x * s;
  temp.y = a->y * s;
  temp.z = a->z * s;
  vec_copy (&temp, out);
}

void vec_rescale (c_VECTOR *a, float s, c_VECTOR *out)
{
/*
  vec_scale: vector multiplication by a scalar.
*/
  c_VECTOR temp;
  float    len;

  len = (a->x*a->x + a->y*a->y + a->z*a->z);
  if (len == 0.0) {
    out->x = 0.0;
    out->y = 0.0;
    out->z = 0.0;
    return;
  }
  s *= (1.0 / len);
  temp.x = a->x * s;
  temp.y = a->y * s;
  temp.z = a->y * s;
  vec_copy (&temp, out);
}

void vec_negate (c_VECTOR *a, c_VECTOR *out)
{
/*
  vec_negate: vector negation (out = -a).
*/
  c_VECTOR temp;

  temp.x = -a->x;
  temp.y = -a->y;
  temp.z = -a->z;
  vec_copy (&temp, out);
}

int32 vec_equal (c_VECTOR *a, c_VECTOR *b)
{
/*
  vec_equal: vector compare.
*/
  return (a->x == b->x && a->y == b->y && a->z == b->z);
}

float vec_length (c_VECTOR *a)
{
/*
  vec_length: computes vector magnitude.
*/
  float len;

  len = sqrt (a->x*a->x + a->y*a->y + a->z*a->z);
  if (len == 0.0) len = 1.0;
  return (len);
}

float vec_distance (c_VECTOR *a, c_VECTOR *b)
{
/*
  vec_distance: computes distance between two vectors.
*/
  c_VECTOR v;

  vec_sub (a, b, &v);
  return (vec_length (&v));
}

float vec_dot (c_VECTOR *a, c_VECTOR *b)
{
/*
  vec_dot: computes dot product of two vectors.
*/
  float mag;

  mag = 1.0 / (vec_length(a) * vec_length(b));
  return (a->x*b->x + a->y*b->y + a->z*b->z) * mag;
}

float vec_dotunit (c_VECTOR *a, c_VECTOR *b)
{
/*
  vec_dotunit: computes dot product of two vectors.
*/
  return (a->x*b->x + a->y*b->y + a->z*b->z);
}

void vec_cross (c_VECTOR *a, c_VECTOR *b, c_VECTOR *out)
{
/*
  vec_cross: computes cross product of two vectors.
*/
  c_VECTOR temp;

  temp.x = a->y*b->z - a->z*b->y;
  temp.y = a->z*b->x - a->x*b->z;
  temp.z = a->x*b->y - a->y*b->x;
  vec_copy (&temp, out);
}

void vec_midpoint (c_VECTOR *a, c_VECTOR *b, c_VECTOR *out)
{
/*
  vec_midpoint: computes middle point of two vectors.
*/
  c_VECTOR temp;

  vec_add (a, b, &temp);
  vec_scale (&temp, 0.5, &temp);
  vec_copy (&temp, out);
}

void vec_lerp (c_VECTOR *a, c_VECTOR *b, float alpha, c_VECTOR *out)
{
/*
  vec_lerp: linear interpolation of two vectors.
*/
  c_VECTOR temp;

#define _LERP(a,l,h) ((l)+(((h)-(l))*(a)))
  temp.x = _LERP (alpha, a->x, b->x);
  temp.y = _LERP (alpha, a->y, b->y);
  temp.z = _LERP (alpha, a->z, b->z);
#undef _LERP
  vec_copy (&temp, out);
}

void vec_combine (c_VECTOR *a, c_VECTOR *b, float as, float bs,
                  c_VECTOR *out)
{
/*
  vec_combine: compute linear combination of two vectors.
*/
  c_VECTOR temp;

  temp.x = (as * a->x) + (bs * b->x);
  temp.y = (as * a->y) + (bs * b->y);
  temp.z = (as * a->z) + (bs * b->z);
  vec_copy (&temp, out);
}

void vec_normalize (c_VECTOR *a, c_VECTOR *out)
{
/*
  vec_normalize: vector normalization.
*/
  float    len;
  c_VECTOR temp;

  len = vec_length (a);
  if (len == 0.0) {
    out->x = 0.0;
    out->y = 0.0;
    out->z = 0.0;
    return;
  }
  len = 1.0 / len;
  temp.x = a->x * len;
  temp.y = a->y * len;
  temp.z = a->z * len;
  vec_copy (&temp, out);
}

