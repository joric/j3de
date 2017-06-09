/*QUATERN.C*******************************************************************

   Clax: Portable keyframing library
         see inclosed LICENSE.TXT for licensing terms.

         for documentation, refer to CLAX.TXT

   author           : Borzom
   file created     : 16/04/97
   file description : quaternion math

   revision history :
     v0.10 (16/04/97) Borzom: Initial version.

   references       : Graphics Gems, Adept

   notes            :

*****************************************************************************/

#include <math.h>
#include <stdio.h>
#include "clax.h"
#include "claxi.h"

void qt_fromang (float ang, float x, float y, float z, c_QUAT *out)
{
/*
  qt_fromang: compute quaterion from [angle,axis] representation.
*/
  float s, omega;

  omega = ang / 2.0;
  s = sin (omega);
  out->w = cos (omega);
  out->x = x * s;
  out->y = y * s;
  out->z = z * s;
}

void qt_toang (c_QUAT *a, float *ang, float *x, float *y, float *z)
{
/*
  qt_toang: convert quaterion to [angle,axis] representation.
*/
  c_QUAT q;
  float  s, omega;

  qt_normalize (a, &q);
  omega = acos (q.w);
  *ang = omega * 2.0;
  s = sin (omega);
  if (fabs (s) > EPSILON) {
    s = 1.0 / s;
    *x = q.x * s;
    *y = q.y * s;
    *z = q.z * s;
  } else {
    *x = 0.0;
    *y = 0.0;
    *z = 0.0;
  }
}

void qt_make (float w, float x, float y, float z, c_QUAT *out)
{
/*
  qt_make: create quaternion (out = [w,[x,y,z]])
*/
  out->w = w;
  out->x = x;
  out->y = y;
  out->z = z;
}

void qt_identity (c_QUAT *out)
{
/*
  qt_identity: make identity quaternion (out = [1,0,0,0]).
*/
  out->w = 1.0;
  out->x = 0.0;
  out->y = 0.0;
  out->z = 0.0;
}

void qt_zero (c_QUAT *out)
{
/*
  qt_zero: zero quaternion (out = [0,0,0,0]).
*/
  out->w = 0.0;
  out->x = 0.0;
  out->y = 0.0;
  out->z = 0.0;
}

void qt_copy (c_QUAT *a, c_QUAT *out)
{
/*
  qt_copy: quaternion copy (out = a).
*/
  out->w = a->w;
  out->x = a->x;
  out->y = a->y;
  out->z = a->z;
}

void qt_print (c_QUAT *a)
{
/*
  qt_print: print quaternion on stdout.
*/
  printf ("w: %9.3f x: %9.3f y: %9.3f z: %9.3f\n", a->w, a->x, a->y, a->z);
}

void qt_add (c_QUAT *a, c_QUAT *b, c_QUAT *out)
{
/*
  qt_add: quaternion addition (out = a+b).
*/
  c_QUAT temp;

  temp.w = a->w + b->w;
  temp.x = a->x + b->x;
  temp.y = a->y + b->y;
  temp.z = a->z + b->z;
  qt_copy (&temp, out);
}

void qt_sub (c_QUAT *a, c_QUAT *b, c_QUAT *out)
{
/*
  qt_sub: quaterion substraction (out = a-b).
*/
  c_QUAT temp;

  temp.w = a->w - b->w;
  temp.x = a->x - b->x;
  temp.y = a->y - b->y;
  temp.z = a->z - b->z;
  qt_copy (&temp, out);
}

void qt_mul (c_QUAT *a, c_QUAT *b, c_QUAT *out)
{
/*
  qt_multiply: quaternion multiplication (out = a*b).
*/
  c_QUAT temp;

  temp.w = a->w*b->w - a->x*b->x - a->y*b->y - a->z*b->z;
  temp.x = a->w*b->x + a->x*b->w + a->y*b->z - a->z*b->y;
  temp.y = a->w*b->y + a->y*b->w + a->z*b->x - a->x*b->z;
  temp.z = a->w*b->z + a->z*b->w + a->x*b->y - a->y*b->x;
  qt_copy (&temp, out);
}

void qt_div (c_QUAT *a, c_QUAT *b, c_QUAT *out)
{
/*
  qt_div: quaterion division (out = a/b).
*/
  c_QUAT q, t, s, temp;

  qt_copy (b, &q);
  q.x = -q.x;
  q.y = -q.y;
  q.z = -q.z;
  qt_mul (a, &q, &t);
  qt_mul (&q, &q, &s);
  temp.w = t.w / s.w;
  temp.x = t.x / s.w;
  temp.y = t.y / s.w;
  temp.z = t.z / s.w;
  qt_copy (&temp, out);
}

void qt_square (c_QUAT *a, c_QUAT *out)
{
/*
  qt_square: quaternion square (out = a^2).
*/
  float  s;
  c_QUAT temp;

  s = 2 * a->w;
  temp.w = (a->w*a->w - a->x*a->x - a->y*a->y - a->z*a->z);
  temp.x = s * a->x;
  temp.y = s * a->y;
  temp.z = s * a->z;
  qt_copy (&temp, out);
}

void qt_sqrt (c_QUAT *a, c_QUAT *out)
{
/*
  qt_sqrt: quaternion square root (out = sqrt(a)).
*/
  float  len, m;
  float  A, B;
  c_QUAT r, temp;

  len = sqrt (a->w*a->w + a->x*a->x + a->y*a->y);
  if (len != 0.0) len = 1.0 / len; else len = 1.0;
  r.w = a->w * len;
  r.x = a->x * len;
  r.y = a->z * len;
  r.z = 0.0;
  m = 1.0 / sqrt (r.w*r.w + r.x*r.x);
  A = sqrt ((1.0 + r.y) * 0.5);
  B = sqrt ((1.0 - r.y) * 0.5);
  temp.w = sqrt (len) * B * r.w * m;
  temp.x = sqrt (len) * B * r.x * m;
  temp.y = sqrt (len) * A;
  temp.z = a->z;
  qt_copy (&temp, out);
}

float qt_length (c_QUAT *a)
{
/*
  qt_length: computes quaternion magnitude.
*/
  float len;

  return sqrt (a->w*a->w + a->x*a->x + a->y*a->y + a->z*a->z);
}

float qt_dot (c_QUAT *a, c_QUAT *b)
{
/*
  qt_dot: computes dot product of a*b.
*/
  float len;

  len = 1.0 / (qt_length (a) * qt_length (b));
  return (a->w*b->w + a->x*b->x + a->y*b->y + a->z*b->z) * len;
}

float qt_dotunit (c_QUAT *a, c_QUAT *b)
{
/*
  qt_dotunit: computes dot product of a*b.
*/
  return (a->w*b->w + a->x*b->x + a->y*b->y + a->z*b->z);
}

void qt_scale (c_QUAT *a, float s, c_QUAT *out)
{
/*
  qt_scale: scale quaternion.
*/
  c_QUAT temp;

  temp.w = a->w * s;
  temp.x = a->x * s;
  temp.y = a->y * s;
  temp.z = a->y * s;
  qt_copy (&temp, out);
}

void qt_rescale (c_QUAT *a, float s, c_QUAT *out)
{
/*
  qt_rescale: scale quaternion to a new length.
*/
  float  len;

  len = qt_length (a);
  if (len == 0.0) {
    qt_identity (out);
    return;
  }
  s /= len;
  qt_scale (a, s, out);
}

int32 qt_equal (c_QUAT *a, c_QUAT *b)
{
/*
  qt_equal: quaternion compare.
*/
  return (a->w == b->w && a->x == b->x && a->y == b->y && a->z == b->z);
}

void qt_normalize (c_QUAT *a, c_QUAT *out)
{
/*
  qt_normalize: quaternion normalization.
*/
  c_QUAT temp;

  qt_scale (a, 1.0 / qt_length (a), &temp);
  qt_copy (&temp, out);
}

void qt_inverse (c_QUAT *a, c_QUAT *out)
{
/*
  qt_inverse: form multiplicative inverse of quaternion.
*/
  c_QUAT temp;
  float  mag;

  mag = (a->w*a->w + a->x*a->x + a->y*a->y + a->z*a->z);
  if (mag != 1.0) mag = 1.0 / mag; else mag = 1.0;
  temp.w = a->w *  mag;
  temp.x = a->x * -mag;
  temp.y = a->y * -mag;
  temp.z = a->z * -mag;
  qt_copy (&temp, out);
}

void qt_negate (c_QUAT *a, c_QUAT *out)
{
/*
  qt_negate: negates quaternion.
*/
  c_QUAT temp;

  qt_normalize (a, &temp);
  temp.x = -temp.x;
  temp.y = -temp.y;
  temp.z = -temp.z;
  qt_copy (&temp, out);
}

void qt_exp (c_QUAT *a, c_QUAT *out)
{
/*
  qt_exp: compute quaternion exponent.
*/
  c_QUAT temp;
  float  len, len1;

  len = sqrt (a->x*a->x + a->y*a->y + a->z*a->z);
  if (len > 0.0) len1 = sin(len) / len; else len1 = 1.0;
  temp.w = cos (len);
  temp.x = a->x * len1;
  temp.y = a->y * len1;
  temp.z = a->z * len1;
  qt_copy (&temp, out);
}

void qt_log (c_QUAT *a, c_QUAT *out)
{
/*
  qt_log: compute quaternion logarithm.
*/
  c_QUAT temp;
  float  len;

  len = sqrt (a->x*a->x + a->y*a->y + a->z*a->z);
  if (a->w != 0.0) len = atan (len / a->w); else len = M_PI/2;
  temp.w = 0.0;
  temp.x = a->x * len;
  temp.y = a->y * len;
  temp.z = a->z * len;
  qt_copy (&temp, out);
}

void qt_lndif (c_QUAT *a, c_QUAT *b, c_QUAT *out)
{
/*
  qt_lndif: calculate logarithm of relative rotation from a to b.
*/
  c_QUAT inv, dif, temp;
  float  len, len1, s;

  qt_inverse (a, &inv);
  qt_mul (&inv, b, &dif);
  len = sqrt (dif.x*dif.x + dif.y*dif.y + dif.z*dif.z);
  s = qt_dot (a, b);
  if (s != 0.0) len1 = atan (len / s); else len1 = M_PI/2;
  if (len != 0.0) len1 /= len;
  temp.w = 0.0;
  temp.x = dif.x * len1;
  temp.y = dif.y * len1;
  temp.z = dif.z * len1;
  qt_copy (&temp, out);
}

void qt_slerp (c_QUAT *a, c_QUAT *b, float spin, float alpha, c_QUAT *out)
{
/*
  qt_slerp: spherical interpolation of quaternions.
*/
  c_QUAT temp;
  float  k1, k2;
  float  angle, anglespin;
  float  sina, cosa;
  int32  flip;

  cosa = qt_dotunit (a, b);
  if (cosa < 0.0) cosa = -cosa, flip = -1; else flip = 1;
  if ((1.0 - cosa) < EPSILON) {
    k1 = 1.0 - alpha;
    k2 = alpha;
  } else {
    angle = acos (cosa);
    sina = sin (angle);
    anglespin = angle + spin*M_PI;
    k1 = sin (angle - alpha*anglespin) / sina;
    k2 = sin (alpha*anglespin) / sina;
  }
  k2 *= flip;
  temp.x = k1*a->x + k2*b->x;
  temp.y = k1*a->y + k2*b->y;
  temp.z = k1*a->z + k2*b->z;
  temp.w = k1*a->w + k2*b->w;
  qt_copy (&temp, out);
}

void qt_slerpl (c_QUAT *a, c_QUAT *b, float spin, float alpha, c_QUAT *out)
{
/*
  qt_slerp: spherical interpolation of quaternions.
*/
  c_QUAT temp;
  float  k1, k2;
  float  angle, anglespin;
  float  sina, cosa;
  int32  flip;

  cosa = qt_dotunit (a, b);
  if (1.0 - fabs (cosa) < EPSILON) {
    k1 = 1.0 - alpha;
    k2 = alpha;
  } else {
    angle = acos (cosa);
    sina = sin (angle);
    anglespin = angle + spin*M_PI;
    k1 = sin (angle - alpha*anglespin) / sina;
    k2 = sin (alpha*anglespin) / sina;
  }
  temp.x = k1*a->x + k2*b->x;
  temp.y = k1*a->y + k2*b->y;
  temp.z = k1*a->z + k2*b->z;
  temp.w = k1*a->w + k2*b->w;
  qt_copy (&temp, out);
}

void qt_matrix (c_QUAT *a, c_MATRIX mat)
{
/*
  qt_matrix: convert a unit quaternion to rotation matrix.

      ( 1-yy-zz , xy+wz   , xz-wy   )
  T = ( xy-wz   , 1-xx-zz , yz+wx   )
      ( xz+wy   , yz-wx   , 1-xx-yy )
*/
  float x2, y2, z2, wx, wy, wz,
        xx, xy, xz, yy, yz, zz;

  x2 = a->x + a->x; y2 = a->y + a->y; z2 = a->z + a->z;
  wx = a->w * x2;   wy = a->w * y2;   wz = a->w * z2;
  xx = a->x * x2;   xy = a->x * y2;   xz = a->x * z2;
  yy = a->y * y2;   yz = a->y * z2;   zz = a->z * z2;

  mat[X][X] = 1.0 - (yy + zz);
  mat[X][Y] = xy + wz;
  mat[X][Z] = xz - wy;
  mat[X][W] = 0.0;
  mat[Y][X] = xy - wz;
  mat[Y][Y] = 1.0 - (xx + zz);
  mat[Y][Z] = yz + wx;
  mat[Y][W] = 0.0;
  mat[Z][X] = xz + wy;
  mat[Z][Y] = yz - wx;
  mat[Z][Z] = 1.0 - (xx + yy);
  mat[Z][W] = 0.0;
}

void qt_invmatrix (c_QUAT *a, c_MATRIX mat)
{
/*
  qt_invmatrix: convert a unit quaternion to inversed rotation matrix.

      ( 1-yy-zz , xy-wz   , xz+wy   )
  T = ( xy+wz   , 1-xx-zz , yz-wx   )
      ( xz-wy   , yz+wx   , 1-xx-yy )
*/
  float x2, y2, z2, wx, wy, wz,
        xx, xy, xz, yy, yz, zz;

  x2 = a->x + a->x; y2 = a->y + a->y; z2 = a->z + a->z;
  wx = a->w * x2;   wy = a->w * y2;   wz = a->w * z2;
  xx = a->x * x2;   xy = a->x * y2;   xz = a->x * z2;
  yy = a->y * y2;   yz = a->y * z2;   zz = a->z * z2;

  mat[X][X] = 1.0 - (yy + zz);
  mat[X][Y] = xy - wz;
  mat[X][Z] = xz + wy;
  mat[X][W] = 0.0;
  mat[Y][X] = xy + wz;
  mat[Y][Y] = 1.0 - (xx + zz);
  mat[Y][Z] = yz - wx;
  mat[Y][W] = 0.0;
  mat[Z][X] = xz - wy;
  mat[Z][Y] = yz + wx;
  mat[Z][Z] = 1.0 - (xx + yy);
  mat[Z][W] = 0.0;
}

void qt_frommat (c_MATRIX mat, c_QUAT *out)
{
/*
  qt_frommat: convert rotation matrix to quaternion.
*/
  float  tr, s;
  float  q[4];
  int16  i, j, k;
  static int nxt[3] = {Y, Z, X};

  tr = mat[X][X] + mat[Y][Y] + mat[Z][Z];
  if (tr > 0.0) {
    s = sqrt (tr + 1.0);
    out->w = s / 2.0;
    s = 0.5 / s;
    out->x = (mat[Y][Z] - mat[Z][Y]) * s;
    out->y = (mat[Z][X] - mat[X][Z]) * s;
    out->z = (mat[X][Y] - mat[Y][X]) * s;
  } else {
    i = X;
    if (mat[Y][Y] > mat[X][X]) i = Y;
    if (mat[Z][Z] > mat[i][i]) i = Z;
    j = nxt[i];
    k = nxt[j];
    s = sqrt ((mat[i][i] - (mat[j][j]+mat[k][k])) + 1.0);
    q[i] = s / 2.0;
    if (s != 0.0) s = 0.5 / s;
    q[W] = (mat[j][k] - mat[k][j]) * s;
    q[j] = (mat[i][j] + mat[j][i]) * s;
    q[k] = (mat[i][k] + mat[k][i]) * s;
    out->w = q[W];
    out->x = q[X];
    out->y = q[Y];
    out->z = q[Z];
  }
}
