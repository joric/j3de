/*SPLINE.C********************************************************************

   Clax: Portable keyframing library
         see inclosed LICENSE.TXT for licensing terms.

         for documentation, refer to CLAX.TXT

   author           : Adept
   file created     : 17/04/97
   file description : spline interpolation routines

   revision history :
     ----- (06/03/97) Adept:  Initial version (spline_1).
     v0.10 (17/04/97) Borzom: Changed to fit clax structure.
           (19/05/97) Borzom: Added 2-key spline support.
           (08/06/97) Borzom: Added track loop/repeat support.

   notes            :
     It is Kochanek-Bartels spline sub-type of hermit curve developped
     especially for computer key animations, first presented at SIGGRAPH'84.

     correct quaternion interpolation is still missing.

*****************************************************************************/

#include <math.h>
#include <stdio.h>
#include "clax.h"
#include "claxi.h"

static void CompElementDeriv (float pp,  float p,   float pn,
                              float *ds, float *dd, float ksm,
                              float ksp, float kdm, float kdp)
{
  float delm, delp;

  delm = p - pp;
  delp = pn - p;
  *ds  = ksm*delm + ksp*delp;
  *dd  = kdm*delm + kdp*delp;
}

static void CompDeriv (t_KEY *keyp, t_KEY *key, t_KEY *keyn)
{
/*
  CompDeriv: compute derivative for key "key".
*/
  float tm, cm, cp, bm, bp, tmcm, tmcp;
  float ksm, ksp, kdm, kdp;
  float dt, fp, fn, c;

  dt = 0.5f * (keyn->frame - keyp->frame);
  fp = (key->frame - keyp->frame) / dt;
  fn = (keyn->frame - key->frame) / dt;
  c  = (float)fabs (key->cont);
  fp = fp + c - c * fp;
  fn = fn + c - c * fn;
  cm = 1.0f - key->cont;
  tm = 0.5f * (1.0f - key->tens);
  cp = 2.0f - cm;
  bm = 1.0f - key->bias;
  bp = 2.0f - bm;
  tmcm = tm * cm;
  tmcp = tm * cp;
  ksm = tmcm * bp * fp; ksp = tmcp * bm * fp;
  kdm = tmcp * bp * fn; kdp = tmcm * bm * fn;
  CompElementDeriv (keyp->val._quat.w, key->val._quat.w, keyn->val._quat.w,
                    &key->dsa, &key->dda, ksm, ksp, kdm, kdp);
  CompElementDeriv (keyp->val._quat.x, key->val._quat.x, keyn->val._quat.x,
                    &key->dsb, &key->ddb, ksm, ksp, kdm, kdp);
  CompElementDeriv (keyp->val._quat.y, key->val._quat.y, keyn->val._quat.y,
                    &key->dsc, &key->ddc, ksm, ksp, kdm, kdp);
  CompElementDeriv (keyp->val._quat.z, key->val._quat.z, keyn->val._quat.z,
                    &key->dsd, &key->ddd, ksm, ksp, kdm, kdp);
}

static void CompDerivFirst (t_KEY *key, t_KEY *keyn, t_KEY *keynn)
{
/*
  CompDerivFirst: compute derivative for first key "key".
*/
  float f20, f10, v20, v10;

  f20 = keynn->frame - key->frame;
  f10 = keyn->frame - key->frame;
  v20 = keynn->val._quat.w - key->val._quat.w;
  v10 = keyn->val._quat.w - key->val._quat.w;
  key->dda = (1-key->tens)*(v20*(0.25f-f10/(2*f20))+(v10-v20/2)*3/2+v20/2);
  v20 = keynn->val._quat.x - key->val._quat.x;
  v10 = keyn->val._quat.x - key->val._quat.x;
  key->ddb = (1-key->tens)*(v20*(0.25f-f10/(2*f20))+(v10-v20/2)*3/2+v20/2);
  v20 = keynn->val._quat.y - key->val._quat.y;
  v10 = keyn->val._quat.y - key->val._quat.y;
  key->ddc = (1-key->tens)*(v20*(0.25f-f10/(2*f20))+(v10-v20/2)*3/2+v20/2);
  v20 = keynn->val._quat.z - key->val._quat.z;
  v10 = keyn->val._quat.z - key->val._quat.z;
  key->ddd = (1-key->tens)*(v20*(0.25f-f10/(2*f20))+(v10-v20/2)*3/2+v20/2);
}

static void CompDerivLast (t_KEY *keypp, t_KEY *keyp, t_KEY *key)
{
/*
  CompDerivLast: compute derivative for last "key".
*/
  float f20, f10, v20, v10;

  f20 = key->frame - keypp->frame;
  f10 = key->frame - keyp->frame;
  v20 = key->val._quat.w - keypp->val._quat.w;
  v10 = key->val._quat.w - keyp->val._quat.w;
  key->dsa = (1-key->tens)*(v20*(0.25f-f10/(2*f20))+(v10-v20/2)*3/2+v20/2);
  v20 = key->val._quat.x - keypp->val._quat.x;
  v10 = key->val._quat.x - keyp->val._quat.x;
  key->dsb = (1-key->tens)*(v20*(0.25f-f10/(2*f20))+(v10-v20/2)*3/2+v20/2);
  v20 = key->val._quat.y - keypp->val._quat.y;
  v10 = key->val._quat.y - keyp->val._quat.y;
  key->dsc = (1-key->tens)*(v20*(0.25f-f10/(2*f20))+(v10-v20/2)*3/2+v20/2);
  v20 = key->val._quat.z - keypp->val._quat.z;
  v10 = key->val._quat.z - keyp->val._quat.z;
  key->dsd = (1-key->tens)*(v20*(0.25f-f10/(2*f20))+(v10-v20/2)*3/2+v20/2);
}

static void CompDerivLoopFirst (t_KEY *keyp, t_KEY *key, t_KEY *keyn, float lf)
{
/*
  CompDerivLoopFirst: compute derivative for first key "key".
*/
  float tm, cm, cp, bm, bp, tmcm, tmcp;
  float ksm, ksp, kdm, kdp;
  float dt, fp, fn, c;

  dt = 0.5f * (keyn->frame - keyp->frame + lf);
  fp = (key->frame - keyp->frame + lf) / dt;
  fn = (keyn->frame - key->frame) / dt;
  c  = (float)fabs (key->cont);
  fp = fp + c - c * fp;
  fn = fn + c - c * fn;
  cm = 1.0f - key->cont;
  tm = 0.5f * (1.0f - key->tens);
  cp = 2.0f - cm;
  bm = 1.0f - key->bias;
  bp = 2.0f - bm;
  tmcm = tm * cm;
  tmcp = tm * cp;
  ksm = tmcm * bp * fp; ksp = tmcp * bm * fp;
  kdm = tmcp * bp * fn; kdp = tmcm * bm * fn;
  CompElementDeriv (keyp->val._quat.w, key->val._quat.w, keyn->val._quat.w,
                    &key->dsa, &key->dda, ksm, ksp, kdm, kdp);
  CompElementDeriv (keyp->val._quat.x, key->val._quat.x, keyn->val._quat.x,
                    &key->dsb, &key->ddb, ksm, ksp, kdm, kdp);
  CompElementDeriv (keyp->val._quat.y, key->val._quat.y, keyn->val._quat.y,
                    &key->dsc, &key->ddc, ksm, ksp, kdm, kdp);
  CompElementDeriv (keyp->val._quat.z, key->val._quat.z, keyn->val._quat.z,
                    &key->dsd, &key->ddd, ksm, ksp, kdm, kdp);
}

static void CompDerivLoopLast (t_KEY *keyp, t_KEY *key, t_KEY *keyn, float lf)
{
/*
  CompDerivLoopLast: compute derivative for last key "key".
*/
  float tm, cm, cp, bm, bp, tmcm, tmcp;
  float ksm, ksp, kdm, kdp;
  float dt, fp, fn, c;

  dt = 0.5f * (keyn->frame - keyp->frame + lf);
  fp = (key->frame - keyp->frame) / dt;
  fn = (keyn->frame - key->frame + lf) / dt;
  c  = (float)fabs (key->cont);
  fp = fp + c - c * fp;
  fn = fn + c - c * fn;
  cm = 1.0f - key->cont;
  tm = 0.5f * (1.0f - key->tens);
  cp = 2.0f - cm;
  bm = 1.0f - key->bias;
  bp = 2.0f - bm;
  tmcm = tm * cm;
  tmcp = tm * cp;
  ksm = tmcm * bp * fp; ksp = tmcp * bm * fp;
  kdm = tmcp * bp * fn; kdp = tmcm * bm * fn;
  CompElementDeriv (keyp->val._quat.w, key->val._quat.w, keyn->val._quat.w,
                    &key->dsa, &key->dda, ksm, ksp, kdm, kdp);
  CompElementDeriv (keyp->val._quat.x, key->val._quat.x, keyn->val._quat.x,
                    &key->dsb, &key->ddb, ksm, ksp, kdm, kdp);
  CompElementDeriv (keyp->val._quat.y, key->val._quat.y, keyn->val._quat.y,
                    &key->dsc, &key->ddc, ksm, ksp, kdm, kdp);
  CompElementDeriv (keyp->val._quat.z, key->val._quat.z, keyn->val._quat.z,
                    &key->dsd, &key->ddd, ksm, ksp, kdm, kdp);
}

static void CompDerivTwo (t_KEY *key)
{
/*
  CompDerivTwo: compute derivative for keys "key" (2-key spline).
*/
  t_KEY *keyn = key->next;

  key->dsa = 0.0;
  key->dsb = 0.0;
  key->dsc = 0.0;
  key->dsd = 0.0;
  key->dda = (keyn->val._quat.w - key->val._quat.w) * (1 - key->tens);
  key->ddb = (keyn->val._quat.x - key->val._quat.x) * (1 - key->tens);
  key->ddc = (keyn->val._quat.y - key->val._quat.y) * (1 - key->tens);
  key->ddd = (keyn->val._quat.z - key->val._quat.z) * (1 - key->tens);
  keyn->dda = 0.0;
  keyn->ddb = 0.0;
  keyn->ddc = 0.0;
  keyn->ddd = 0.0;
  keyn->dsa = (keyn->val._quat.w - key->val._quat.w) * (1 - keyn->tens);
  keyn->dsb = (keyn->val._quat.x - key->val._quat.x) * (1 - keyn->tens);
  keyn->dsc = (keyn->val._quat.y - key->val._quat.y) * (1 - keyn->tens);
  keyn->dsd = (keyn->val._quat.z - key->val._quat.z) * (1 - keyn->tens);
}

static void CompAB (t_KEY *prev, t_KEY *cur, t_KEY *next)
{
/*
  CompAB: Compute derivatives for rotation keys.
*/
  c_QUAT qprev, qnext, q;
  c_QUAT qp, qm, qa, qb, qae, qbe;
  c_QUAT QA, QB, QC;        /* key quaternions */
  c_QUAT QAA, QAB, QAC;     /* key angle/axis representation */
  float  tm, cm, cp, bm, bp, tmcm, tmcp, ksm, ksp, kdm, kdp;
  float  dt, fp, fn, c;

  qt_copy (&cur->val._quat, &QAB);
  qt_copy (&cur->qa, &QB);
  if (prev) {
    qt_copy (&prev->val._quat, &QAA);
    qt_copy (&prev->qa, &QA);
  }
  if (next) {
    qt_copy (&next->val._quat, &QAC);
    qt_copy (&next->qa, &QC);
  }
  if (prev != NULL) {
    if (fabs (QAB.w - QAA.w) > 2*M_PI - EPSILON) {
      qt_copy (&QAB, &q);
      q.w = 0.0;
      qt_log (&q, &qm);
     } else {
      qt_copy (&QA, &qprev);
      if (qt_dotunit (&qprev, &QB) < 0.0) qt_negate (&qprev, &qprev);
      qt_lndif (&qprev, &QB, &qm);
    }
  }
  if (next != NULL) {
    if (fabs (QAC.w - QAB.w) > 2*M_PI - EPSILON) {
      qt_copy (&QAC, &q);
      q.w = 0.0;
      qt_log (&q, &qp);
    } else {
      qt_copy (&QC, &qnext);
      if (qt_dotunit (&qnext, &QB) < 0.0) qt_negate (&qnext, &qnext);
      qt_lndif (&QB, &qnext, &qp);
    }
  }
  if (prev == NULL) qt_copy (&qp, &qm);
  if (next == NULL) qt_copy (&qm, &qp);
  fp = fn = 1.0f;
  cm = 1.0f - cur->cont;
  if (prev && next) {
    dt = 0.5f * (next->frame - prev->frame);
    fp = (cur->frame - prev->frame) / dt;
    fn = (next->frame - cur->frame) / dt;
    c = (float)fabs( cur->cont );
    fp = fp + c - c * fp;
    fn = fn + c - c * fn;
  }
  tm = 0.5f * (1.0f - cur->tens);
  cp = 2.0f - cm;
  bm = 1.0f - cur->bias;
  bp = 2.0f - bm;
  tmcm = tm * cm;
  tmcp = tm * cp;
  ksm  = 1.0f - tmcm * bp * fp;
  ksp  = -tmcp * bm * fp;
  kdm  = tmcp * bp * fn;
  kdp  = tmcm * bm * fn - 1.0f;
  qa.x = 0.5f * (kdm * qm.x + kdp * qp.x);
  qb.x = 0.5f * (ksm * qm.x + ksp * qp.x);
  qa.y = 0.5f * (kdm * qm.y + kdp * qp.y);
  qb.y = 0.5f * (ksm * qm.y + ksp * qp.y);
  qa.z = 0.5f * (kdm * qm.z + kdp * qp.z);
  qb.z = 0.5f * (ksm * qm.z + ksp * qp.z);
  qa.w = 0.5f * (kdm * qm.w + kdp * qp.w);
  qb.w = 0.5f * (ksm * qm.w + ksp * qp.w);
  qt_exp (&qa, &qae);
  qt_exp (&qb, &qbe);
  qt_mul (&QB, &qae, &cur->ds);
  qt_mul (&QB, &qbe, &cur->dd);
}

float spline_ease (float t, float a, float b)
{
/*
  spline_ease: remap parameter between two keys to apply eases.
*/
  float k;
  float s = a+b;

  if (s == 0.0) return t;
  if (s > 1.0f) {
    a = a/s;
    b = b/s;
  }
  k = 1.0f/(2.0f-a-b);
  if (t < a) return ((k/a)*t*t);
    else {
      if (t < 1.0f-b) return (k*(2*t-a));
        else {
          t = 1.0f-t;
          return (1.0f-(k/b)*t*t);
        }
    }
}

int32 spline_init (t_TRACK *track)
{
/*
  spline_init: compute spline derivations.
*/
  t_KEY *curr;
  t_KEY *keys = track->keys;
  t_KEY *last = track->last;

  if (!keys) return clax_err_nullptr;
  if (!keys->next) return clax_err_spline;

  if (keys->next->next) { /* 3 or more keys */
    for (curr = keys->next; curr->next; curr = curr->next)
      CompDeriv (curr->prev, curr, curr->next);
    if (track->flags & clax_track_loop) {
      CompDerivLoopFirst (last->prev, keys, keys->next, track->frames);
      CompDerivLoopLast (last->prev, last, keys->next, track->frames);
    } else {
      CompDerivFirst (keys, keys->next, keys->next->next);
      CompDerivLast (curr->prev->prev, curr->prev, curr);
    }
  } else CompDerivTwo (keys); /* 2 keys */
  return clax_err_ok;
}

int32 spline_initrot (t_TRACK *track)
{
/*
  spline_initrot: compute spline derivations for rotation track.
*/
  t_KEY *curr;
  t_KEY *keys = track->keys;
  t_KEY *last = track->last;

  if (!keys) return clax_err_nullptr;
  if (!keys->next) return clax_err_spline;

  if (keys->next->next) { /* 3 or more keys */
    for (curr = keys->next; curr->next; curr = curr->next)
      CompAB (curr->prev, curr, curr->next);
/*    if (track->flags & clax_track_loop) {
      CompAB (last->prev, keys, keys->next);
      CompAB (keys->prev, last, keys->next);
    } else { */
      CompAB (NULL, keys, keys->next);
      CompAB (keys, last, NULL);
//    }
  } else {
    CompAB (NULL, keys, keys->next);
    CompAB (keys, last, NULL);
  }
  return clax_err_ok;
}

int32 spline_getkey_float (t_TRACK *track, float frame, float *out)
{
/*
  spline_getkey_float: get spline interpolated float for frame "frame".
*/
  t_KEY *keys;
  float  t, t2, t3;
  float  h[4];

  if (frame < 0.0) return clax_err_badframe;
  if (!track || !track->keys) return clax_err_nullptr;

  if (frame < track->last->frame) keys = track->keys; else
    keys = track->last;
  while (keys->next && frame > keys->next->frame) keys = keys->next;
  track->last = keys;
  if (!keys->next || frame < keys->frame) { /* frame is above last key */
    *out = keys->val._float;
    return clax_err_ok;
  }
  t = (frame - keys->frame) / (keys->next->frame - keys->frame);
  t = spline_ease (t, keys->easefrom, keys->next->easeto);
  t2 = t * t;
  t3 = t2 * t;
  h[0] =  2 * t3 - 3 * t2 + 1;
  h[1] = -2 * t3 + 3 * t2;
  h[2] = t3 - 2 * t2 + t;
  h[3] = t3 - t2;
  *out = (h[0]*keys->val._float) + (h[1]*keys->next->val._float) +
         (h[2]*keys->dda) +        (h[3]*keys->next->dsa);
  return clax_err_ok;
}

int32 spline_getkey_vect (t_TRACK *track, float frame, c_VECTOR *out)
{
/*
  spline_getkey_vect: get spline interpolated vector for frame "frame".
*/
  t_KEY *keys;
  float  t, t2, t3;
  float  h[4];

  if (frame < 0.0) return clax_err_badframe;
  if (!track || !track->keys) return clax_err_nullptr;

  if (frame < track->last->frame) keys = track->keys; else
    keys = track->last;
  while (keys->next && frame > keys->next->frame) keys = keys->next;
  track->last = keys;
  if (!keys->next || frame < keys->frame) { /* frame is above last key */
    vec_copy (&keys->val._vect, out);
    return clax_err_ok;
  }
  t = (frame - keys->frame) / (keys->next->frame - keys->frame);
  t = spline_ease (t, keys->easefrom, keys->next->easeto);
  t2 = t * t;
  t3 = t2 * t;
  h[0] =  2 * t3 - 3 * t2 + 1;
  h[1] = -2 * t3 + 3 * t2;
  h[2] = t3 - 2 * t2 + t;
  h[3] = t3 - t2;
  out->x = (h[0]*keys->val._vect.x) + (h[1]*keys->next->val._vect.x) +
           (h[2]*keys->dda) +         (h[3]*keys->next->dsa);
  out->y = (h[0]*keys->val._vect.y) + (h[1]*keys->next->val._vect.y) +
           (h[2]*keys->ddb) +         (h[3]*keys->next->dsb);
  out->z = (h[0]*keys->val._vect.z) + (h[1]*keys->next->val._vect.z) +
           (h[2]*keys->ddc) +         (h[3]*keys->next->dsc);
  return clax_err_ok;
}

int32 spline_getkey_quat (t_TRACK *track, float frame, c_QUAT *out)
{
/*
  spline_getkey_quat: get spline interpolated quaternion for frame "frame".
*/
  t_KEY  *keys;
  c_QUAT a, b, p, q, q1;
  float  t, angle, spin;
  
  if (frame < 0.0) return clax_err_badframe;
  if (!track || !track->keys) return clax_err_nullptr;

  if (frame < track->last->frame) keys = track->keys; else
    keys = track->last;
  while (keys->next && frame > keys->next->frame) keys = keys->next;
  track->last = keys;
  if (!keys->next || frame < keys->frame) { /* frame is above last key */
    qt_copy (&keys->qa, out);
    return clax_err_ok;
  }
  t = (frame - keys->frame) / (keys->next->frame - keys->frame);
  t = spline_ease (t, keys->easefrom, keys->next->easeto);
  qt_copy (&keys->qa, &a);
  qt_copy (&keys->next->qa, &b);

  angle = keys->next->val._quat.w - keys->val._quat.w;
  if (angle > 0) spin = (float)floor (angle / (2*M_PI));
    else spin = (float)ceil (angle / (2*M_PI));
//  !!! FIX !!!
//  angle = angle - (2*M_PI)*spin;
//  if (fabs (angle) > M_PI) {
    qt_slerpl (&a,&b,spin,t,&p);
    qt_slerpl (&keys->dd, &keys->next->ds,spin,t,&q);
    t = (((1.0f-t)*2.0f)*t);
    qt_slerpl (&p,&q,0,t,&q1);
//  } else {
//    qt_slerp (&a,&b,spin,t,&p);
//    qt_slerp (&keys->dd,&keys->next->ds,spin,t,&q);
//    t = (((1.0-t)*2.0)*t);
//    qt_slerp (&p,&q,0,t,&q1);
//  }
  qt_copy (&q1, out);
  return clax_err_ok;
}
