/*FILE_3DS.C******************************************************************

   Clax: Portable keyframing library
         see inclosed LICENSE.TXT for licensing terms.

         for documentation, refer to CLAX.TXT

   author           : Borzom
   file created     : 04/05/97
   file description : 3D studio 4.0 driver (world + motion)

   revision history :
     ----- (xx/xx/96) Jare:   Initial version (3dsrdr.c).
     v0.10 (04/05/97) Borzom: Rewritten to fit clax structure.
           (10/05/97) Borzom: Added version check.
           (11/05/97) Borzom: Added color track and dummy name chunks.
           (22/05/97) Borzom: Added more material chunks.

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

enum clax_3ds_chunks_ { /* Chunk ID */
  CHUNK_RGBF         = 0x0010, CHUNK_RGBB         = 0x0011,
  CHUNK_PRJ          = 0xC23D, CHUNK_MLI          = 0x3DAA,
  CHUNK_MAIN         = 0x4D4D, CHUNK_OBJMESH      = 0x3D3D,
  CHUNK_BKGCOLOR     = 0x1200, CHUNK_AMBCOLOR     = 0x2100,
  CHUNK_OBJBLOCK     = 0x4000, CHUNK_TRIMESH      = 0x4100,
  CHUNK_VERTLIST     = 0x4110, CHUNK_VERTFLAGS    = 0x4111,
  CHUNK_FACELIST     = 0x4120, CHUNK_FACEMAT      = 0x4130,
  CHUNK_MAPLIST      = 0x4140, CHUNK_SMOOLIST     = 0x4150,
  CHUNK_TRMATRIX     = 0x4160, CHUNK_MESHCOLOR    = 0x4165,
  CHUNK_TXTINFO      = 0x4170, CHUNK_LIGHT        = 0x4600,
  CHUNK_SPOTLIGHT    = 0x4610, CHUNK_CAMERA       = 0x4700,
  CHUNK_HIERARCHY    = 0x4F00, CHUNK_VIEWPORT     = 0x7001,
  CHUNK_MATERIAL     = 0xAFFF, CHUNK_MATNAME      = 0xA000,
  CHUNK_AMBIENT      = 0xA010, CHUNK_DIFFUSE      = 0xA020,
  CHUNK_SPECULAR     = 0xA030, CHUNK_TEXTURE      = 0xA200,
  CHUNK_BUMPMAP      = 0xA230, CHUNK_REFLECTION   = 0xA220,
  CHUNK_MAPFILE      = 0xA300, CHUNK_MAPFLAGS     = 0xA351,
  CHUNK_MAPUSCALE    = 0xA354, CHUNK_MAPVSCALE    = 0xA356,
  CHUNK_MAPUOFFSET   = 0xA358, CHUNK_MAPVOFFSET   = 0xA35A,
  CHUNK_KEYFRAMER    = 0xB000, CHUNK_AMBIENTKEY   = 0xB001,
  CHUNK_TRACKINFO    = 0xB002, CHUNK_TRACKOBJNAME = 0xB010,
  CHUNK_TRACKPIVOT   = 0xB013, CHUNK_TRACKPOS     = 0xB020,
  CHUNK_TRACKROTATE  = 0xB021, CHUNK_TRACKSCALE   = 0xB022,
  CHUNK_TRACKMORPH   = 0xB026, CHUNK_TRACKHIDE    = 0xB029,
  CHUNK_OBJNUMBER    = 0xB030, CHUNK_TRACKCAMERA  = 0xB003,
  CHUNK_TRACKFOV     = 0xB023, CHUNK_TRACKROLL    = 0xB024,
  CHUNK_TRACKCAMTGT  = 0xB004, CHUNK_TRACKLIGHT   = 0xB005,
  CHUNK_TRACKLIGTGT  = 0xB006, CHUNK_TRACKSPOTL   = 0xB007,
  CHUNK_TRACKCOLOR   = 0xB025, CHUNK_FRAMES       = 0xB008,
  CHUNK_DUMMYNAME    = 0xB011, CHUNK_MAPROTANGLE  = 0xA35C,
  CHUNK_SHININESS    = 0xA040, CHUNK_SHINSTRENGTH = 0xA041,
  CHUNK_TRANSPARENCY = 0xA050, CHUNK_TRANSFALLOFF = 0xA052,
  CHUNK_REFBLUR      = 0xA053, CHUNK_SELFILLUM    = 0xA084,
  CHUNK_TWOSIDED     = 0xA081, CHUNK_TRANSADD     = 0xA083,
  CHUNK_WIREON       = 0xA085, CHUNK_SOFTEN       = 0xA08C,
  CHUNK_MATTYPE      = 0xA100, CHUNK_AMOUNTOF     = 0x0030
};

typedef struct { /* 3DS chunk structure */
  word  chunk_id;                /* chunk id (clax_3ds_chunks_) */
  dword chunk_size;              /* chunk length                */
} c_CHUNK;

typedef struct { /* Chunk reader list */
  word id;                       /* chunk id        */
  int  sub;                      /* has subchunks   */
  int  (*func) (FILE *f);        /* reader function */
} c_LISTKEY, c_LISTWORLD;

static int read_NULL         (FILE *f); /* (skip chunk)            */
static int read_RGBF         (FILE *f); /* RGB float               */
static int read_RGBB         (FILE *f); /* RGB byte                */
static int read_AMOUNTOF     (FILE *f); /* Amount of               */
static int read_ASCIIZ       (FILE *f); /* ASCIIZ string           */
static int read_TRIMESH      (FILE *f); /* Triangular mesh         */
static int read_VERTLIST     (FILE *f); /* Vertex list             */
static int read_FACELIST     (FILE *f); /* Face list               */
static int read_FACEMAT      (FILE *f); /* Face material           */
static int read_MAPLIST      (FILE *f); /* Mapping list            */
static int read_TRMATRIX     (FILE *f); /* Transformation matrix   */
static int read_LIGHT        (FILE *f); /* Light                   */
static int read_SPOTLIGHT    (FILE *f); /* Spotlight               */
static int read_CAMERA       (FILE *f); /* Camera                  */
static int read_MATERIAL     (FILE *f); /* Material                */
static int read_MATNAME      (FILE *f); /* Material name           */
static int read_FRAMES       (FILE *f); /* Number of frames        */
static int read_OBJNUMBER    (FILE *f); /* Object number           */
static int read_TRACKOBJNAME (FILE *f); /* Track object name       */
static int read_DUMMYNAME    (FILE *f); /* Dummy object name       */
static int read_TRACKPIVOT   (FILE *f); /* Track pivot point       */
static int read_TRACKPOS     (FILE *f); /* Track position          */
static int read_TRACKCOLOR   (FILE *f); /* Track color             */
static int read_TRACKROT     (FILE *f); /* Track rotation          */
static int read_TRACKSCALE   (FILE *f); /* Track scale             */
static int read_TRACKFOV     (FILE *f); /* Track fov               */
static int read_TRACKROLL    (FILE *f); /* Track roll              */
static int read_TRACKMORPH   (FILE *f); /* Track morph             */
static int read_TRACKHIDE    (FILE *f); /* Track hide              */
static int read_MATTYPE      (FILE *f); /* Material: type          */
static int read_MATTWOSIDED  (FILE *f); /* Material: two sided     */
static int read_MATSOFTEN    (FILE *f); /* Material: soften        */
static int read_MATWIRE      (FILE *f); /* Material: wire          */
static int read_MATTRANSADD  (FILE *f); /* Material: transparency  */
static int read_MAPFLAGS     (FILE *f); /* Map flags               */
static int read_MAPFILE      (FILE *f); /* Map file                */
static int read_MAPUSCALE    (FILE *f); /* Map 1/U scale           */
static int read_MAPVSCALE    (FILE *f); /* Map 1/V scale           */
static int read_MAPUOFFSET   (FILE *f); /* Map U offset            */
static int read_MAPVOFFSET   (FILE *f); /* Map V offset            */
static int read_MAPROTANGLE  (FILE *f); /* Map rotation angle      */

static c_LISTWORLD world_chunks[] = { /* World definition chunks */
  {CHUNK_RGBF,         0, read_RGBF},
  {CHUNK_RGBB,         0, read_RGBB},
  {CHUNK_AMOUNTOF,     0, read_AMOUNTOF},
  {CHUNK_PRJ,          1, read_NULL},
  {CHUNK_MLI,          1, read_NULL},
  {CHUNK_MAIN,         1, read_NULL},
  {CHUNK_OBJMESH,      1, read_NULL},
  {CHUNK_BKGCOLOR,     1, read_NULL},
  {CHUNK_AMBCOLOR,     1, read_NULL},
  {CHUNK_OBJBLOCK,     1, read_ASCIIZ},
  {CHUNK_TRIMESH,      1, read_TRIMESH},
  {CHUNK_VERTLIST,     0, read_VERTLIST},
  {CHUNK_VERTFLAGS,    0, read_NULL},
  {CHUNK_FACELIST,     1, read_FACELIST},
  {CHUNK_MESHCOLOR,    0, read_NULL},
  {CHUNK_FACEMAT,      0, read_FACEMAT},
  {CHUNK_MAPLIST,      0, read_MAPLIST},
  {CHUNK_TXTINFO,      0, read_NULL},
  {CHUNK_SMOOLIST,     0, read_NULL},
  {CHUNK_TRMATRIX,     0, read_TRMATRIX},
  {CHUNK_LIGHT,        1, read_LIGHT},
  {CHUNK_SPOTLIGHT,    0, read_SPOTLIGHT},
  {CHUNK_CAMERA,       0, read_CAMERA},
  {CHUNK_HIERARCHY,    1, read_NULL},
  {CHUNK_VIEWPORT,     0, read_NULL},
  {CHUNK_MATERIAL,     1, read_MATERIAL},
  {CHUNK_MATNAME,      0, read_MATNAME},
  {CHUNK_AMBIENT,      1, read_NULL},
  {CHUNK_DIFFUSE,      1, read_NULL},
  {CHUNK_SPECULAR,     1, read_NULL},
  {CHUNK_TEXTURE,      1, read_NULL},
  {CHUNK_BUMPMAP,      1, read_NULL},
  {CHUNK_REFLECTION,   1, read_NULL},
  {CHUNK_MAPFILE,      0, read_MAPFILE},
  {CHUNK_MAPFLAGS,     0, read_MAPFLAGS},
  {CHUNK_MAPUSCALE,    0, read_MAPUSCALE},
  {CHUNK_MAPVSCALE,    0, read_MAPVSCALE},
  {CHUNK_MAPUOFFSET,   0, read_MAPUOFFSET},
  {CHUNK_MAPVOFFSET,   0, read_MAPVOFFSET},
  {CHUNK_MAPROTANGLE,  0, read_MAPROTANGLE},
  {CHUNK_SHININESS,    1, read_NULL},
  {CHUNK_SHINSTRENGTH, 1, read_NULL},
  {CHUNK_TRANSPARENCY, 1, read_NULL},
  {CHUNK_TRANSFALLOFF, 1, read_NULL},
  {CHUNK_REFBLUR,      1, read_NULL},
  {CHUNK_SELFILLUM,    1, read_NULL},
  {CHUNK_TWOSIDED,     0, read_MATTWOSIDED},
  {CHUNK_TRANSADD,     0, read_MATTRANSADD},
  {CHUNK_WIREON,       0, read_MATWIRE},
  {CHUNK_SOFTEN,       0, read_MATSOFTEN},
  {CHUNK_MATTYPE,      0, read_MATTYPE}
};

static c_LISTKEY key_chunks[] = { /* Keyframer chunks */
  {CHUNK_MAIN,         1, read_NULL},
  {CHUNK_KEYFRAMER,    1, read_NULL},
  {CHUNK_AMBIENTKEY,   1, read_NULL},
  {CHUNK_TRACKINFO,    1, read_NULL},
  {CHUNK_FRAMES,       0, read_FRAMES},
  {CHUNK_TRACKOBJNAME, 0, read_TRACKOBJNAME},
  {CHUNK_DUMMYNAME,    0, read_DUMMYNAME},
  {CHUNK_TRACKPIVOT,   0, read_TRACKPIVOT},
  {CHUNK_TRACKPOS,     0, read_TRACKPOS},
  {CHUNK_TRACKCOLOR,   0, read_TRACKCOLOR},
  {CHUNK_TRACKROTATE,  0, read_TRACKROT},
  {CHUNK_TRACKSCALE,   0, read_TRACKSCALE},
  {CHUNK_TRACKMORPH,   0, read_TRACKMORPH},
  {CHUNK_TRACKHIDE,    0, read_TRACKHIDE},
  {CHUNK_OBJNUMBER,    0, read_OBJNUMBER},
  {CHUNK_TRACKCAMERA,  1, read_NULL},
  {CHUNK_TRACKCAMTGT,  1, read_NULL},
  {CHUNK_TRACKLIGHT,   1, read_NULL},
  {CHUNK_TRACKLIGTGT,  1, read_NULL},
  {CHUNK_TRACKSPOTL,   1, read_NULL},
  {CHUNK_TRACKFOV,     0, read_TRACKFOV},
  {CHUNK_TRACKROLL,    0, read_TRACKROLL}
};

static int   c_chunk_last;       /* parent chunk      */
static int   c_chunk_prev;       /* previous chunk    */
static int   c_chunk_curr;       /* current chunk     */
static int   c_id;               /* current id        */
static char  c_string[64];       /* current name      */
static void *c_node;             /* current node      */

/*****************************************************************************
  internal functions
*****************************************************************************/

static void vec_swap (c_VECTOR *a)
{
/*
  swap_vert: swap y/z in vector.
*/
  float tmp;

#ifdef CLAX_SWAP_YZ
  tmp  = a->y;
  a->y = a->z;
  a->z = tmp;
#endif
}

static void qt_swap (c_QUAT *a)
{
/*
  swap_vert: swap y/z in vector.
*/
  float tmp;

#ifdef CLAX_SWAP_YZ
  tmp  = a->y;
  a->y = a->z;
  a->z = tmp;
#endif
}

static void mat_swap (c_MATRIX a)
{
/*
  swap_mat: swap y/z in matrix.
*/
  int   i;
  float tmp;

#ifdef CLAX_SWAP_YZ
  for (i = 0; i < 3; i++) { /* swap columns */
    tmp = a[i][Y];
    a[i][Y] = a[i][Z];
    a[i][Z] = tmp;
  }
  for (i = 0; i < 4; i++) { /* swap rows */
    tmp = a[Y][i];
    a[Y][i] = a[Z][i];
    a[Z][i] = tmp;
  }
#endif
}

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

static void clear_map (c_MAP *map)
{
/*
  clear_map: reset material map to default;
*/
  map->file = NULL;
  map->flags = 0;
  map->U_scale = 0.0;
  map->V_scale = 0.0;
  map->U_offset = 0.0;
  map->V_offset = 0.0;
  map->rot_angle = 0.0;
}

static void clear_mat (c_MATERIAL *mat)
{
/*
  clear_mat: reset material to default.
*/
  mat->shading = 0;
  mat->flags = 0;
  clear_map (&mat->texture);
  clear_map (&mat->bump);
  clear_map (&mat->reflection);
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

/*****************************************************************************
  chunk readers (world)
*****************************************************************************/

static int read_NULL (FILE *f)
{
/*
  read_NULL: "dummy" chunk reader.
*/
  if (f) {} /* to skip the warning */
  return clax_err_ok;
}

static int read_RGBF (FILE *f)
{
/*
  read_RGBF: RGB float reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;
  c_RGB      *rgb = NULL;
  float       c[3];

  switch (c_chunk_last) {
    case CHUNK_LIGHT:        rgb = &(((c_LIGHT *)c_node)->color); break;
    case CHUNK_AMBIENT:      rgb = &(mat->ambient); break;
    case CHUNK_DIFFUSE:      rgb = &(mat->diffuse); break;
    case CHUNK_SPECULAR:     rgb = &(mat->specular); break;
  }
  if (fread (c, sizeof (c), 1, f) != 1) return clax_err_badfile;
  if (rgb) {
    rgb->r = c[0];
    rgb->g = c[1];
    rgb->b = c[2];
  }
  return clax_err_ok;
}

static int read_RGBB (FILE *f)
{
/*
  read_RGBB: RGB Byte reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;
  c_RGB      *rgb = NULL;
  byte        c[3];

  switch (c_chunk_last) {
    case CHUNK_LIGHT:        rgb = &(((c_LIGHT *)c_node)->color); break;
    case CHUNK_AMBIENT:      rgb = &(mat->ambient); break;
    case CHUNK_DIFFUSE:      rgb = &(mat->diffuse); break;
    case CHUNK_SPECULAR:     rgb = &(mat->specular); break;
  }
  if (fread (c, sizeof (c), 1, f) != 1) return clax_err_badfile;
  if (rgb) {
    rgb->r = (float)c[0] / 255.0;
    rgb->g = (float)c[1] / 255.0;
    rgb->b = (float)c[2] / 255.0;
  }
  return clax_err_ok;
}

static int read_AMOUNTOF (FILE *f)
{
/*
  read_AMOUNTOF: "amount of" reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;
  float      *fl = NULL;
  word        w;

  switch (c_chunk_last) {
    case CHUNK_SHININESS:    fl = &(mat->shininess); break;
    case CHUNK_SHINSTRENGTH: fl = &(mat->shin_strength); break;
    case CHUNK_TRANSPARENCY: fl = &(mat->transparency); break;
    case CHUNK_TRANSFALLOFF: fl = &(mat->trans_falloff); break;
    case CHUNK_REFBLUR:      fl = &(mat->refblur); break;
    case CHUNK_SELFILLUM:    fl = &(mat->self_illum);
  }
  if (fread (&w, sizeof (w), 1, f) != 1) return clax_err_badfile;
  if (fl) *fl = (float)w / 100.0;
  return clax_err_ok;
}

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

static int read_TRIMESH (FILE *f)
{
/*
  read_TRIMESH: Triangular mesh reader.
*/
  c_OBJECT *obj;

  if (f) {} /* to skip the warning */
  if ((obj = (c_OBJECT *)malloc (sizeof (c_OBJECT))) == NULL)
    return clax_err_nomem;
  if ((obj->name = strcopy (c_string)) == NULL) return clax_err_nomem;
  obj->id = c_id++;
  obj->parent = -1;
  obj->flags = 0;
  vec_zero (&obj->pivot);
  vec_zero (&obj->translate);
  vec_zero (&obj->scale);
  qt_zero (&obj->rotate);
  mat_zero (obj->matrix);
  c_node = obj;
  clax_add_world (clax_obj_object, obj);
  return clax_err_ok;
}

static int read_VERTLIST (FILE *f)
{
/*
  read_VERTLIST: Vertex list reader.
*/
  c_OBJECT *obj = (c_OBJECT *)c_node;
  c_VERTEX *v;
  float     c[3];
  word      nv;

  if (fread (&nv, sizeof (nv), 1, f) != 1) return clax_err_badfile;
  if ((v = (c_VERTEX *)malloc (nv * sizeof (c_VERTEX))) == NULL)
    return clax_err_nomem;
  obj->vertices = v;
  obj->numverts = nv;
  while (nv-- > 0) {
    if (fread (c, sizeof (c), 1, f) != 1) return clax_err_badfile;
    vec_make (c[0], c[1], c[2], &v->vert);
    vec_swap (&v->vert);
    v->u = 0.0;
    v->v = 0.0;
    v++;
  }
  return clax_err_ok;
}

static int read_FACELIST (FILE *f)
{
/*
  read_FACELIST: Face list reader.
*/
  c_OBJECT *obj = (c_OBJECT *)c_node;
  c_FACE   *fc;
  word      c[3];
  word      nv, flags;

  if (fread (&nv, sizeof (nv), 1, f) != 1) return clax_err_badfile;
  if ((fc = (c_FACE *)malloc (nv * sizeof (c_FACE))) == NULL)
    return clax_err_nomem;
  obj->faces = fc;
  obj->numfaces = nv;
  while (nv-- > 0) {
    if (fread (c, sizeof (c), 1, f) != 1) return clax_err_badfile;
    if (fread (&flags, sizeof (flags), 1, f) != 1) return clax_err_badfile;
    fc->a = c[0];
    fc->b = c[1];
    fc->c = c[2];
    fc->pa = &obj->vertices[c[0]];
    fc->pb = &obj->vertices[c[1]];
    fc->pc = &obj->vertices[c[2]];
    fc->flags = 0;
    fc->mat = 0;
    if (flags & 0x08) fc->flags |= clax_face_wrapU;
    if (flags & 0x10) fc->flags |= clax_face_wrapV;
    fc++;
  }
  return clax_err_ok;
}

static int read_FACEMAT (FILE *f)
{
/*
  read_FACEMAT: Face material reader.
*/
  c_FACE     *fc = ((c_OBJECT *)c_node)->faces;
  w_NODE     *node;
  c_MATERIAL *mat;
  word        n, nf;

  if (read_ASCIIZ (f)) return clax_err_badfile;
  if (fread (&n, sizeof (n), 1, f) != 1) return clax_err_badfile;
  clax_byname (c_string, &node);
  if (!node) return clax_err_undefined;
  mat = (c_MATERIAL *)node->object;
  while (n-- > 0) {
    if (fread (&nf, sizeof (nf), 1, f) != 1) return clax_err_badfile;
    fc[nf].mat = mat->id;
    fc[nf].pmat = mat;
  }
  return clax_err_ok;
}

static int read_MAPLIST (FILE *f)
{
/*
  read_MAPLIST: Map list reader.
*/
  c_VERTEX *v = ((c_OBJECT *)c_node)->vertices;
  float     c[2];
  word      nv;

  if (fread (&nv, sizeof (nv), 1, f) != 1) return clax_err_badfile;
  while (nv-- > 0) {
    if (fread (c, sizeof (c), 1, f) != 1) return clax_err_badfile;
    v->u = c[0];
    v->v = c[1];
    v++;
  }
  return clax_err_ok;
}

static int read_TRMATRIX (FILE *f)
{
/*
  read_TRMATRIX: Transformation matrix reader.
*/
  c_OBJECT *obj = (c_OBJECT *)c_node;
  c_VERTEX *v = obj->vertices;
  c_VECTOR  piv;
  c_MATRIX  mat;
  float     pivot[3];
  int       i, j;

  mat_identity (mat);
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      if (fread (&mat[i][j], sizeof (float), 1, f) != 1)
        return clax_err_badfile;
  if (fread (pivot, sizeof (pivot), 1, f) != 1) return clax_err_badfile;
  vec_make (pivot[0], pivot[1], pivot[2], &piv);
  vec_swap (&piv);
  mat_swap (mat);
  mat_invscale (mat, mat);
  for (i = 0; i < obj->numverts; i++) {
    vec_sub (&v->vert, &piv, &v->vert);
    mat_mulvec (mat, &v->vert, &v->vert);
    v++;
  }
  return clax_err_ok;
}

static int read_LIGHT (FILE *f)
{
/*
  read_LIGHT: Light reader.
*/
  float   c[3];
  c_LIGHT *light;

  if ((light = (c_LIGHT *)malloc (sizeof (c_LIGHT))) == NULL)
    return clax_err_nomem;
  if (fread (c, sizeof (c), 1, f) != 1) return clax_err_badfile;
  if ((light->name = strcopy (c_string)) == NULL) return clax_err_nomem;
  light->id = c_id++;
  light->flags = clax_light_omni;
  vec_make (c[0], c[1], c[2], &light->pos);
  vec_swap (&light->pos);
  c_node = light;
  clax_add_world (clax_obj_light, light);
  return clax_err_ok;
}

static int read_SPOTLIGHT (FILE *f)
{
/*
  read_SPOTLIGHT: Spot light reader.
*/
  float   c[5];
  c_LIGHT *light = (c_LIGHT *)c_node;

  if (fread (c, sizeof (c), 1, f) != 1) return clax_err_badfile;
  light->target.x = c[0];
  light->target.y = c[1];
  light->target.z = c[2];
  light->hotspot = c[3];
  light->falloff = c[4];
  light->flags = clax_light_spot;
  light->roll = 0.0;
  vec_swap (&light->target);
  return clax_err_ok;
}

static int read_CAMERA (FILE *f)
{
/*
  read_CAMERA: Camera reader.
*/
  float    c[8];
  c_CAMERA *cam;

  if ((cam = (c_CAMERA *)malloc (sizeof (c_CAMERA))) == NULL)
    return clax_err_nomem;
  if (fread (c, sizeof (c), 1, f) != 1) return clax_err_badfile;
  if ((cam->name = strcopy (c_string)) == NULL) return clax_err_nomem;
  cam->id = c_id++;
  cam->roll = c[6];
  cam_lens_fov (c[7], &cam->fov);
  vec_make (c[0], c[1], c[2], &cam->pos);
  vec_make (c[3], c[4], c[5], &cam->target);
  vec_swap (&cam->pos);
  vec_swap (&cam->target);
  c_node = cam;
  clax_add_world (clax_obj_camera, cam);
  return clax_err_ok;
}

static int read_MATERIAL (FILE *f)
{
/*
  read_MATERIAL: Material reader.
*/
  c_MATERIAL *mat;

  if (f) {} /* to skip the warning */
  if ((mat = (c_MATERIAL *)malloc (sizeof (c_MATERIAL))) == NULL)
    return clax_err_nomem;
  clear_mat (mat);
  mat->id = c_id++;
  c_node = mat;
  clax_add_world (clax_obj_material, mat);
  return clax_err_ok;
}

static int read_MATNAME (FILE *f)
{
/*
  read_MATNAME: Material name reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;

  if (read_ASCIIZ (f)) return clax_err_badfile;
  if ((mat->name = strcopy (c_string)) == NULL) return clax_err_nomem;
  return clax_err_ok;
}

static int read_MATTYPE (FILE *f)
{
/*
  read_MATTYPE: Material type reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;
  word        type;

  if (fread (&type, sizeof (type), 1, f) != 1) return clax_err_badfile;
  mat->shading = type;
  return clax_err_ok;
}

static int read_MATTWOSIDED (FILE *f)
{
/*
  read_MATTWOSIDED: Material two sided reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;

  if (f) {} /* to skip the warning */
  mat->flags |= clax_mat_twosided;
  return clax_err_ok;
}

static int read_MATSOFTEN (FILE *f)
{
/*
  read_MATSOFTEN: Material soften reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;

  if (f) {} /* to skip the warning */
  mat->flags |= clax_mat_soften;
  return clax_err_ok;
}

static int read_MATWIRE (FILE *f)
{
/*
  read_MATWIRE: Material wireframe reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;

  if (f) {} /* to skip the warning */
  mat->flags |= clax_mat_wire;
  return clax_err_ok;
}

static int read_MATTRANSADD (FILE *f)
{
/*
  read_MATTRANSADD: Material transparency add reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;

  if (f) {} /* to skip the warning */
  mat->flags |= clax_mat_transadd;
  return clax_err_ok;
}

static int read_MAPFILE (FILE *f)
{
/*
  read_MAPFILE: MAP file reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;
  c_MAP      *map = NULL;

  if (read_ASCIIZ (f)) return clax_err_badfile;
  switch (c_chunk_last) {
    case CHUNK_TEXTURE: map = &(mat->texture); break;
    case CHUNK_BUMPMAP: map = &(mat->bump); break;
    case CHUNK_REFLECTION: map = &(mat->reflection);
  }
  if (map)
    if ((map->file = strcopy (c_string)) == NULL) return clax_err_nomem;
  return clax_err_ok;
}

static int read_MAPFLAGS (FILE *f)
{
/*
  read_MAPFLAGS: MAP flags reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;
  c_MAP      *map = NULL;
  word        flags;

  if (fread (&flags, sizeof (flags), 1, f) != 1) return clax_err_badfile;
  switch (c_chunk_last) {
    case CHUNK_TEXTURE: map = &(mat->texture); break;
    case CHUNK_BUMPMAP: map = &(mat->bump); break;
    case CHUNK_REFLECTION: map = &(mat->reflection);
  }
  if (map) map->flags = flags;
  return clax_err_ok;
}

static int read_MAPUSCALE (FILE *f)
{
/*
  read_MAPUSCALE: MAP U scale reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;
  c_MAP      *map = NULL;
  float       U;

  if (fread (&U, sizeof (U), 1, f) != 1) return clax_err_badfile;
  switch (c_chunk_last) {
    case CHUNK_TEXTURE: map = &(mat->texture); break;
    case CHUNK_BUMPMAP: map = &(mat->bump); break;
    case CHUNK_REFLECTION: map = &(mat->reflection);
  }
  if (map) map->U_scale = U;
  return clax_err_ok;
}

static int read_MAPVSCALE (FILE *f)
{
/*
  read_MAPUSCALE: MAP U scale reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;
  c_MAP      *map = NULL;
  float       V;

  if (fread (&V, sizeof (V), 1, f) != 1) return clax_err_badfile;
  switch (c_chunk_last) {
    case CHUNK_TEXTURE: map = &(mat->texture); break;
    case CHUNK_BUMPMAP: map = &(mat->bump); break;
    case CHUNK_REFLECTION: map = &(mat->reflection);
  }
  if (map) map->V_scale = V;
  return clax_err_ok;
}

static int read_MAPUOFFSET (FILE *f)
{
/*
  read_MAPUSCALE: MAP U offset reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;
  c_MAP      *map = NULL;
  float       U;

  if (fread (&U, sizeof (U), 1, f) != 1) return clax_err_badfile;
  switch (c_chunk_last) {
    case CHUNK_TEXTURE: map = &(mat->texture); break;
    case CHUNK_BUMPMAP: map = &(mat->bump); break;
    case CHUNK_REFLECTION: map = &(mat->reflection);
  }
  if (map) map->U_offset = U;
  return clax_err_ok;
}

static int read_MAPVOFFSET (FILE *f)
{
/*
  read_MAPUSCALE: MAP V offset reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;
  c_MAP      *map = NULL;
  float       V;

  if (fread (&V, sizeof (V), 1, f) != 1) return clax_err_badfile;
  switch (c_chunk_last) {
    case CHUNK_TEXTURE: map = &(mat->texture); break;
    case CHUNK_BUMPMAP: map = &(mat->bump); break;
    case CHUNK_REFLECTION: map = &(mat->reflection);
  }
  if (map) map->V_offset = V;
  return clax_err_ok;
}

static int read_MAPROTANGLE (FILE *f)
{
/*
  read_MAPUSCALE: MAP rotation angle reader.
*/
  c_MATERIAL *mat = (c_MATERIAL *)c_node;
  c_MAP      *map = NULL;
  float       angle;

  if (fread (&angle, sizeof (angle), 1, f) != 1) return clax_err_badfile;
  switch (c_chunk_last) {
    case CHUNK_TEXTURE: map = &(mat->texture); break;
    case CHUNK_BUMPMAP: map = &(mat->bump); break;
    case CHUNK_REFLECTION: map = &(mat->reflection);
  }
  if (map) map->rot_angle = angle;
  return clax_err_ok;
}

/*****************************************************************************
  chunk readers (keyframer)
*****************************************************************************/

static int read_FRAMES (FILE *f)
{
/*
  read_FRAMES: Frames reader.
*/
  dword c[2];

  if (fread (c, sizeof (c), 1, f) != 1) return clax_err_badfile;
  clax_scene->f_start = c[0];
  clax_scene->f_end = c[1];
  return clax_err_ok;
}

static int read_OBJNUMBER (FILE *f)
{
/*
  read_OBJNUMBER: Object number reader. (3DS 4.0+)
*/
  word n;

  if (fread (&n, sizeof (n), 1, f) != 1) return clax_err_badfile;
  c_id = n;
  return clax_err_ok;
}

static int read_DUMMYNAME (FILE *f)
{
/*
  read_DUMMYNAME: Dummy object name reader.
*/
  c_OBJECT *obj = (c_OBJECT *)c_node;

  if (read_ASCIIZ (f)) return clax_err_badfile;
  if ((obj->name = strcopy (c_string)) == NULL) return clax_err_nomem;
  return clax_err_ok;
}

static int read_TRACKOBJNAME (FILE *f)
{
/*
  read_TRACKOBJNAME: Track object name reader.
*/
  w_NODE    *node;
  k_NODE    *pnode;
  c_OBJECT  *obj = NULL; /* to skip the warning */
  c_LIGHT   *light = NULL;
  c_CAMERA  *cam = NULL;
  c_AMBIENT *amb = NULL;
  void      *track;
  word       flags[2];
  sword      parent;
  int        wparent = -1;

  /* for 3DS 3.0 compatibility */
  if (c_chunk_prev != CHUNK_OBJNUMBER) c_id++;

  if (read_ASCIIZ (f)) return clax_err_badfile;
  if (strcmp (c_string, "$AMBIENT$") == 0) {
    if ((amb = (c_AMBIENT *)malloc (sizeof (c_AMBIENT))) == NULL)
      return clax_err_nomem;
    if ((amb->name = strcopy (c_string)) == NULL) return clax_err_nomem;
    amb->id = 1024+c_id;
    vec_zero ((c_VECTOR *)&amb->color);
    clax_add_world (clax_obj_ambient, amb);
  } else if (strcmp (c_string, "$$$DUMMY") == 0) {
    if ((obj = (c_OBJECT *)malloc (sizeof (c_OBJECT))) == NULL)
      return clax_err_nomem;
    obj->id = 1024+c_id;
    obj->flags = clax_obj_dummy;
    obj->numverts = 0;
    obj->numfaces = 0;
    obj->vertices = NULL;
    obj->faces = NULL;
    vec_zero (&obj->translate);
    vec_zero (&obj->scale);
    qt_zero (&obj->rotate);
    clax_add_world (clax_obj_object, obj);
  } else {
    clax_byname (c_string, &node);
    if (!node) return clax_err_undefined;
    obj = (c_OBJECT *)node->object;
    cam = (c_CAMERA *)node->object;
    light = (c_LIGHT *)node->object;
  }
  if (fread (flags, sizeof (flags), 1, f) != 1) return clax_err_badfile;
  if (fread (&parent, sizeof (parent), 1, f) != 1) return clax_err_badfile;
  if (parent != -1) {
    for (pnode = clax_scene->keyframer; pnode; pnode = pnode->next)
      if (pnode->id == parent)
        wparent = ((c_OBJECT *)pnode->object)->id;
  }
  if (c_chunk_last == CHUNK_TRACKINFO) {
    obj->parent = wparent;
    if (flags[0] & 0x800) obj->flags |= clax_obj_chidden;
    if ((track = malloc (sizeof (t_OBJECT))) == NULL) return clax_err_nomem;
    memset (track, 0, sizeof (t_OBJECT));
    clax_add_track (clax_track_object, c_id, parent, track, obj);
    c_node = obj;
  }
  if (c_chunk_last == CHUNK_TRACKCAMERA) {
    cam->parent1 = wparent;
    if ((track = malloc (sizeof (t_CAMERA))) == NULL) return clax_err_nomem;
    memset (track, 0, sizeof (t_CAMERA));
    clax_add_track (clax_track_camera, c_id, parent, track, cam);
  }
  if (c_chunk_last == CHUNK_TRACKCAMTGT) {
    cam->parent2 = wparent;
    if ((track = malloc (sizeof (t_CAMERATGT))) == NULL)
      return clax_err_nomem;
    memset (track, 0, sizeof (t_CAMERATGT));
    clax_add_track (clax_track_cameratgt, c_id, parent, track, cam);
  }
  if (c_chunk_last == CHUNK_TRACKLIGHT) {
    light->parent1 = wparent;
    if ((track = malloc (sizeof (t_LIGHT))) == NULL) return clax_err_nomem;
    memset (track, 0, sizeof (t_LIGHT));
    clax_add_track (clax_track_light, c_id, parent, track, light);
  }
  if (c_chunk_last == CHUNK_TRACKSPOTL) {
    light->parent1 = wparent;
    if ((track = malloc (sizeof (t_SPOTLIGHT))) == NULL)
      return clax_err_nomem;
    memset (track, 0, sizeof (t_SPOTLIGHT));
    clax_add_track (clax_track_spotlight, c_id, parent, track, light);
  }
  if (c_chunk_last == CHUNK_TRACKLIGTGT) {
    light->parent2 = wparent;
    if ((track = malloc (sizeof (t_LIGHTTGT))) == NULL)
      return clax_err_nomem;
    memset (track, 0, sizeof (t_LIGHTTGT));
    clax_add_track (clax_track_lighttgt, c_id, parent, track, light);
  }
  if (c_chunk_last == CHUNK_AMBIENTKEY) {
    if ((track = malloc (sizeof (t_AMBIENT))) == NULL) return clax_err_nomem;
    memset (track, 0, sizeof (t_AMBIENT));
    clax_add_track (clax_track_ambient, c_id, parent, track, amb);
  }
  return clax_err_ok;
}

static int read_TRACKPIVOT (FILE *f)
{
/*
  read_TRACKPIVOT: Track pivot point reader.
*/
  c_OBJECT *obj = (c_OBJECT *)c_node;
  float     pos[3];
  int       i;

  if (fread (pos, sizeof (pos), 1, f) != 1) return clax_err_badfile;
  vec_make (pos[0], pos[1], pos[2], &obj->pivot);
  vec_swap (&obj->pivot);
  for (i = 0; i < obj->numverts; i++)
    vec_sub (&obj->vertices[i].vert, &obj->pivot, &obj->vertices[i].vert);
  return clax_err_ok;
}

static int read_KFLAGS (FILE *f, word *nf, t_KEY *key)
{
/*
  read_KFLAGS: Key flags/spline reader.
*/
  word  unknown, flags;
  int   i;
  float dat;

  key->tens = 0.0;
  key->cont = 0.0;
  key->bias = 0.0;
  key->easeto = 0.0;
  key->easefrom = 0.0;
  if (fread (nf, sizeof (word), 1, f) != 1) return clax_err_badfile;
  if (fread (&unknown, sizeof (word), 1, f) != 1) return clax_err_badfile;
  if (fread (&flags, sizeof (flags), 1, f) != 1) return clax_err_badfile;
  for (i = 0; i < 16; i++) {
    if (flags & (1 << i)) {
      if (fread (&dat, sizeof (dat), 1, f) != 1) return clax_err_badfile;
      switch (i) {
        case 0: key->tens = dat; break;
        case 1: key->cont = dat; break;
        case 2: key->bias = dat; break;
        case 3: key->easeto = dat; break;
        case 4: key->easefrom = dat;
      }
    }
  }
  return clax_err_ok;
}

static int read_TFLAGS (FILE *f, t_TRACK *track, word *n)
{
/*
  read_TFLAGS: Track flags reader.
*/
  word flags[7];

  if (fread (flags, sizeof (flags), 1, f) != 1) return clax_err_badfile;
  if ((flags[0] & 0x02) == 0x02) track->flags = clax_track_repeat;
  if ((flags[0] & 0x03) == 0x03) track->flags = clax_track_loop;
  *n = flags[5];
  return clax_err_ok;
}

static int read_TRACKPOS (FILE *f)
{
/*
  read_TRACKPOS: Track position reader.
*/
  t_TRACK *track;
  t_KEY  *key;
  float  pos[3];
  word   n, nf;

  track = alloc_track();
  if (read_TFLAGS (f, track, &n) != 0) return clax_err_badfile;
  while (n-- > 0) {
    if ((key = (t_KEY *)malloc (sizeof (t_KEY))) == NULL)
      return clax_err_nomem;
    if (read_KFLAGS (f, &nf, key)) return clax_err_badfile;
    if (fread (pos, sizeof (pos), 1, f) != 1) return clax_err_badfile;
    vec_make (pos[0], pos[1], pos[2], &key->val._vect);
    vec_swap (&key->val._vect);
    add_key (track, key, nf);
  }
  spline_init (track);
  clax_set_track (clax_key_pos, c_id, track);
  return clax_err_ok;
}

static int read_TRACKCOLOR (FILE *f)
{
/*
  read_TRACKCOLOR: Track color reader.
*/
  t_TRACK *track;
  t_KEY *key;
  float  pos[3];
  word   n, nf;

  track = alloc_track();
  if (read_TFLAGS (f, track, &n) != 0) return clax_err_badfile;
  while (n-- > 0) {
    if ((key = (t_KEY *)malloc (sizeof (t_KEY))) == NULL)
      return clax_err_nomem;
    if (read_KFLAGS (f, &nf, key)) return clax_err_badfile;
    if (fread (pos, sizeof (pos), 1, f) != 1) return clax_err_badfile;
    vec_make (pos[0], pos[1], pos[2], &key->val._vect);
    vec_swap (&key->val._vect);
    add_key (track, key, nf);
  }
  spline_init (track);
  clax_set_track (clax_key_color, c_id, track);
  return clax_err_ok;
}

static int read_TRACKROT (FILE *f)
{
/*
  read_TRACKROT: Track rotation reader.
*/
  t_TRACK *track;
  t_KEY   *key;
  c_QUAT   q, old;
  float    pos[4];
  word     keys,n, nf;
  int angle;

  track = alloc_track();
  qt_identity (&old);
  if (read_TFLAGS (f, track, &n) != 0) return clax_err_badfile;
  keys = n;
  while (n-- > 0) {
    if ((key = (t_KEY *)malloc (sizeof (t_KEY))) == NULL)
      return clax_err_nomem;
    if (read_KFLAGS (f, &nf, key)) return clax_err_badfile;
    if (fread (pos, sizeof(pos), 1, f) != 1) return clax_err_badfile;
    qt_fromang (pos[0], pos[1], pos[2], pos[3], &q);
	// !!! FIX !!! I SAID ANGLE IS ABSOLUTE!!!!!!!!!
    if (keys == n-1) angle = pos[0]; else angle += pos[0];
    qt_make (angle, pos[1], pos[2], pos[3], &key->val._quat);
    qt_swap (&key->val._quat);
    qt_swap (&q);
    qt_mul (&q, &old, &old);
    qt_copy (&old, &key->qa);
    add_key (track, key, nf);
  }
  spline_initrot (track);
  clax_set_track (clax_key_rotate, c_id, track);
  return clax_err_ok;
}

static int read_TRACKSCALE (FILE *f)
{
/*
  read_TRACKSCALE: Track scale reader.
*/
  t_TRACK *track;
  t_KEY *key;
  float  pos[3];
  word   n, nf;

  track = alloc_track();
  if (read_TFLAGS (f, track, &n) != 0) return clax_err_badfile;
  while (n-- > 0) {
    if ((key = (t_KEY *)malloc (sizeof (t_KEY))) == NULL)
      return clax_err_nomem;
    if (read_KFLAGS (f, &nf, key)) return clax_err_badfile;
    if (fread (pos, sizeof (pos), 1, f) != 1) return clax_err_badfile;
    vec_make (pos[0], pos[1], pos[2], &key->val._vect);
    vec_swap (&key->val._vect);
    add_key (track, key, nf);
  }
  spline_init (track);
  clax_set_track (clax_key_scale, c_id, track);
  return clax_err_ok;
}

static int read_TRACKFOV (FILE *f)
{
/*
  read_TRACKFOV: Track FOV reader.
*/
  t_TRACK *track;
  t_KEY *key;
  word  n, nf;
  float fov;

  track = alloc_track();
  if (read_TFLAGS (f, track, &n) != 0) return clax_err_badfile;
  while (n-- > 0) {
    if ((key = (t_KEY *)malloc (sizeof (t_KEY))) == NULL)
      return clax_err_nomem;
    if (read_KFLAGS (f, &nf, key)) return clax_err_badfile;
    if (fread (&fov, sizeof (fov), 1, f) != 1) return clax_err_badfile;
    key->val._float = fov;
    add_key (track, key, nf);
  }
  spline_init (track);
  clax_set_track (clax_key_fov, c_id, track);
  return clax_err_ok;
}

static int read_TRACKROLL (FILE *f)
{
/*
  read_TRACKROLL: Track ROLL reader.
*/
  t_TRACK *track;
  t_KEY *key;
  word   n, nf;
  float  roll;

  track = alloc_track();
  if (read_TFLAGS (f, track, &n) != 0) return clax_err_badfile;
  while (n-- > 0) {
    if ((key = (t_KEY *)malloc (sizeof (t_KEY))) == NULL)
      return clax_err_nomem;
    if (read_KFLAGS (f, &nf, key)) return clax_err_badfile;
    if (fread(&roll, sizeof(roll), 1, f) != 1) return clax_err_badfile;
    key->val._float = roll;
    add_key (track, key, nf);
  }
  spline_init (track);
  clax_set_track (clax_key_roll, c_id, track);
  return clax_err_ok;
}

static int read_TRACKMORPH (FILE *f)
{
/*
  read_TRACKMORPH: Track morph reader.
*/
  t_TRACK *track;
  t_KEY  *key;
  w_NODE *node;
  word    n, nf;

  track = alloc_track();
  if (read_TFLAGS (f, track, &n) != 0) return clax_err_badfile;
  while (n-- > 0) {
    if ((key = (t_KEY *)malloc (sizeof (t_KEY))) == NULL)
      return clax_err_nomem;
    if (read_KFLAGS (f, &nf, key)) return clax_err_badfile;
    if (read_ASCIIZ (f)) return clax_err_badfile;
    clax_byname (c_string, &node);
    if (!node) return clax_err_undefined;
    key->val._int = ((c_OBJECT *)node->object)->id;
    add_key (track, key, nf);
  }
  clax_set_track (clax_key_morph, c_id, track);
  return clax_err_ok;
}

static int read_TRACKHIDE (FILE *f)
{
/*
  read_TRACKHIDE: Track hide reader.
*/
  t_TRACK *track;
  t_KEY *key;
  word   unknown[2];
  word   n, nf;
  int    hide = 0;

  track = alloc_track();
  if (read_TFLAGS (f, track, &n) != 0) return clax_err_badfile;
  while (n-- > 0) {
    if ((key = (t_KEY *)malloc (sizeof (t_KEY))) == NULL)
      return clax_err_nomem;
    if (fread (&nf, sizeof (nf), 1, f) != 1) return clax_err_badfile;
    if (fread (unknown, sizeof (word), 2, f) != 2) return clax_err_badfile;
    key->val._int = (hide ^= 1);
    add_key (track, key, nf);
  }
  clax_set_track (clax_key_hide, c_id, track);
  return clax_err_ok;
}

static int read_CHUNK (FILE *f, c_CHUNK *h)
{
/*
  read_CHUNK: Chunk reader.
*/
  if (fread (&h->chunk_id, sizeof (word), 1, f) != 1)
    return clax_err_badfile;
  if (fread (&h->chunk_size, sizeof (dword), 1, f) != 1)
    return clax_err_badfile;
  return clax_err_ok;
}

/*****************************************************************************
  chunk readers control
*****************************************************************************/

static int ChunkReaderWorld (FILE *f, long p, word parent)
{
/*
  ChunkReaderWorld: Recursive chunk reader (world).
*/
  c_CHUNK h;
  long    pc;
  int     n, i, error;

  c_chunk_last = parent;
  while ((pc = ftell (f)) < p) {
    if (read_CHUNK (f, &h) != 0) return clax_err_badfile;
    c_chunk_curr = h.chunk_id;
    n = -1;
    for (i = 0; i < sizeof (world_chunks) / sizeof (world_chunks[0]); i++)
      if (h.chunk_id == world_chunks[i].id) {
        n = i;
        break;
      }
    if (n < 0) fseek (f, pc + h.chunk_size, SEEK_SET);
    else {
      pc = pc + h.chunk_size;
      if ((error = world_chunks[n].func (f)) != 0) return error;
      if (world_chunks[n].sub)
        if ((error = ChunkReaderWorld (f, pc, h.chunk_id)) != 0)
          return error;
      fseek (f, pc, SEEK_SET);
      c_chunk_prev = h.chunk_id;
    }
    if (ferror (f)) return clax_err_badfile;
  }
  return clax_err_ok;
}

static int ChunkReaderKey (FILE *f, long p, word parent)
{
/*
  ChunkReaderKey: Recursive chunk reader (keyframer).
*/
  c_CHUNK h;
  long    pc;
  int     n, i, error;

  c_chunk_last = parent;
  while ((pc = ftell (f)) < p) {
    if (read_CHUNK (f, &h) != 0) return clax_err_badfile;
    c_chunk_curr = h.chunk_id;
    n = -1;
    for (i = 0; i < sizeof (key_chunks) / sizeof (key_chunks[0]); i++)
      if (h.chunk_id == key_chunks[i].id) {
        n = i;
        break;
      }
    if (n < 0) fseek (f, pc + h.chunk_size, SEEK_SET);
    else {
      pc = pc + h.chunk_size;
      if ((error = key_chunks[n].func (f)) != 0) return error;
      if (key_chunks[n].sub)
        if ((error = ChunkReaderKey (f, pc, h.chunk_id)) != 0) return error;
      fseek (f, pc, SEEK_SET);
      c_chunk_prev = h.chunk_id;
    }
    if (ferror (f)) return clax_err_badfile;
  }
  return clax_err_ok;
}

/*****************************************************************************
  world/motion load routines
*****************************************************************************/

int32 clax_load_mesh_3DS (FILE *f)
{
/*
  clax_load_mesh_3DS: loads mesh data from 3ds file "filename"
                      into scene "scene".
*/
  byte version;
  long length;

  c_id = 0;
  fseek (f, 0, SEEK_END);
  length = ftell (f);
  fseek (f, 28L, SEEK_SET);
  if (fread (&version, sizeof (byte), 1, f) != 1) return clax_err_badfile;
  if (version < 2) return clax_err_badver; /* 3DS 3.0+ supported */
  fseek (f, 0, SEEK_SET);
  return ChunkReaderWorld (f, length, 0);
}

int32 clax_load_motion_3DS (FILE *f)
{
/*
  clax_loadmotion: loads motion data from 3ds file "filename"
                   into scene "scene".
*/
  byte version;
  long length;

  c_id = -1;
  fseek (f, 0, SEEK_END);
  length = ftell (f);
  fseek (f, 28L, SEEK_SET);
  if (fread (&version, sizeof (byte), 1, f) != 1) return clax_err_badfile;
  if (version < 2) return clax_err_badver; /* 3DS 3.0+ supported */
  fseek (f, 0, SEEK_SET);
  return ChunkReaderKey (f, length, 0);
}
