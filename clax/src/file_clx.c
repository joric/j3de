/*FILE_CLX.C******************************************************************

   Clax: Portable keyframing library
         see inclosed LICENSE.TXT for licensing terms.

         for documentation, refer to CLAX.TXT

   author           : Borzom
   file created     : 19/06/97
   file description : Clax fileformat driver (world + motion)

   revision history :
     v0.10 (19/06/97) Borzom: Initial version.

   notes            :

*****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "clax.h"
#include "claxi.h"

/*****************************************************************************
  chunks/readers definitions, structures
*****************************************************************************/

#define  FORMAT_MAGIC   'XC'
#define  FORMAT_VERSION 1.0

enum clax_track_types_ { /* track key types */
  track_float  = 0,              /* float      */
  track_vector = 1,              /* vector     */
  track_quat   = 2,              /* quaternion */
  track_int    = 3               /* integer    */
};

enum clax_format_flags_ { /* .CLX format flags */
  flag_world  = 1,               /* includes world information     */
  flag_motion = 2                /* includes keyframer information */
};

enum clax_format_chunks_ { /* .CLX format chunk types */
  CHUNK_WORLD     = 0x100, CHUNK_KEYFRAMER = 0x200,
  CHUNK_OCAMERA   = 0x300, CHUNK_OLIGHT    = 0x301,
  CHUNK_OOBJECT   = 0x302, CHUNK_OMATERIAL = 0x303,
  CHUNK_OAMBIENT  = 0x304, CHUNK_TCAMERA   = 0x400,
  CHUNK_TCAMTGT   = 0x401, CHUNK_TLIGHT    = 0x402,
  CHUNK_TSPOTL    = 0x403, CHUNK_TLIGHTTGT = 0x404,
  CHUNK_TOBJECT   = 0x405, CHUNK_TAMBIENT  = 0x406
};

typedef struct { /* CLX header structure */
  int magic;                     /* magic number   */
  int version;                   /* format version */
  int flags;                     /* flags          */
} c_HEADER;

typedef struct { /* CLX chunk structure */
  int16 chunk_id;                /* chunk id (clax_format_chunks_) */
  int32 chunk_size;              /* chunk size                     */
} c_CHUNK;

typedef struct { /* Chunk world list structure */
  int  id;                       /* chunk id                  */
  int  (*func_read)  (FILE *f);  /* reader function           */
  void (*func_write) (FILE *f, w_NODE *n); /* writer function */
} c_LISTWORLD;

typedef struct { /* Chunk key list structure */
  int  id;                       /* chunk id                  */
  int  (*func_read)  (FILE *f);  /* reader function           */
  void (*func_write) (FILE *f, k_NODE *n); /* writer function */
} c_LISTKEY;

static int read_NULL        (FILE *f); /* (skip reader)               */
static int read_OCAMERA     (FILE *f); /* world: camera               */
static int read_OLIGHT      (FILE *f); /* world: light                */
static int read_OOBJECT     (FILE *f); /* world: object               */
static int read_OMATERIAL   (FILE *f); /* world: material             */
static int read_OAMBIENT    (FILE *f); /* world: ambient              */
static int read_KEYFRAMER   (FILE *f); /* keyframer: info             */
static int read_TCAMERA     (FILE *f); /* keyframer: camera           */
static int read_TCAMTGT     (FILE *f); /* keyframer: camera target    */
static int read_TLIGHT      (FILE *f); /* keyframer: light            */
static int read_TSPOTL      (FILE *f); /* keyframer: spotlight        */
static int read_TLIGHTTGT   (FILE *f); /* keyframer: light target     */
static int read_TOBJECT     (FILE *f); /* keyframer: object           */
static int read_TAMBIENT    (FILE *f); /* keyframer: ambient          */
static void write_OCAMERA   (FILE *f, w_NODE *n); /* w: camera        */
static void write_OLIGHT    (FILE *f, w_NODE *n); /* w: light         */
static void write_OOBJECT   (FILE *f, w_NODE *n); /* w: object        */
static void write_OMATERIAL (FILE *f, w_NODE *n); /* w: material      */
static void write_OAMBIENT  (FILE *f, w_NODE *n); /* w: ambient       */
static void write_TCAMERA   (FILE *f, k_NODE *n); /* k: camera        */
static void write_TCAMTGT   (FILE *f, k_NODE *n); /* k: camera target */
static void write_TLIGHT    (FILE *f, k_NODE *n); /* k: light         */
static void write_TSPOTL    (FILE *f, k_NODE *n); /* k: spotlight     */
static void write_TLIGHTTGT (FILE *f, k_NODE *n); /* k: light target  */
static void write_TOBJECT   (FILE *f, k_NODE *n); /* k: object        */
static void write_TAMBIENT  (FILE *f, k_NODE *n); /* k: ambient       */

static c_LISTWORLD world_chunks[] = { /* World definition chunks */
  {CHUNK_OCAMERA,   read_OCAMERA,   write_OCAMERA},
  {CHUNK_OLIGHT,    read_OLIGHT,    write_OLIGHT},
  {CHUNK_OOBJECT,   read_OOBJECT,   write_OOBJECT},
  {CHUNK_OMATERIAL, read_OMATERIAL, write_OMATERIAL},
  {CHUNK_OAMBIENT,  read_OAMBIENT,  write_OAMBIENT}
};

static c_LISTKEY key_chunks[] = { /* Keyframer chunks */
  {CHUNK_KEYFRAMER, read_KEYFRAMER, NULL},
  {CHUNK_TCAMERA,   read_TCAMERA,   write_TCAMERA},
  {CHUNK_TCAMTGT,   read_TCAMTGT,   write_TCAMTGT},
  {CHUNK_TLIGHT,    read_TLIGHT,    write_TLIGHT},
  {CHUNK_TSPOTL,    read_TSPOTL,    write_TSPOTL},
  {CHUNK_TLIGHTTGT, read_TLIGHTTGT, write_TLIGHTTGT},
  {CHUNK_TOBJECT,   read_TOBJECT,   write_TOBJECT},
  {CHUNK_TAMBIENT,  read_TAMBIENT,  write_TAMBIENT}
};

static char c_string[64];        /* temporary string */

/*****************************************************************************
  internal functions
*****************************************************************************/

static char *strcopy (char *src)
{
/*
  strcopy: copies asciiz string with memory allocation.
           i know, i could use strdup(), but its not ANSI C.
*/
  char *temp;

  if ((temp = (char *)malloc (strlen (src)+1)) == NULL) return NULL;
  strcpy (temp, src);
  return temp;
}

static t_TRACK *alloc_track ()
{
/*
  alloc_track: allocate memory for a track.
*/
  t_TRACK *track;

  track = (t_TRACK *)malloc (sizeof (t_TRACK));
  track->keys = NULL;
  track->last = NULL;
  track->flags = 0;
  track->frames = 0.0;
  track->numkeys = 0;
  return track;
}

static int add_key (t_TRACK *track, t_KEY *key, int frame)
{
/*
  add_key: add a key to track.
*/
  if (track == NULL || key == NULL) return clax_err_nullptr;
  key->frame = frame;
  key->next = NULL;
  if (track->keys == NULL) {
    key->prev = NULL;
    track->keys = key;
  } else {
    key->prev = track->last;
    track->last->next = key;
  }
  track->frames = key->frame;
  track->last = key;
  track->numkeys++;
  return clax_err_ok;
}

static int translate_chunk_world (int type)
{
/*
  translate_chunk_world: translate clax world type to chunk.
*/
  switch (type) {
    case clax_obj_camera:      return CHUNK_OCAMERA;
    case clax_obj_light:       return CHUNK_OLIGHT;
    case clax_obj_object:      return CHUNK_OOBJECT;
    case clax_obj_material:    return CHUNK_OMATERIAL;
    case clax_obj_ambient:     return CHUNK_OAMBIENT;
  }
  return -1;
}

static int translate_chunk_key (int type)
{
/*
  translate_chunk_key: translate clax keyframer type to chunk.
*/
  switch (type) {
    case clax_track_camera:    return CHUNK_TCAMERA;
    case clax_track_cameratgt: return CHUNK_TCAMTGT;
    case clax_track_light:     return CHUNK_TLIGHT;
    case clax_track_spotlight: return CHUNK_TSPOTL;
    case clax_track_lighttgt:  return CHUNK_TLIGHTTGT;
    case clax_track_object:    return CHUNK_TOBJECT;
    case clax_track_ambient:   return CHUNK_TAMBIENT;
  }
  return -1;
}

/*****************************************************************************
  chunk readers/writers (world)
*****************************************************************************/

static int read_ASCIIZ (FILE *f)
{
/*
  read_ASCIIZ: ASCIIZ string reader.
*/
  char *s = c_string;
  int   c;

  while ((c = fgetc (f)) != EOF && c != '\0') *s++ = (char)c;
  if (c == EOF) return clax_err_badfile;
  *s = '\0';
  return clax_err_ok;
}

static int read_NULL (FILE *f)
{
/*
  read_NULL: "dummy" chunk reader.
*/
  if (f) {} /* to skip the warning */
  return clax_err_ok;
}

static int read_VECTOR (FILE *f, c_VECTOR *v)
{
/*
  read_VECTOR: Vector reader.
*/
  if (fread (&v->x, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&v->y, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&v->z, sizeof (float), 1, f) != 1) return clax_err_badfile;
  return clax_err_ok;
}

static int read_QUAT (FILE *f, c_QUAT *q)
{
/*
  read_QUAT: Quaternion reader.
*/
  if (fread (&q->w, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&q->x, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&q->y, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&q->z, sizeof (float), 1, f) != 1) return clax_err_badfile;
  return clax_err_ok;
}

static int read_MAP (FILE *f, c_MAP *m)
{
/*
  read_MAP: Material MAP reader.
*/
  if (read_ASCIIZ (f)) return clax_err_badfile;
  m->file = strcopy (c_string);
  if (fread (&m->flags, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&m->U_scale, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&m->V_scale, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&m->U_offset, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&m->V_offset, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&m->rot_angle, sizeof (float), 1, f) != 1) return clax_err_badfile;
  return clax_err_ok;
}

static int read_OCAMERA (FILE *f)
{
/*
  read_OCAMERA: Camera reader.
*/
  c_CAMERA *cam;

  if ((cam = (c_CAMERA *)malloc (sizeof (c_CAMERA))) == NULL)
    return clax_err_nomem;
  if (read_ASCIIZ (f)) return clax_err_badfile;
  if ((cam->name = strcopy (c_string)) == NULL) return clax_err_nomem;
  if (fread (&cam->id, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&cam->parent1, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&cam->parent2, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (read_VECTOR (f, &cam->pos)) return clax_err_badfile;
  if (read_VECTOR (f, &cam->target)) return clax_err_badfile;
  if (fread (&cam->fov, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&cam->roll, sizeof (float), 1, f) != 1) return clax_err_badfile;
  clax_add_world (clax_obj_camera, cam);
  return clax_err_ok;
}

static int read_OLIGHT (FILE *f)
{
/*
  read_OLIGHT: Light reader.
*/
  c_LIGHT *light;

  if ((light = (c_LIGHT *)malloc (sizeof (c_LIGHT))) == NULL)
    return clax_err_nomem;
  if (read_ASCIIZ (f)) return clax_err_badfile;
  if ((light->name = strcopy (c_string)) == NULL) return clax_err_nomem;
  if (fread (&light->id, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&light->parent1, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&light->parent2, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&light->flags, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (read_VECTOR (f, &light->pos)) return clax_err_badfile;
  if (light->flags == clax_light_spot) {
    if (read_VECTOR (f, &light->target)) return clax_err_badfile;
    if (fread (&light->roll, sizeof (float), 1, f) != 1) return clax_err_badfile;
    if (fread (&light->hotspot, sizeof (float), 1, f) != 1) return clax_err_badfile;
    if (fread (&light->falloff, sizeof (float), 1, f) != 1) return clax_err_badfile;
  }
  if (read_VECTOR (f, (c_VECTOR *)&light->color)) return clax_err_badfile;
  clax_add_world (clax_obj_light, light);
  return clax_err_ok;
}

static int read_OOBJECT (FILE *f)
{
/*
  read_OOBJECT: Object reader.
*/
  c_OBJECT   *obj;
  w_NODE     *node;
  int16       buffer[5];
  int         i;

  if ((obj = (c_OBJECT *)malloc (sizeof (c_OBJECT))) == NULL)
    return clax_err_nomem;
  if (read_ASCIIZ (f)) return clax_err_badfile;
  if ((obj->name = strcopy (c_string)) == NULL) return clax_err_nomem;
  if (fread (&obj->id, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&obj->parent, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&obj->numverts, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&obj->numfaces, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&obj->flags, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (read_VECTOR (f, &obj->pivot)) return clax_err_badfile;
  if ((obj->flags & clax_obj_dummy) == 0) {
    if ((obj->vertices = (c_VERTEX *)malloc
        (obj->numverts * sizeof (c_VERTEX))) == NULL) return clax_err_nomem;
    if ((obj->faces = (c_FACE *)malloc
        (obj->numfaces * sizeof (c_FACE))) == NULL) return clax_err_nomem;
    for (i = 0; i < obj->numverts; i++) {
      if (read_VECTOR (f, &obj->vertices[i].vert)) return clax_err_badfile;
      if (fread (&obj->vertices[i].u, sizeof (float), 1, f) != 1) return clax_err_badfile;
      if (fread (&obj->vertices[i].v, sizeof (float), 1, f) != 1) return clax_err_badfile;
    }
    for (i = 0; i < obj->numfaces; i++) {
      if (fread (buffer, sizeof (buffer), 1, f) != 1) return clax_err_badfile;
      clax_byid (buffer[4], &node);
      obj->faces[i].a = buffer[0];
      obj->faces[i].b = buffer[1];
      obj->faces[i].c = buffer[2];
      obj->faces[i].flags = buffer[3];
      obj->faces[i].mat = buffer[4];
      obj->faces[i].pa = &obj->vertices[buffer[0]];
      obj->faces[i].pb = &obj->vertices[buffer[1]];
      obj->faces[i].pc = &obj->vertices[buffer[2]];
      obj->faces[i].pmat = (c_MATERIAL *)node->object;
    }
  } else {
    obj->vertices = NULL;
    obj->faces = NULL;
  }
  vec_zero (&obj->translate);
  vec_zero (&obj->scale);
  qt_zero (&obj->rotate);
  mat_zero (obj->matrix);
  clax_add_world (clax_obj_object, obj);
  return clax_err_ok;
}

static int read_OMATERIAL (FILE *f)
{
/*
  read_OMATERIAL: Material reader.
*/
  c_MATERIAL *mat;

  if ((mat = (c_MATERIAL *)malloc (sizeof (c_MATERIAL))) == NULL)
    return clax_err_nomem;
  read_ASCIIZ (f);
  if ((mat->name = strcopy (c_string)) == NULL) return clax_err_nomem;
  if (fread (&mat->id, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&mat->shading, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&mat->flags, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (read_VECTOR (f, (c_VECTOR *)&mat->ambient)) return clax_err_badfile;
  if (read_VECTOR (f, (c_VECTOR *)&mat->diffuse)) return clax_err_badfile;
  if (read_VECTOR (f, (c_VECTOR *)&mat->specular)) return clax_err_badfile;
  if (fread (&mat->shininess, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&mat->shin_strength, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&mat->transparency, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&mat->trans_falloff, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&mat->refblur, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&mat->self_illum, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (read_MAP (f, &mat->texture)) return clax_err_badfile;
  if (read_MAP (f, &mat->bump)) return clax_err_badfile;
  if (read_MAP (f, &mat->reflection)) return clax_err_badfile;
  clax_add_world (clax_obj_material, mat);
  return clax_err_ok;
}

static int read_OAMBIENT (FILE *f)
{
/*
  read_OAMBIENT: Ambient reader.
*/
  c_AMBIENT *amb;

  if ((amb = (c_AMBIENT *)malloc (sizeof (c_AMBIENT))) == NULL)
    return clax_err_nomem;
  if (read_ASCIIZ (f)) return clax_err_badfile;
  if ((amb->name = strcopy (c_string)) == NULL) return clax_err_nomem;
  if (fread (&amb->id, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (read_VECTOR (f, (c_VECTOR *)&amb->color)) return clax_err_badfile;
  clax_add_world (clax_obj_ambient, amb);
  return clax_err_ok;
}

static void write_ASCIIZ (FILE *f, char *s)
{
/*
  write_ASCIIZ: Asciiz writer.
*/
  while (*s) {
    fwrite (s, sizeof (char), 1, f);
    s++;
  }
  fwrite (s, sizeof (char), 1, f);
}

static void write_VECTOR (FILE *f, c_VECTOR *v)
{
/*
  write_VECTOR: Vector writer.
*/
  fwrite (&v->x, sizeof (float), 1, f);
  fwrite (&v->y, sizeof (float), 1, f);
  fwrite (&v->z, sizeof (float), 1, f);
}

static void write_QUAT (FILE *f, c_QUAT *q)
{
/*
  write_QUAT: Quaternion writer.
*/
  fwrite (&q->w, sizeof (float), 1, f);
  fwrite (&q->x, sizeof (float), 1, f);
  fwrite (&q->y, sizeof (float), 1, f);
  fwrite (&q->z, sizeof (float), 1, f);
}

static void write_MAP (FILE *f, c_MAP *m)
{
/*
  write_MAP: Material MAP writer.
*/
  write_ASCIIZ (f, m->file);
  fwrite (&m->flags, sizeof (int), 1, f);
  fwrite (&m->U_scale, sizeof (float), 1, f);
  fwrite (&m->V_scale, sizeof (float), 1, f);
  fwrite (&m->U_offset, sizeof (float), 1, f);
  fwrite (&m->V_offset, sizeof (float), 1, f);
  fwrite (&m->rot_angle, sizeof (float), 1, f);
}

static void write_OCAMERA (FILE *f, w_NODE *n)
{
/*
  write_OCAMERA: Camera writer.
*/
  c_CAMERA *cam = (c_CAMERA *)n->object;

  write_ASCIIZ (f, cam->name);
  fwrite (&cam->id, sizeof (int), 1, f);
  fwrite (&cam->parent1, sizeof (int), 1, f);
  fwrite (&cam->parent2, sizeof (int), 1, f);
  write_VECTOR (f, &cam->pos);
  write_VECTOR (f, &cam->target);
  fwrite (&cam->fov, sizeof (float), 1, f);
  fwrite (&cam->roll, sizeof (float), 1, f);
}

static void write_OLIGHT (FILE *f, w_NODE *n)
{
/*
  write_OLIGHT: Light writer.
*/
  c_LIGHT *light = (c_LIGHT *)n->object;

  write_ASCIIZ (f, light->name);
  fwrite (&light->id, sizeof (int), 1, f);
  fwrite (&light->parent1, sizeof (int), 1, f);
  fwrite (&light->parent2, sizeof (int), 1, f);
  fwrite (&light->flags, sizeof (int), 1, f);
  write_VECTOR (f, &light->pos);
  if (light->flags == clax_light_spot) {
    write_VECTOR (f, &light->target);
    fwrite (&light->roll, sizeof (float), 1, f);
    fwrite (&light->hotspot, sizeof (float), 1, f);
    fwrite (&light->falloff, sizeof (float), 1, f);
  }
  write_VECTOR (f, (c_VECTOR *)&light->color);
}

static void write_OOBJECT (FILE *f, w_NODE *n)
{
/*
  write_OOBJECT: Object writer.
*/
  c_OBJECT *obj = (c_OBJECT *)n->object;
  int       flags = (obj->flags & ~(clax_obj_hidden|clax_obj_morph));
  int       buffer[5];
  int       i;

  write_ASCIIZ (f, obj->name);
  fwrite (&obj->id, sizeof (int), 1, f);
  fwrite (&obj->parent, sizeof (int), 1, f);
  fwrite (&obj->numverts, sizeof (int), 1, f);
  fwrite (&obj->numfaces, sizeof (int), 1, f);
  fwrite (&flags, sizeof (int), 1, f);
  write_VECTOR (f, &obj->pivot);
  if ((obj->flags & clax_obj_dummy) == 0) {
    for (i = 0; i < obj->numverts; i++) {
      write_VECTOR (f, &obj->vertices[i].vert);
      fwrite (&obj->vertices[i].u, sizeof (float), 1, f);
      fwrite (&obj->vertices[i].v, sizeof (float), 1, f);
    }
    for (i = 0; i < obj->numfaces; i++) {
      buffer[0] = obj->faces[i].a;
      buffer[1] = obj->faces[i].b;
      buffer[2] = obj->faces[i].c;
      buffer[3] = obj->faces[i].flags;
      buffer[4] = obj->faces[i].mat;
      fwrite (buffer, sizeof (buffer), 1, f);
    }
  }
}

static void write_OMATERIAL (FILE *f, w_NODE *n)
{
/*
  write_OMATERIAL: Material writer.
*/
  c_MATERIAL *mat = (c_MATERIAL *)n->object;

  write_ASCIIZ (f, mat->name);
  fwrite (&mat->id, sizeof (int), 1, f);
  fwrite (&mat->shading, sizeof (int), 1, f);
  fwrite (&mat->flags, sizeof (int), 1, f);
  write_VECTOR (f, (c_VECTOR *)&mat->ambient);
  write_VECTOR (f, (c_VECTOR *)&mat->diffuse);
  write_VECTOR (f, (c_VECTOR *)&mat->specular);
  fwrite (&mat->shininess, sizeof (float), 1, f);
  fwrite (&mat->shin_strength, sizeof (float), 1, f);
  fwrite (&mat->transparency, sizeof (float), 1, f);
  fwrite (&mat->trans_falloff, sizeof (float), 1, f);
  fwrite (&mat->refblur, sizeof (float), 1, f);
  fwrite (&mat->self_illum, sizeof (float), 1, f);
  write_MAP (f, &mat->texture);
  write_MAP (f, &mat->bump);
  write_MAP (f, &mat->reflection);
}

static void write_OAMBIENT (FILE *f, w_NODE *n)
{
/*
  write_OAMBIENT: Ambient writer.
*/
  c_AMBIENT *amb = (c_AMBIENT *)n->object;

  write_ASCIIZ (f, amb->name);
  fwrite (&amb->id, sizeof (int), 1, f);
  write_VECTOR (f, (c_VECTOR *)&amb->color);
}

/*****************************************************************************
  chunk readers/writers (keyframer)
*****************************************************************************/

static int read_SPLINE (FILE *f, t_KEY *key)
{
/*
  read_SPLINE: Spline flags reader.
*/
  if (fread (&key->tens, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&key->bias, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&key->cont, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&key->easeto, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&key->easefrom, sizeof (float), 1, f) != 1) return clax_err_badfile;
  return clax_err_ok;
}

static int read_TRACKINFO (FILE *f, void *track, int type)
{
/*
  read_TRACKINFO: Track info reader.
*/
  w_NODE *node;
  int     id, oid, parent;

  if (fread (&id, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&parent, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (fread (&oid, sizeof (int), 1, f) != 1) return clax_err_badfile;
  clax_byid (oid, &node);
  clax_add_track (type, id, parent, track, node->object);
  return clax_err_ok;
}

static int read_TRACK (FILE *f, t_TRACK **track, int type)
{
/*
  read_TRACK: Track reader.
*/
  t_TRACK *t;
  t_KEY   *key;
  float    frame;
  int      n;

  if (fread (&n, sizeof (int), 1, f) != 1) return clax_err_badfile;
  if (n == 0) {
    *track = NULL;
    return clax_err_ok;
  }
  t = alloc_track ();
  if (fread (&t->flags, sizeof (int), 1, f) != 1) return clax_err_badfile;
  while (n--) {
    if ((key = (t_KEY *)malloc (sizeof (t_KEY))) == NULL) return clax_err_nomem;
    if (fread (&frame, sizeof (float), 1, f) != 1) return clax_err_badfile;
    switch (type) {
      case track_float:
        if (fread (&key->val._float, sizeof (float), 1, f) != 1) return clax_err_badfile;
        read_SPLINE (f, key);
        break;
      case track_vector:
        if (read_VECTOR (f, &key->val._vect)) return clax_err_badfile;
        read_SPLINE (f, key);
        break;
      case track_quat:
        if (read_QUAT (f, &key->val._quat)) return clax_err_badfile;
        read_SPLINE (f, key);
        break;
      case track_int: ;
        if (fread (&key->val._int, sizeof (int), 1, f) != 1) return clax_err_badfile;
    }
    add_key (t, key, frame);
  }
  if (type == track_float  ||
      type == track_vector ||
      type == track_quat) spline_init (t);
  *track = t;
  return clax_err_ok;
}

static int read_KEYFRAMER (FILE *f)
{
/*
  read_KEYFRAMER: Keyframer info reader.
*/
  if (fread (&clax_scene->f_start, sizeof (float), 1, f) != 1) return clax_err_badfile;
  if (fread (&clax_scene->f_end, sizeof (float), 1, f) != 1) return clax_err_badfile;
  return clax_err_ok;
}

static int read_TCAMERA (FILE *f)
{
/*
  read_TCAMERA: Camera track reader.
*/
  t_CAMERA *cam;

  if ((cam = (t_CAMERA *)malloc (sizeof (t_CAMERA))) == NULL) return clax_err_nomem;
  read_TRACKINFO (f, cam, clax_track_camera);
  read_TRACK (f, &cam->pos, track_vector);
  read_TRACK (f, &cam->fov, track_float);
  read_TRACK (f, &cam->roll, track_float);
  return clax_err_ok;
}

static int read_TCAMTGT (FILE *f)
{
/*
  read_TCAMTGT: Camera target reader.
*/
  t_CAMERATGT *cam;

  if ((cam = (t_CAMERATGT *)malloc (sizeof (t_CAMERATGT))) == NULL) return clax_err_nomem;
  read_TRACKINFO (f, cam, clax_track_cameratgt);
  read_TRACK (f, &cam->pos, track_vector);
  return clax_err_ok;
}

static int read_TLIGHT (FILE *f)
{
/*
  read_TLIGHT: Light track reader.
*/
  t_LIGHT *light;

  if ((light = (t_LIGHT *)malloc (sizeof (t_LIGHT))) == NULL) return clax_err_nomem;
  read_TRACKINFO (f, light, clax_track_light);
  read_TRACK (f, &light->pos, track_vector);
  read_TRACK (f, &light->color, track_vector);
  return clax_err_ok;
}

static int read_TSPOTL (FILE *f)
{
/*
  read_TSPOTL: Spotlight track reader.
*/
  t_SPOTLIGHT *light;

  if ((light = (t_SPOTLIGHT *)malloc (sizeof (t_SPOTLIGHT))) == NULL) return clax_err_nomem;
  read_TRACKINFO (f, light, clax_track_spotlight);
  read_TRACK (f, &light->pos, track_vector);
  read_TRACK (f, &light->color, track_vector);
  read_TRACK (f, &light->roll, track_float);
  return clax_err_ok;
}

static int read_TLIGHTTGT (FILE *f)
{
/*
  read_TLIGHTTGT: Light target track reader.
*/
  t_LIGHTTGT *light;

  if ((light = (t_LIGHTTGT *)malloc (sizeof (t_LIGHTTGT))) == NULL) return clax_err_nomem;
  read_TRACKINFO (f, light, clax_track_lighttgt);
  read_TRACK (f, &light->pos, track_vector);
  return clax_err_ok;
}

static int read_TOBJECT (FILE *f)
{
/*
  read_TOBJECT: Object track reader.
*/
  t_OBJECT *obj;

  if ((obj = (t_OBJECT *)malloc (sizeof (t_OBJECT))) == NULL) return clax_err_nomem;
  read_TRACKINFO (f, obj, clax_track_object);
  read_TRACK (f, &obj->translate, track_vector);
  read_TRACK (f, &obj->scale, track_vector);
  read_TRACK (f, &obj->rotate, track_quat);
  read_TRACK (f, &obj->morph, track_int);
  read_TRACK (f, &obj->hide, track_int);
  return clax_err_ok;
}

static int read_TAMBIENT (FILE *f)
{
/*
  read_TAMBIENT: Ambient track reader.
*/
  t_AMBIENT *amb;

  if ((amb = (t_AMBIENT *)malloc (sizeof (t_AMBIENT))) == NULL) return clax_err_nomem;
  read_TRACKINFO (f, amb, clax_track_ambient);
  read_TRACK (f, &amb->color, track_vector);
  return clax_err_ok;
}

static void write_SPLINE (FILE *f, t_KEY *key)
{
/*
  write_SPLINE: Spline flags writer.
*/
  fwrite (&key->tens, sizeof (float), 1, f);
  fwrite (&key->bias, sizeof (float), 1, f);
  fwrite (&key->cont, sizeof (float), 1, f);
  fwrite (&key->easeto, sizeof (float), 1, f);
  fwrite (&key->easefrom, sizeof (float), 1, f);
}

static void write_TRACKINFO (FILE *f, k_NODE *n)
{
/*
  write_TRACKINFO: Track info writer.
*/
  c_OBJECT *obj = (c_OBJECT *)n->object;
  int       noparent = -1;

  fwrite (&n->id, sizeof (int), 1, f);
  if (n->parent) fwrite (&n->parent->id, sizeof (int), 1, f);
    else fwrite (&noparent, sizeof (int), 1, f);
  fwrite (&obj->id, sizeof (int), 1, f);
}

static void write_TRACK (FILE *f, int type, t_TRACK *track)
{
/*
  write_TRACK: Track writer.
*/
  int    zero = 0;
  t_KEY *keys;

  if (!track || !track->keys) { /* NULL track */
    fwrite (&zero, sizeof (int), 1, f);
    return;
  }
  fwrite (&track->numkeys, sizeof (int), 1, f);
  fwrite (&track->flags, sizeof (int), 1, f);
  for (keys = track->keys; keys; keys = keys->next) {
    fwrite (&keys->frame, sizeof (float), 1, f);
    switch (type) {
      case track_float:
        fwrite (&keys->val._float, sizeof (float), 1, f);
        write_SPLINE (f, keys);
        break;
      case track_vector:
        write_VECTOR (f, &keys->val._vect);
        write_SPLINE (f, keys);
        break;
      case track_quat:
        write_QUAT (f, &keys->val._quat);
        write_SPLINE (f, keys);
        break;
      case track_int: ;
        fwrite (&keys->val._int, sizeof (int), 1, f);
    }
  }
}

static void write_KEYFRAMER (FILE *f)
{
/*
  write_KEYFRAMER: Keyframer info writer.
*/
  fwrite (&clax_scene->f_start, sizeof (float), 1, f);
  fwrite (&clax_scene->f_end, sizeof (float), 1, f);
}

static void write_TCAMERA (FILE *f, k_NODE *n)
{
/*
  write_TCAMERA: Camera track writer.
*/
  t_CAMERA *cam = (t_CAMERA *)n->track;

  write_TRACKINFO (f, n);
  write_TRACK (f, track_vector, cam->pos);
  write_TRACK (f, track_float, cam->fov);
  write_TRACK (f, track_float, cam->roll);
}

static void write_TCAMTGT (FILE *f, k_NODE *n)
{
/*
  write_TCAMTGT: Camera target track writer.
*/
  t_CAMERATGT *cam = (t_CAMERATGT *)n->track;

  write_TRACKINFO (f, n);
  write_TRACK (f, track_vector, cam->pos);
}

static void write_TLIGHT (FILE *f, k_NODE *n)
{
/*
  write_TLIGHT: Light track writer.
*/
  t_LIGHT *light = (t_LIGHT *)n->track;

  write_TRACKINFO (f, n);
  write_TRACK (f, track_vector, light->pos);
  write_TRACK (f, track_vector, light->color);
}

static void write_TSPOTL (FILE *f, k_NODE *n)
{
/*
  write_TSPOTL: Spotlight track writer.
*/
  t_SPOTLIGHT *light = (t_SPOTLIGHT *)n->track;

  write_TRACKINFO (f, n);
  write_TRACK (f, track_vector, light->pos);
  write_TRACK (f, track_vector, light->color);
  write_TRACK (f, track_float, light->roll);
}

static void write_TLIGHTTGT (FILE *f, k_NODE *n)
{
/*
  write_TLIGHTTGT: Light target track writer.
*/
  t_LIGHTTGT *light = (t_LIGHTTGT *)n->track;

  write_TRACKINFO (f, n);
  write_TRACK (f, track_vector, light->pos);
}

static void write_TOBJECT (FILE *f, k_NODE *n)
{
/*
  write_TOBJECT: Object track writer.
*/
  t_OBJECT *obj = (t_OBJECT *)n->track;

  write_TRACKINFO (f, n);
  write_TRACK (f, track_vector, obj->translate);
  write_TRACK (f, track_vector, obj->scale);
  write_TRACK (f, track_quat, obj->rotate);
  write_TRACK (f, track_int, obj->morph);
  write_TRACK (f, track_int, obj->hide);
}

static void write_TAMBIENT (FILE *f, k_NODE *n)
{
/*
  write_TAMBIENT: Ambient track writer.
*/
  t_AMBIENT *amb = (t_AMBIENT *)n->track;

  write_TRACKINFO (f, n);
  write_TRACK (f, track_vector, amb->color);
}

/*****************************************************************************
  chunk readers control
*****************************************************************************/

static int ChunkReaderWorld (FILE *f, int32 size)
{
/*
  ChunkReaderWorld: Chunk reader (world).
*/
  c_CHUNK chunk;
  int     i, n;

  while (ftell (f) < size) {
    if (fread (&chunk.chunk_id, sizeof (int16), 1, f) != 1) return clax_err_badfile;
    if (fread (&chunk.chunk_size, sizeof (int32), 1, f) != 1) return clax_err_badfile;
    n = -1;
    for (i = 0; i < sizeof (world_chunks) / sizeof (world_chunks[0]); i++)
      if (chunk.chunk_id == world_chunks[i].id) {
        n = i;
        break;
      }
    if (n != -1)
      world_chunks[n].func_read (f);
        else fseek (f, chunk.chunk_size, SEEK_CUR);
  }
  return clax_err_ok;
}

static int ChunkReaderKey (FILE *f, int32 size)
{
/*
  ChunkReaderKey: Chunk reader (keyframer).
*/
  c_CHUNK chunk;
  int     i, n;

  while (ftell (f) < size) {
    if (fread (&chunk.chunk_id, sizeof (int16), 1, f) != 1) return clax_err_badfile;
    if (fread (&chunk.chunk_size, sizeof (int32), 1, f) != 1) return clax_err_badfile;
    n = -1;
    for (i = 0; i < sizeof (key_chunks) / sizeof (key_chunks[0]); i++)
      if (chunk.chunk_id == key_chunks[i].id) {
        n = i;
        break;
      }
    if (n != -1)
      key_chunks[n].func_read (f);
        else fseek (f, chunk.chunk_size, SEEK_CUR);
  }
  return clax_err_ok;
}

static void ChunkWriterWorld (FILE *f)
{
/*
  ChunkWriterWorld: Chunk writer (world).
*/
  c_CHUNK chunk;
  w_NODE  *node;
  long     oldpos, curpos, size;
  int      i, n;

  for (node = clax_scene->world; node; node = node->next) {
    chunk.chunk_id = translate_chunk_world (node->type);
    chunk.chunk_size = 0;
    fwrite (&chunk.chunk_id, sizeof (int16), 1, f);
    fwrite (&chunk.chunk_size, sizeof (int32), 1, f);
    oldpos = ftell (f);
    for (i = 0; i < sizeof (world_chunks) / sizeof (world_chunks[0]); i++)
      if (chunk.chunk_id == world_chunks[i].id) {
        n = i;
        break;
      }
    world_chunks[n].func_write (f, node);
    curpos = ftell (f);
    fseek (f, oldpos - 4, SEEK_SET);
    size = curpos - oldpos;
    fwrite (&size, sizeof (int32), 1, f);
    fseek (f, curpos, SEEK_SET);
  }
}

static void ChunkWriterKey (FILE *f)
{
/*
  ChunkWriterKey: Chunk writer (keyframer).
*/
  c_CHUNK chunk;
  k_NODE  *node;
  long     oldpos, curpos, size;
  int      i, n;

  chunk.chunk_id = CHUNK_KEYFRAMER;
  chunk.chunk_size = 0;
  fwrite (&chunk.chunk_id, sizeof (int16), 1, f);
  fwrite (&chunk.chunk_size, sizeof (int32), 1, f);
  oldpos = ftell (f);
  write_KEYFRAMER (f);
  curpos = ftell (f);
  fseek (f, oldpos - 4, SEEK_SET);
  size = curpos - oldpos;
  fwrite (&size, sizeof (int32), 1, f);
  fseek (f, curpos, SEEK_SET);
  for (node = clax_scene->keyframer; node; node = node->next) {
    chunk.chunk_id = translate_chunk_key (node->type);
    chunk.chunk_size = 0;
    fwrite (&chunk.chunk_id, sizeof (int16), 1, f);
    fwrite (&chunk.chunk_size, sizeof (int32), 1, f);
    oldpos = ftell (f);
    for (i = 0; i < sizeof (key_chunks) / sizeof (key_chunks[0]); i++)
      if (chunk.chunk_id == key_chunks[i].id) {
        n = i;
        break;
      }
    key_chunks[n].func_write (f, node);
    curpos = ftell (f);
    fseek (f, oldpos - 4, SEEK_SET);
    size = curpos - oldpos;
    fwrite (&size, sizeof (int32), 1, f);
    fseek (f, curpos, SEEK_SET);
  }
}

/*****************************************************************************
  world/motion load routines
*****************************************************************************/

int32 clax_load_mesh_CLX (FILE *f)
{
/*
  clax_load_mesh_CLX: loads mesh data from clx file "filename"
                      into scene "scene".
*/
  c_HEADER header;
  c_CHUNK  chunk;
  int32    size;

  fseek (f, 0, SEEK_END);
  size = ftell (f);
  fseek (f, 0, SEEK_SET);
  if (fread (&header, sizeof (header), 1, f) != 1) return clax_err_badfile;
  return ChunkReaderWorld (f, size);
}

int32 clax_load_motion_CLX (FILE *f)
{
/*
  clax_load_motion_CLX: loads motion data from clx file "f"
                        into scene "scene".
*/
  c_HEADER header;
  c_CHUNK  chunk;
  int32    size;

  fseek (f, 0, SEEK_END);
  size = ftell (f);
  fseek (f, 0, SEEK_SET);
  if (fread (&header, sizeof (header), 1, f) != 1) return clax_err_badfile;
  return ChunkReaderKey (f, size);
}

/*****************************************************************************
  world/motion save routines
*****************************************************************************/

int32 clax_save_CLX (FILE *f)
{
/*
  clax_save_CLX: write scene data to clx file "f".
*/
  c_HEADER header;
  c_CHUNK  chunk;

  if (!clax_scene) return clax_err_undefined;
  if (!clax_scene->world) return clax_err_notloaded;
  header.magic = FORMAT_MAGIC;
  header.version = FORMAT_VERSION;
  header.flags = flag_world;
  if (clax_scene->keyframer) header.flags |= flag_motion;
  fwrite (&header, sizeof (header), 1, f);
  ChunkWriterWorld (f);
  if (clax_scene->keyframer) ChunkWriterKey (f);
  return clax_err_ok;
}
