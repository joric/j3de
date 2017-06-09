/*CLAX.C**********************************************************************

   Clax: Portable keyframing library
         see inclosed LICENSE.TXT for licensing terms.

         for documentation, refer to CLAX.TXT

   author           : Borzom
   file created     : 17/04/97
   file description : main clax routines

   revision history :
     v0.10 (17/04/97) Borzom: Initial version.

   notes            :
     the library itself, if it has bugs, its probably here :)

*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "clax.h"
#include "claxi.h"

#include "main.h"
#include "light.h"
#include "render.h"
#include "fatmap.h"
#include "meta.h"



/*****************************************************************************
  copyright, format table, etc...
*****************************************************************************/

  char    clax_version[]   = "clax version 0.10(beta)";
  char    clax_copyright[] = "copyright (c) 1997 borzom";

  c_SCENE  *clax_scene;  /* current active scene  */
  c_CAMERA *clax_camera; /* current active camera */
  int32     clax_flags;  /* curreng flags         */

  int    gstate;
  int    id = 0;
 

struct {
  char   *name;                   /* file extension        */
  int32 (*load_mesh)   (FILE *f); /* loads mesh            */
  int32 (*load_motion) (FILE *f); /* loads motion          */
  int32 (*save_scene)  (FILE *f); /* saves the whole scene */
} clax_drivers[] = {
  {"3DS", clax_load_mesh_3DS, clax_load_motion_3DS, NULL},
};


/*****************************************************************************
  internal functions
*****************************************************************************/

static void calc_objnormals (c_OBJECT *obj)
{
/*
  calc_normals: calculates face/vertex normals.
*/

  c_VECTOR  a, b, normal;
  int32     i, j, num;

  for (i = 0; i < obj->numfaces; i++) { /* face normals */
    vec_sub (&obj->faces[i].p[0]->vert,
             &obj->faces[i].p[1]->vert, &a);
    vec_sub (&obj->faces[i].p[1]->vert,
             &obj->faces[i].p[2]->vert, &b);
    vec_cross (&a, &b, &normal);
    vec_normalize (&normal, &obj->faces[i].norm);
  }  
  for (i = 0; i < obj->numverts; i++) { /* vertex normals */
    num = 0;
    vec_zero (&normal);
    for (j = 0; j < obj->numfaces; j++) {
      if (obj->faces[j].a == i ||
          obj->faces[j].b == i ||
          obj->faces[j].c == i) {
           vec_add (&normal, &obj->faces[j].norm, &normal);
        num++;
      }
    }
      if (num) vec_scale (&normal, 1.0 / (float)num, &normal);
      vec_normalize (&normal, &obj->vertices[i].norm);
	
  }

}

static void recalc_objnormals (c_OBJECT *obj)
{
/*
  calc_normals: calculates face/vertex normals.
*/
  c_VECTOR  a, b, normal;
  int32     i;

  for ( i = 0; i < obj->numverts; i++ )
    vec_zero( &obj->vertices[i].pnorm );

  for ( i = 0; i < obj->numfaces; i++ ) {
    vec_sub (&obj->faces[i].p[0]->pvert,
             &obj->faces[i].p[1]->pvert, &a);
    vec_sub (&obj->faces[i].p[1]->pvert,
             &obj->faces[i].p[2]->pvert, &b);
    vec_cross (&a, &b, &normal);
    vec_normalize (&normal, &obj->faces[i].pnorm);
    
    vec_add( &obj->faces[i].p[0]->pnorm, &normal, &obj->faces[i].p[0]->pnorm );
    vec_add( &obj->faces[i].p[1]->pnorm, &normal, &obj->faces[i].p[1]->pnorm );
    vec_add( &obj->faces[i].p[2]->pnorm, &normal, &obj->faces[i].p[2]->pnorm );
  }
  for ( i = 0; i < obj->numverts; i++ )
    vec_normalize( &obj->vertices[i].pnorm, &obj->vertices[i].pnorm );
}

static
void
calc_normals ()
{
/*
  calc_normals: calculates face/vertex normals.
*/
  w_NODE *node;

  for (node = clax_scene->world; node; node = node->next)
    if (node->type == clax_obj_object)
      calc_objnormals ((c_OBJECT *)node->object);
}

static
void
calc_bbox ()
{
/*
  calc_bbox: calculate bounding boxes for objects.
*/
  w_NODE   *node;
  c_OBJECT *obj;
  c_VECTOR  min, max;
  int       i;

  for (node = clax_scene->world; node; node = node->next)
    if (node->type == clax_obj_object) {
      obj = (c_OBJECT *)node->object;

	  min.x = min.y = min.z = 0;
	  max.x = max.y = max.z = 0;

	  if( obj->numverts ) {
		  vec_copy (&obj->vertices[0].vert, &min);
		  vec_copy (&obj->vertices[0].vert, &max);

		  for (i = 1; i < obj->numverts; i++) {

			if (obj->vertices[i].vert.x < min.x)
			  min.x = obj->vertices[i].vert.x;
			if (obj->vertices[i].vert.y < min.y)
			  min.y = obj->vertices[i].vert.y;
			if (obj->vertices[i].vert.z < min.z)
			  min.z = obj->vertices[i].vert.z;

			if (obj->vertices[i].vert.x > max.x)
			  max.x = obj->vertices[i].vert.x;
			if (obj->vertices[i].vert.y > max.y)
			  max.y = obj->vertices[i].vert.y;
			if (obj->vertices[i].vert.z > max.z)
			  max.z = obj->vertices[i].vert.z;
		  }
	  }

      // build box
      obj->bbox.p[0].x = max.x; obj->bbox.p[0].y = max.y; obj->bbox.p[0].z = max.z;
      obj->bbox.p[1].x = max.x; obj->bbox.p[1].y = max.y; obj->bbox.p[1].z = min.z;
      obj->bbox.p[2].x = max.x; obj->bbox.p[2].y = min.y; obj->bbox.p[2].z = min.z;
      obj->bbox.p[3].x = max.x; obj->bbox.p[3].y = min.y; obj->bbox.p[3].z = max.z;
      obj->bbox.p[4].x = min.x; obj->bbox.p[4].y = max.y; obj->bbox.p[4].z = max.z;
      obj->bbox.p[5].x = min.x; obj->bbox.p[5].y = max.y; obj->bbox.p[5].z = min.z;
      obj->bbox.p[6].x = min.x; obj->bbox.p[6].y = min.y; obj->bbox.p[6].z = min.z;
      obj->bbox.p[7].x = min.x; obj->bbox.p[7].y = min.y; obj->bbox.p[7].z = max.z;
    }
}


#define  CULL_OUTSIDE     0
#define  CULL_INTERSECT   1
#define  CULL_INSIDE      2

#define  CLIP_FRONT       0x01
#define  CLIP_BEHIND      0x02
#define  CLIP_LEFT        0x04
#define  CLIP_RIGHT       0x08
#define  CLIP_ABOVE       0x10
#define  CLIP_BELOW       0x20


// returns 1 if  a < b
int
isLess( float a, float b )
{
// a < b
  float  diff = a - b;
  int    i = *(long *)&diff;
  return (unsigned long)i >> 31;
}


int
cullBox( c_BOUNDBOX* box, c_MATRIX m )
{
  int        i, d[8];
  c_VECTOR   vec;

  for ( i = 0; i < 8; i++ ) {  
    // transform points

    mat_mulvec( m, &box->p[i], &vec );
    vec.x *= clax_camera->xClip;
    vec.y *= clax_camera->yClip;

    // Check for visibility
    d[i] = 0;

    d[i] |= isLess( vec.z, clax_camera->znear );      // front
    d[i] |= isLess( clax_camera->zfar, vec.z ) << 1;  // behind
    d[i] |= isLess( vec.x, -vec.z ) << 2;             // left
    d[i] |= isLess( vec.z,  vec.x ) << 3;             // right
    d[i] |= isLess( vec.z,  vec.y ) << 4;             // above
    d[i] |= isLess( vec.y, -vec.z ) << 5;             // below
  }

  

  
  if ( d[0] & d[1] & d[2] & d[3] & d[4] & d[5] & d[6] & d[7] )
    return CULL_OUTSIDE;
  else if ( d[0] | d[1] | d[2] | d[3] | d[4] | d[5] | d[6] | d[7] )
    return CULL_INTERSECT;
  else
    return CULL_INSIDE;
}

void
intersect( c_VERTEX* out, c_VERTEX* in, c_VERTEX* mid, int plane )
{
  float  din, dout, t, it;

  switch ( plane ) {
    case CLIP_FRONT:
      din  = in->pvert.z - clax_camera->znear;
      dout = out->pvert.z - clax_camera->znear;
      break;
    case CLIP_BEHIND:
      din  = in->pvert.z - clax_camera->zfar;
      dout = out->pvert.z - clax_camera->zfar;
      break;
    case CLIP_LEFT:
      din  = in->pvert.z + in->pvert.x;
      dout = out->pvert.z + out->pvert.x;
      break;
    case CLIP_RIGHT:
      din  = in->pvert.z - in->pvert.x;
      dout = out->pvert.z - out->pvert.x;
      break;
    case CLIP_ABOVE:
      din  = in->pvert.z - in->pvert.y;
      dout = out->pvert.z - out->pvert.y;
      break;
    case CLIP_BELOW:
      din  = in->pvert.z + in->pvert.y;
      dout = out->pvert.z + out->pvert.y;
      break;
  }

  t = din / (din - dout);
  it = 1.0f - t;
  
  mid->pvert.x = t * out->pvert.x + it * in->pvert.x;
  mid->pvert.y = t * out->pvert.y + it * in->pvert.y;
  mid->pvert.z = t * out->pvert.z + it * in->pvert.z;

  mid->u       = t * out->u       + it * in->u;
  mid->v       = t * out->v       + it * in->v;

  mid->i       = t * out->i       + it * in->i;

  mid->flags = 0;


  switch ( plane ) {
    case CLIP_FRONT:
      mid->flags |= isLess( clax_camera->zfar, mid->pvert.z ) << 1;  // behind
    case CLIP_BEHIND:
      mid->flags |= isLess( mid->pvert.x, -mid->pvert.z ) << 2;      // left
    case CLIP_LEFT:
      mid->flags |= isLess( mid->pvert.z,  mid->pvert.x ) << 3;      // right
    case CLIP_RIGHT:
      mid->flags |= isLess( mid->pvert.z,  mid->pvert.y ) << 4;      // above
    case CLIP_ABOVE:
      mid->flags |= isLess( mid->pvert.y, -mid->pvert.z ) << 5;      // below
    case CLIP_BELOW:
      mid->flags |= isLess( mid->pvert.z, clax_camera->znear );      // front
      break;
  }

}

void
clipPoly( c_VERTEX* a, c_VERTEX* b, c_VERTEX* c, int id )
{
  int        i, j, clip;
  int        nextvert, nin, nout, plane;
  int        endC, startC;
  c_VERTEX*  start, * end, * q, ** tmp, ** in, ** out;
  static c_VERTEX*  inArray[15], * outArray[15];
  static c_VERTEX   newVert[15];


  plane = CLIP_FRONT;
  nout = 0;
  nin = 3;
  nextvert = 0;

  in = inArray;
  out = outArray;
  
  in[0] = a;
  in[1] = b;
  in[2] = c;

  for ( i = 0; i < 6; i++ ) {
  
    // check if need to clip to this plane
    clip = 0;
    for ( j = 0; j < nin; j++ ) {
      start = in[j];
      if ( start->flags & plane )
        clip = 1;
    }
    
    if ( clip ) {
      for ( j = 0; j < nin; j++ ) {
        start = in[j];
        startC = start->flags & plane;
        if ( j == (nin - 1) )
          end = in[0];  // wrap
        else
          end = in[j + 1];

        endC = end->flags & plane;
        
        if ( startC ) {
          if ( !endC ) {
            q = &newVert[nextvert++];
            intersect( start, end, q, plane );
            out[nout++] = q;
          }
        } else {
          if ( endC ) {
            q = &newVert[nextvert++];
            intersect( end, start, q, plane );
            out[nout++] = start;
            out[nout++] = q;
          } else {
            out[nout++] = start;
          }
        }
      }
      tmp = in;
      in = out;
      out = tmp;
      nin = nout;
      nout = 0;
    
    }    
    plane <<= 1;
  }

  // render clipped polygon

  if ( nin != 0 ) {
    for ( j = 0; j < nin; j++ ) {
      in[j]->sz = 1.0f/in[j]->pvert.z;
      in[j]->sx = clax_camera->xOffset + (in[j]->pvert.x * clax_camera->xOffset * in[j]->sz);
      in[j]->sy = clax_camera->yOffset - (in[j]->pvert.y * clax_camera->yOffset * in[j]->sz);
    }
    
    for ( j = 1; j < nin - 1; j++ ) {
      drawTriangle( in[0], in[j], in[j + 1], id );
    }
  }

}





//static
void
do_transform()
{

  w_NODE*    node, * from, * to , *node1;
  c_OBJECT*  obj, * o1, * o2, *backup;
  c_VERTEX*  v1, * v2;
  c_FACE*    f1, * f2;
  c_VECTOR   vec, camPos;
  c_MATRIX   objmat, invobjmat;
  float      alpha, invDepth;
  float      intensity;
  int32      i, j, cull;
  c_FACE*   face;
  c_VERTEX*  v;
  c_BOUNDBOX  pbbox;
  float  dot;
  c_VECTOR  sight;

  // frame id
  id++;

  if ( !clax_camera )
    return;

  // for depth cueuing..    
  invDepth = 1.0f / (clax_camera->zfar - clax_camera->znear);
  
  // initialize light list
  initLights( clax_scene, clax_camera->matrix );

  for ( node = clax_scene->world; node; node = node->next ) 
  {
	  
    if ( node->type == clax_obj_object ) {

	  
      obj = (c_OBJECT *)node->object;

	  c_OBJECT o_copy;

	  bool use_envmap=false;

	  m_shade=0;
	  if (curFrame>350)
	  {	
	  char name[]="Fx01";
	  if ( strcmp (obj->name, name) == 0)
	  {
			use_envmap=true;
			NextFrame(curFrame); //metaballs
			{			
				memcpy(&o_copy,obj,sizeof(c_OBJECT));
				o_copy.numverts = triangles_tesselated*3;
				o_copy.numfaces = triangles_tesselated;
				o_copy.faces	= mf;
				o_copy.vertices	= mv;
				obj=&o_copy;
			}
			m_shade=1;
	  }	  
	  }
	  if (rootcmp (obj->name, "Part") == 0)
	  {
			clax_byname("Box13",&node1);
			o1 = (c_OBJECT *)node1->object;
				memcpy(&o_copy,o1,sizeof(c_OBJECT));
				mat_copy(obj->matrix, o_copy.matrix);
				obj=&o_copy;
	  }
/*
	  if (rootcmp (obj->name, "Part") == 0)
	  {
			clax_byname("Box12",&node1);
			o1 = (c_OBJECT *)node1->object;
				memcpy(&o_copy,obj,sizeof(c_OBJECT));
				mat_copy(obj->matrix, o_copy.matrix);
				obj=&o_copy;
	  }
*/	  
/*
        printf ("<<object>> name: %s, id: %d\n", obj->name, obj->id);
        printf ("  parent:      %d\n", obj->parent);
        printf ("  vertices:    %d, faces: %d\n", obj->numverts, obj->numfaces);
        printf ("  pivot:       x: %9.3f, y: %9.3f, z: %9.3f\n",
                obj->pivot.x, obj->pivot.y, obj->pivot.z);
        printf ("  translate:   x: %9.3f, y: %9.3f, z: %9.3f\n",
                obj->translate.x, obj->translate.y, obj->translate.z);
        printf ("  scale:       x: %9.3f, y: %9.3f, z: %9.3f\n",
                obj->scale.x, obj->scale.y, obj->scale.z);
        printf ("  rotate:      x: %9.3f, y: %9.3f, z: %9.3f, w: %9.3f\n",
                obj->rotate.x, obj->rotate.y, obj->rotate.z, obj->rotate.w);
        printf ("  flags:       ");
        if (obj->flags) {
          if (obj->flags & clax_obj_hidden) printf ("hidden ");
          if (obj->flags & clax_obj_chidden) printf ("display-hidden ");
          if (obj->flags & clax_obj_dummy) printf ("dummy ");
          if (obj->flags & clax_obj_morph) printf ("morph ");
        } else printf ("none");
        printf ("\n  matrix:\n");
        printf ("    xx: %9.3f xy: %9.3f xz: %9.3f xw: %9.3f\n",
                obj->matrix[X][X], obj->matrix[X][Y],
                obj->matrix[X][Z], obj->matrix[X][W]);
        printf ("    yx: %9.3f yy: %9.3f yz: %9.3f yw: %9.3f\n",
                obj->matrix[Y][X], obj->matrix[Y][Y],
                obj->matrix[Y][Z], obj->matrix[Y][W]);
        printf ("    zx: %9.3f zy: %9.3f zz: %9.3f zw: %9.3f\n",
                obj->matrix[Z][X], obj->matrix[Z][Y],
                obj->matrix[Z][Z], obj->matrix[Z][W]);
        printf("\n");

        printf ("  vertex list:\n");
        for (i = 0; i < obj->numverts; i++) {
          printf ("    x: %9.3f, y: %9.3f, z: %9.3f\n",
                  obj->vertices[i].vert.x,
                  obj->vertices[i].vert.y,
                  obj->vertices[i].vert.z );
        }
        printf ("  face list:\n");
        for (i = 0; i < obj->numfaces; i++) {
          printf ("    a: %3d b: %3d c: %3d, flags %3d, material: %3d\n",
                  obj->faces[i].a, obj->faces[i].b, obj->faces[i].c,
                  obj->faces[i].flags, obj->faces[i].mat);
        }
        printf ("\n");
///////////*/



      mat_mul( clax_camera->matrix, obj->matrix, objmat );

	  if( ((obj->flags & clax_obj_hidden) == 0) &&
          ((obj->flags & clax_obj_chidden) == 0) )         
      {        
		{
            
          cull = cullBox( &obj->bbox, objmat );
          if ( cull == CULL_OUTSIDE )
            continue;
  
          mat_inverse_c( obj->matrix, invobjmat );
    
          face = obj->faces;
          mat_mulvec( invobjmat, &clax_camera->pos, &camPos );

          for ( i = 0; i < obj->numfaces; i++ ) {

			mat_mulnorm( objmat, &face->norm, &face->pnorm );			

            sight.x = face->p[0]->vert.x - camPos.x;
            sight.y = face->p[0]->vert.y - camPos.y;
            sight.z = face->p[0]->vert.z - camPos.z;
            
            dot = vec_dotunit( &face->norm, &sight );

			float clr=(face->pnorm.z);
			if (clr<0) clr=0;
			if (clr>1) clr=1;	

			m_intensity=clr*200+55;

           if ( dot > 0.0 )
			{				

              for ( j = 0; j < 3; j++ ) 
			  {
  
                v = face->p[j];
  
//				if ( v->id != id )
				{
                  v->id = id;

                  // transform

				  mat_mulvec( objmat, &v->vert, &v->pvert );

                  mat_mulnorm( objmat, &v->norm, &v->pnorm );
  
                  // calculate shading
                  intensity = calcLight( &v->pvert, &v->pnorm );

				  v->i = intensity*255;

				  //env_mapping - just a fast hack
				  if (use_envmap){
				  c_VECTOR uv; vec_normalize(&v->pnorm,&uv);
				  v->u = map(uv.x); v->v = map(uv.y);};
				  // end of hack

                  if ( cull == CULL_INTERSECT ) {
  
                    v->pvert.x *= clax_camera->xClip;
                    v->pvert.y *= clax_camera->yClip;
  
                    // compute outcode
                    v->flags = 0;
                    v->flags |= isLess( v->pvert.z, clax_camera->znear );      // front
                    v->flags |= isLess( clax_camera->zfar, v->pvert.z ) << 1;  // behind
                    v->flags |= isLess( v->pvert.x, -v->pvert.z ) << 2;        // left
                    v->flags |= isLess( v->pvert.z,  v->pvert.x ) << 3;        // right
                    v->flags |= isLess( v->pvert.z,  v->pvert.y ) << 4;        // above
                    v->flags |= isLess( v->pvert.y, -v->pvert.z ) << 5;        // below

                    if ( v->flags == 0 ) {
                      // project vertex if it doesn't need clipping
                      v->sz = 1.0f / v->pvert.z;
                      v->sx = clax_camera->xOffset + (v->pvert.x * clax_camera->xOffset * v->sz);
                      v->sy = clax_camera->yOffset - (v->pvert.y * clax_camera->yOffset * v->sz);
                    }
                  } else {
  
                    // project vertex                
                    v->sz = 1.0f / v->pvert.z;
                    v->sx = clax_camera->xOffset + (v->pvert.x * clax_camera->xScale * v->sz);
                    v->sy = clax_camera->yOffset - (v->pvert.y * clax_camera->yScale * v->sz);
  
                  }
                }
              }
             
              // draw face

			  int id=0;
			  if (face->pmat) id=(int)face->pmat->id; // GPF!!! ????


			  //material - just a fast hack
			  if (obj==&o_copy) { id=8; };


              if ( cull == CULL_INTERSECT ) {
                if ( !(face->p[0]->flags & face->p[1]->flags & face->p[2]->flags) ) {
  
                  if ( face->p[0]->flags | face->p[1]->flags | face->p[2]->flags ) {
                    gstate = 1;
                    clipPoly( face->p[0], face->p[1], face->p[2], id );
                  } else {
                    gstate = 0;
                    drawTriangle( face->p[0], face->p[1], face->p[2], id );
                  }
                }
              } else {
                gstate = 0;
                drawTriangle( face->p[0], face->p[1], face->p[2], id );
              }
            } 

            face++;
          }
			
        }
			
      }

    }	


  }


}


static void clax_free_track (t_TRACK *track)
{
/*
  clax_free_track: deallocated memory used by track.
*/
  t_KEY *key, *next;

  if (!track) return;
  for (key = track->keys; key; key = next) {
    next = key->next;
    free (key);
  }
  free (track);
}

static int32 strucmp (char *s1, char *s2)
{
/*
  strucmp: non-case sensitive string compare.
*/
  int32 diff = 0;

  do {
    diff += (toupper (*s1) - toupper (*s2));
  } while (*s1++ && *s2++);
  return diff;
}

static int32 rootcmp (char *s1, char *s2)
{
/*
  strucmp: compare strings before point
*/
  int32 diff = 0;

  do {
    diff += (toupper (*s1) - toupper (*s2));
  } while (*s1++ && *s2++ && (*s2==(char)"."));
  return diff;
}


/*****************************************************************************
  clax library (initialization, error handling)
*****************************************************************************/

int32 clax_init (int32 flags)
{
/*
  clax_init: clax initialization.
*/
  clax_scene  = NULL;
  clax_camera = NULL;
  clax_flags  = flags;
  return clax_err_ok;
}

int32 clax_done ()
{
/*
  clax_done: clax deinitialization.
*/
  clax_scene = NULL;
  clax_flags = 0;
  return clax_err_ok;
}

char *clax_geterror (int32 code)
{
/*
  clax_geterror: return error string.
*/
  switch (code) {
    case clax_err_nomem:     return "not enough memory";
    case clax_err_nofile:    return "file not found";
    case clax_err_badfile:   return "corrupted file";
    case clax_err_badver:    return "unsupported version";
    case clax_err_badformat: return "unsupported format";
    case clax_err_badframe:  return "invalid frame number";
    case clax_err_badname:   return "invalid object name";
    case clax_err_noframes:  return "no frames in keyframer";
    case clax_err_notloaded: return "scene not loaded";
    case clax_err_nullptr:   return "null pointer assignment";
    case clax_err_invparam:  return "invalid parameter";
    case clax_err_spline:    return "less than 2 keys in spline";
    case clax_err_singular:  return "cannot inverse singular matrix";
    case clax_err_badid:     return "bad object id";
    case clax_err_exist:     return "object already exist";
    case clax_err_undefined:
    default:                 return "internal error";
  }
}

/*****************************************************************************
  clax library (time and world handling)
*****************************************************************************/

int32 clax_getframes (float *start, float *end)
{
/*
  clax_getframes: return number of frames.
*/
  if (!clax_scene) return clax_err_notloaded;
  if (!clax_scene->keyframer) return clax_err_notloaded;
  if (clax_scene->f_end - clax_scene->f_start == 0) return clax_err_noframes;
  *start = clax_scene->f_start;
  *end = clax_scene->f_end;
  return clax_err_ok;
}

int32 clax_setframe (float frame)
{
/*
  clax_setframe: set current frame number.
*/
  if (!clax_scene) return clax_err_notloaded;
  if (!clax_scene->keyframer) return clax_err_notloaded;
  if (clax_scene->f_end - clax_scene->f_start == 0) return clax_err_noframes;
  if (clax_scene->f_start <= frame && clax_scene->f_end > frame) {
    clax_scene->f_current = frame;
    return clax_err_ok;
  } else return clax_err_badframe;
}

int32 clax_getframe (float *frame)
{
/*
  clax_getframe: get current frame number.
*/
  if (!clax_scene) return clax_err_notloaded;
  if (!clax_scene->keyframer) return clax_err_notloaded;
  if (clax_scene->f_end - clax_scene->f_start == 0) return clax_err_noframes;
  *frame = clax_scene->f_current;
  return clax_err_ok;
}

int32 clax_setactive_scene (c_SCENE *scene)
{
/*
  clax_setactive_scene: set active scene.
*/
  if (!scene) return clax_err_nullptr;
  clax_scene = scene;
  return clax_err_ok;
}

int32 clax_setactive_camera (c_CAMERA *cam)
{
/*
  clax_setactive_camera: set active camera.
*/
  clax_camera = cam;
  return clax_err_ok;
}

int32 clax_getactive_scene (c_SCENE **scene)
{
/*
  clax_getactive_scene: get active scene.
*/
  *scene = clax_scene;
  return clax_err_ok;
}

int32 clax_getactive_camera (c_CAMERA **camera)
{
/*
  clax_getactive_camera: get active camera.
*/
  *camera = clax_camera;
  return clax_err_ok;
}

int32 clax_byname (char *name, w_NODE **node)
{
/*
  clax_byname: find object by name (world tree).
*/
  if (!clax_scene || !clax_scene->world) return clax_err_notloaded;
  for (*node = clax_scene->world; *node; *node = (*node)->next) {
    switch ((*node)->type) {
      case clax_obj_camera:
        if (strcmp (((c_CAMERA *)((*node)->object))->name, name) == 0)
          return clax_err_ok; break;
      case clax_obj_object:
        if (strcmp (((c_OBJECT *)((*node)->object))->name, name) == 0)
          return clax_err_ok; break;
      case clax_obj_light:
        if (strcmp (((c_LIGHT *)((*node)->object))->name, name) == 0)
          return clax_err_ok; break;
      case clax_obj_material:
        if (strcmp (((c_MATERIAL *)((*node)->object))->name, name) == 0)
          return clax_err_ok; break;
      case clax_obj_ambient:
        if (strcmp (((c_AMBIENT *)((*node)->object))->name, name) == 0)
          return clax_err_ok;
    }
  }
  return clax_err_ok; /* return NULL */
}

int32 clax_byid (int32 id, w_NODE **node)
{
/*
  clax_byid: find object by id (world tree).
*/
  if (!clax_scene || !clax_scene->world) return clax_err_notloaded;
  for (*node = clax_scene->world; *node; *node = (*node)->next) {
    switch ((*node)->type) {
      case clax_obj_camera:
        if (((c_CAMERA *)((*node)->object))->id == id) return clax_err_ok;
        break;
      case clax_obj_object:
        if (((c_OBJECT *)((*node)->object))->id == id) return clax_err_ok;
        break;
      case clax_obj_light:
        if (((c_LIGHT *)((*node)->object))->id == id) return clax_err_ok;
        break;
      case clax_obj_material:
        if (((c_MATERIAL *)((*node)->object))->id == id) return clax_err_ok;
        break;
      case clax_obj_ambient:
        if (((c_AMBIENT *)((*node)->object))->id == id) return clax_err_ok;
    }
  }
  return clax_err_ok; /* return NULL */
}

int32 clax_findfirst (int32 attr, w_NODE **node)
{
/*
  clax_findfirst: finds first node with attribute "attr" (world tree).
*/
  if (!clax_scene || !clax_scene->world) return clax_err_notloaded;
  for (*node = clax_scene->world; *node; *node = (*node)->next)
    if ((*node)->type & attr) return clax_err_ok;
  return clax_err_ok; /* return NULL */
}

int32 clax_findnext (int32 attr, w_NODE **node)
{
/*
  clax_findnext: finds next node with attribute "attr" (world tree).
*/
  if (!clax_scene || !clax_scene->world) return clax_err_notloaded;
  for (*node = (*node)->next; *node; *node = (*node)->next)
    if ((*node)->type & attr) return clax_err_ok;
  return clax_err_ok; /* return NULL */
}

/*****************************************************************************
  clax library (world/keyframer constructors)
*****************************************************************************/

int32 clax_add_world (int32 type, void *obj)
{
/*
  clax_add_world: add object to world list.
*/
  w_NODE *node;

  if (!clax_scene) return clax_err_notloaded;
  if ((node = (w_NODE *)malloc (sizeof (w_NODE))) == NULL)
    return clax_err_nomem;
  node->type = type;
  node->object = obj;
  node->next = NULL;
  if (!clax_scene->world) {
    node->prev = NULL;
    clax_scene->world = node;
    clax_scene->wtail = node;
  } else {
    node->prev = clax_scene->wtail;
    clax_scene->wtail->next = node;
    clax_scene->wtail = node;
  }
  return clax_err_ok;
}

int32 clax_add_track (int32 type, int32 id, int32 parent, void *track, void *obj)
{
/*
  clax_add_track: add track to keyframer list.
*/
  k_NODE *node, *pnode;

  if (!clax_scene) return clax_err_notloaded;
  if ((node = (k_NODE *)malloc (sizeof (k_NODE))) == NULL)
    return clax_err_nomem;
  node->type = type;
  node->id = id;
  node->track = track;
  node->brother = NULL;
  node->child = NULL;
  node->next = NULL;
  node->object = obj;
  if (!clax_scene->keyframer) {
    node->prev = NULL;
    clax_scene->keyframer = node;
    clax_scene->ktail = node;
  } else {
    node->prev = clax_scene->ktail;
    clax_scene->ktail->next = node;
    clax_scene->ktail = node;
  }
  if (parent != -1) { /* update hierarchy tree */
    for (pnode = clax_scene->keyframer; pnode; pnode = pnode->next)
      if (pnode->id == parent) {
        node->parent = pnode;
        if (pnode->child == NULL) pnode->child = node; else {
          node->brother = pnode->child;
          pnode->child = node;
        }
      }
  } else node->parent = NULL;
  return clax_err_ok;
}

int32 clax_set_track (int32 type, int32 id, t_TRACK *track)
{
/*
  clax_set_track: assign a track to keyframer node.
*/
  k_NODE *node = clax_scene->keyframer;
  void   *obj;

  if (!clax_scene || !clax_scene->keyframer) return clax_err_notloaded;
  while (node && node->id != id) node = node->next;
  if (!node) return clax_err_undefined;
  obj = node->track;

  switch (node->type) {
    case clax_track_camera:
      switch (type) {
        case clax_key_pos:  ((t_CAMERA *)obj)->pos = track; break;
        case clax_key_fov:  ((t_CAMERA *)obj)->fov = track; break;
        case clax_key_roll: ((t_CAMERA *)obj)->roll = track;
      }
      break;
    case clax_track_cameratgt:
      switch (type) {
        case clax_key_pos: ((t_CAMERATGT *)obj)->pos = track;
      }
      break;
    case clax_track_light:
      switch (type) {
        case clax_key_pos:   ((t_LIGHT *)obj)->pos = track; break;
        case clax_key_color: ((t_LIGHT *)obj)->color = track;
      }
      break;
    case clax_track_lighttgt:
      switch (type) {
        case clax_key_pos:   ((t_LIGHTTGT *)obj)->pos = track; break;
      }
    case clax_track_spotlight:
      switch (type) {
        case clax_key_pos:   ((t_SPOTLIGHT *)obj)->pos = track; break;
        case clax_key_color: ((t_SPOTLIGHT *)obj)->color = track; break;
        case clax_key_roll:  ((t_SPOTLIGHT *)obj)->roll = track;
      }
      break;
    case clax_track_object:
      switch (type) {
        case clax_key_pos:    ((t_OBJECT *)obj)->translate = track; break;
        case clax_key_scale:  ((t_OBJECT *)obj)->scale = track; break;
        case clax_key_rotate: ((t_OBJECT *)obj)->rotate = track; break;
        case clax_key_morph:  ((t_OBJECT *)obj)->morph = track; break;
        case clax_key_hide:   ((t_OBJECT *)obj)->hide = track;
      }
      break;
    case clax_track_ambient:
      switch (type) {
        case clax_key_color: ((t_AMBIENT *)obj)->color = track;
      }
  }
  return clax_err_ok;
}

/*****************************************************************************
  clax library (scene load/save/free functions)
*****************************************************************************/

int32 clax_alloc_scene (c_SCENE **scene)
{
/*
  clax_alloc_scene: allocates memory for a new scene.
*/
  if ((*scene = (c_SCENE *)malloc (sizeof (c_SCENE))) == NULL)
    return clax_err_nomem;
  (*scene)->world = NULL;
  (*scene)->wtail = NULL;
  (*scene)->keyframer = NULL;
  (*scene)->ktail = NULL;
  return clax_err_ok;
}

int32 clax_load_world (char *filename, c_SCENE *scene)
{
/*
  clax_load_world: loads mesh data from file "filename" into scene "scene".
*/
//  int32   (*loader)(FILE *f) = NULL;
  c_SCENE *old_scene = clax_scene;
  FILE    *f;
  int32    error;

/*
  for (i = 0; i < sizeof (clax_drivers) / sizeof (clax_drivers[0]); i++) {
    s = strchr (filename, '.') + 1;
    if (strucmp (s, clax_drivers[i].name) == 0)
      loader = clax_drivers[i].load_mesh;
  }
  if (!loader) return clax_err_badformat;
*/

  scene->f_start = 0;
  scene->f_end = 0;
  scene->f_current = 0;
  scene->world = NULL;
  scene->wtail = NULL;
  scene->keyframer = NULL;
  scene->ktail = NULL;
  if ((f = fopen (filename, "rb")) == NULL) return clax_err_nofile;
  clax_setactive_scene (scene);
//  error = loader (f);
  error = clax_load_mesh_3DS( f );
  fclose (f);

  if (error) {
    clax_setactive_scene (old_scene);
    clax_free_world (scene);
    return error;
  }

  clax_setactive_scene (old_scene);
  return clax_err_ok;
}

int32 clax_load_motion (char *filename, c_SCENE *scene)
{
/*
  clax_load_motion: loads motion data from file "filename"
                    into scene "scene".
*/
//  int32    (*loader)(FILE *f) = NULL;
  c_SCENE  *old_scene = clax_scene;
  FILE     *f;
  int32     error;

/*
  if (!scene->world) return clax_err_notloaded;
  for (i = 0; i < sizeof (clax_drivers) / sizeof (clax_drivers[0]); i++) {
    s = strchr (filename, '.') + 1;
    if (strucmp (s, clax_drivers[i].name) == 0)
      loader = clax_drivers[i].load_motion;
  }
  if (!loader) return clax_err_badformat;
*/

  if ((f = fopen (filename, "rb")) == NULL) return clax_err_nofile;
  clax_setactive_scene (scene);
  error = clax_load_motion_3DS( f );
//  error = loader (f);
  fclose (f);
  if (error) {
    clax_setactive_scene (old_scene);
    clax_free_motion (scene);
    return error;
  }
  
  clax_setframe (0);
  clax_update ();
  calc_bbox ();
  calc_normals();

  clax_setactive_scene (old_scene);
  return clax_err_ok;
}

int32 clax_load_scene (char *filename, c_SCENE *scene)
{
/*
  clax_load_scene: loads mesh and keyframer data from file "filename"
                   into scene "scene".
*/
  int32 error;

  if ((error = clax_load_world (filename, scene)) != clax_err_ok)
    return error;
  return clax_load_motion (filename, scene);
}

int32 clax_free_world (c_SCENE *scene)
{
/*
  clax_free_world: release all memory used by world.
*/
  w_NODE     *node, *next;
  c_LIGHT    *light;
  c_CAMERA   *cam;
  c_OBJECT   *obj;
  c_MATERIAL *mat;
  c_AMBIENT  *amb;

  if (!scene || !scene->world) return clax_err_nullptr;
  for (node = scene->world; node; node = next) {
    switch (node->type) {
      case clax_obj_light:
        light = (c_LIGHT *)node->object;
        free (light->name);
        break;
      case clax_obj_camera:
        cam = (c_CAMERA *)node->object;
        free (cam->name);
        break;
      case clax_obj_object:
        obj = (c_OBJECT *)node->object;
        free (obj->name);
        free (obj->vertices);
        free (obj->faces);
        break;
      case clax_obj_material:
        mat = (c_MATERIAL *)node->object;
        free (mat->name);
        break;
      case clax_obj_ambient:
        amb = (c_AMBIENT *)node->object;
        free (amb->name);
    }
    next = node->next;
    free (node->object);
    free (node);
  }
  scene->world = NULL;
  scene->wtail = NULL;
  return clax_err_ok;
}

int32 clax_free_motion (c_SCENE *scene)
{
/*
  clax_free_motion: release all memory used by keyframer.
*/
  k_NODE      *node, *next;
  t_CAMERA    *cam;
  t_CAMERATGT *camtgt;
  t_LIGHT     *light;
  t_LIGHTTGT  *litgt;
  t_SPOTLIGHT *spotl;
  t_OBJECT    *obj;
  t_AMBIENT   *amb;

  if (!scene || !scene->keyframer) return clax_err_nullptr;
  for (node = scene->keyframer; node; node = next) {
    switch (node->type) {
      case clax_track_camera:
        cam = (t_CAMERA *)node->track;
        clax_free_track (cam->pos);
        clax_free_track (cam->fov);
        clax_free_track (cam->roll);
        break;
      case clax_track_cameratgt:
        camtgt = (t_CAMERATGT *)node->track;
        clax_free_track (camtgt->pos);
        break;
      case clax_track_light:
        light = (t_LIGHT *)node->track;
        clax_free_track (light->pos);
        clax_free_track (light->color);
        break;
      case clax_track_spotlight:
        spotl = (t_SPOTLIGHT *)node->track;
        clax_free_track (spotl->pos);
        clax_free_track (spotl->color);
        clax_free_track (spotl->roll);
        break;
      case clax_track_lighttgt:
        litgt = (t_LIGHTTGT *)node->track;
        clax_free_track (litgt->pos);
        break;
      case clax_track_object:
        obj = (t_OBJECT *)node->track;
        clax_free_track (obj->translate);
        clax_free_track (obj->scale);
        clax_free_track (obj->rotate);
        clax_free_track (obj->morph);
        clax_free_track (obj->hide);
        break;
      case clax_track_ambient:
        amb = (t_AMBIENT *)node->track;
        clax_free_track (amb->color);
    }
    next = node->next;
    free (node->track);
    free (node);
  }
  scene->keyframer = NULL;
  scene->ktail = NULL;
  return clax_err_ok;
}

int32 clax_free_mesh (c_SCENE *scene)
{
/*
  clax_free_mesh: release all memory used by meshes.
*/
  w_NODE     *node;
  c_OBJECT   *obj;

  if (!scene || !scene->world ||
      (clax_flags & clax_transform)) return clax_err_nullptr;
  for (node = scene->world; node; node = node->next) {
    switch (node->type) {
      case clax_obj_object:
        obj = (c_OBJECT *)node->object;
        free (obj->vertices); obj->vertices = NULL;
        free (obj->faces);    obj->faces = NULL;
        break;
    }
  }
  return clax_err_ok;
}

int32 clax_free_scene (c_SCENE *scene)
{
  int32 error;

  if ((error = clax_free_world (scene)) != clax_err_ok) return error;
  return clax_free_motion (scene);
}


/*****************************************************************************
  clax library (track handling)
*****************************************************************************/

int32 clax_getkey_float (t_TRACK *track, float frame, float *out)
{
/*
  clax_getkey_float: return float key at frame "frame".
*/
  t_KEY *keys;

  if (frame < 0.0) return clax_err_badframe;
  if (!track || !track->keys) return clax_err_nullptr;
  keys = track->keys;

  if (track->flags != 0) frame = (float)fmod (frame, track->frames);

  if (!keys->next || frame < keys->frame) {
    *out = keys->val._float;
    return clax_err_ok;
  }
  /* more than one key, spline interpolation */
  return spline_getkey_float (track, frame, out);
}

int32 clax_getkey_vect (t_TRACK *track, float frame, c_VECTOR *out)
{
/*
  clax_getkey_vect: return vector key at frame "frame".
*/
  t_KEY *keys;

  if (frame < 0.0) return clax_err_badframe;
  if (!track || !track->keys) return clax_err_nullptr;
  keys = track->keys;

  if (track->flags != 0) frame = (float)fmod (frame, track->frames);

  if (!keys->next || frame < keys->frame) {
    vec_copy (&keys->val._vect, out);
    return clax_err_ok;
  }
  /* more than one key, spline interpolation */
  return spline_getkey_vect (track, frame, out);
}

int32 clax_getkey_rgb (t_TRACK *track, float frame, c_RGB *out)
{
/*
  clax_getkey_rgb: return rgb key at frame "frame".
*/
  c_VECTOR *vect = (c_VECTOR *)out;

  return spline_getkey_vect (track, frame, vect);
}

int32 clax_getkey_quat (t_TRACK *track, float frame, c_QUAT *out)
{
/*
  clax_getkey_quat: return quaternion key at frame "frame".
*/
  t_KEY *keys;
  float  alpha;

  if (frame < 0.0) return clax_err_badframe;
  if (!track || !track->keys) return clax_err_nullptr;
  keys = track->keys;

  if (track->flags != 0) frame = (float)fmod (frame, track->frames);

  if (clax_flags & clax_slerp) {
    if (frame < track->last->frame) keys = track->keys; else
      keys = track->last;
    while (keys->next && frame > keys->next->frame) keys = keys->next;
    track->last = keys;
    if (!keys->next || frame < keys->frame) {
      qt_copy (&keys->qa, out);
      return clax_err_ok;
    }
    alpha = (frame - keys->frame) / (keys->next->frame - keys->frame);
    alpha = spline_ease (alpha, keys->easefrom, keys->next->easeto);
    qt_slerp (&keys->qa, &keys->next->qa, 0, alpha, out);
    return clax_err_ok;
  } else {
    if (!keys->next || frame < keys->frame) {
      qt_copy (&keys->qa, out);
      return clax_err_ok;
    }
    return spline_getkey_quat (track, frame, out);
  }
}

int32 clax_getkey_hide (t_TRACK *track, float frame, int32 *out)
{
/*
  clax_getkey_hide: return hide key at frame "frame".
*/
  t_KEY *keys;

  if (frame < 0.0) return clax_err_badframe;
  if (!track || !track->keys) {
    *out = 0;
    return clax_err_nullptr;
  }
  keys = track->keys;
  if (track->flags != 0) frame = (float)fmod (frame, track->frames);
  if (frame < keys->frame) {
    *out = 0;
    return clax_err_ok;
  }
  if (frame < track->last->frame) keys = track->keys; else
    keys = track->last;
  while (keys->next && frame > keys->next->frame) keys = keys->next;
  track->last = keys;
  *out = keys->val._int;
  return clax_err_ok;
}

int32 clax_getkey_morph (t_TRACK *track, float frame, c_MORPH *out)
{
/*
  clax_getkey_morph: return morph key at frame "frame".
*/
  t_KEY *keys;
  float  alpha;

  if (frame < 0.0) return clax_err_badframe;
  if (!track || !track->keys) return clax_err_nullptr;
  if (frame < track->last->frame) keys = track->keys; else
    keys = track->last;
  while (keys->next && frame > keys->next->frame) keys = keys->next;
  track->last = keys;
  if (!keys->next || frame < keys->frame) {
    out->from = keys->prev->val._int;
    out->to = keys->val._int;
    out->alpha = 1.0;
    return clax_err_ok;
  }
  out->from = keys->val._int;
  out->to = keys->next->val._int;
  alpha = (frame - keys->frame) / (keys->next->frame - keys->frame);
  out->alpha = spline_ease (alpha, keys->easefrom, keys->next->easeto);
  return clax_err_ok;
}

/*****************************************************************************
  clax library (keyframer)
*****************************************************************************/


int32 clax_update ()
{
/*
  clax_update: update all keyframer data.
*/
  k_NODE      *node, *child;
  c_CAMERA    *cam;
  t_CAMERA    *tcam;
  t_CAMERATGT *tcamt;
  c_LIGHT     *light;
  t_LIGHT     *tlight;
  t_SPOTLIGHT *tspotl;
  t_LIGHTTGT  *tlightt;
  c_AMBIENT   *amb;
  t_AMBIENT   *tamb;
  c_OBJECT    *obj,*cobj;
  t_OBJECT    *tobj;
  float        frame = clax_scene->f_current;
  c_MATRIX     c, d;
  int32        hidden;
  

  if (!clax_scene) return clax_err_notloaded;
  if (!clax_scene->world || !clax_scene->keyframer)
    return clax_err_notloaded;

  /* update objects */
  for (node = clax_scene->keyframer; node; node = node->next) {
    switch (node->type) {
      case clax_track_camera:
        cam = (c_CAMERA *)node->object;
        tcam = (t_CAMERA *)node->track;
        clax_getkey_vect (tcam->pos, frame, &cam->pos);
        clax_getkey_float (tcam->fov, frame, &cam->fov);
        clax_getkey_float (tcam->roll, frame, &cam->roll);
        break;
      case clax_track_cameratgt:
        cam = (c_CAMERA *)node->object;
        tcamt = (t_CAMERATGT *)node->track;
        clax_getkey_vect (tcamt->pos, frame, &cam->target);
        break;
      case clax_track_light:
        light = (c_LIGHT *)node->object;
        tlight = (t_LIGHT *)node->track;
        clax_getkey_vect (tlight->pos, frame, &light->pos);
        clax_getkey_rgb (tlight->color, frame, &light->color);
        break;
      case clax_track_spotlight:
        light = (c_LIGHT *)node->object;
        tspotl = (t_SPOTLIGHT *)node->track;
        clax_getkey_vect (tspotl->pos, frame, &light->pos);
        clax_getkey_rgb (tspotl->color, frame, &light->color);
        clax_getkey_float (tspotl->roll, frame, &light->roll);
        break;
      case clax_track_lighttgt:
        light = (c_LIGHT *)node->object;
        tlightt = (t_LIGHTTGT *)node->track;
        clax_getkey_vect (tlightt->pos, frame, &light->target);
        break;
      case clax_track_object:
        obj = (c_OBJECT *)node->object;
        tobj = (t_OBJECT *)node->track;
        clax_getkey_vect (tobj->translate, frame, &obj->translate);
        clax_getkey_vect (tobj->scale, frame, &obj->scale);
        clax_getkey_quat (tobj->rotate, frame, &obj->rotate);
        clax_getkey_hide (tobj->hide, frame, &hidden);

        if (clax_getkey_morph (tobj->morph, frame, &obj->morph) ==
            clax_err_ok) obj->flags |= clax_obj_morph;
            else obj->flags &= ~clax_obj_morph;
        if (hidden) obj->flags |= clax_obj_hidden; else
          obj->flags &= ~clax_obj_hidden;
        qt_invmatrix (&obj->rotate, c);
        mat_setscale (&obj->scale, d);
        c[X][W] = obj->translate.x;
        c[Y][W] = obj->translate.y;
        c[Z][W] = obj->translate.z;
        mat_mul (c, d, obj->matrix);
        break;
      case clax_track_ambient:
        amb = (c_AMBIENT *)node->object;
        tamb = (t_AMBIENT *)node->track;
        clax_getkey_rgb (tamb->color, frame, &amb->color);
    }
  }

  /* update hierarchy */
  if (clax_flags & clax_hierarchy) {
    for (node = clax_scene->keyframer; node; node = node->next) {
      if (node->type == clax_track_object) {
        for (child = node->child; child; child = child->brother) {
          if (child->type == clax_track_object) {
            obj = (c_OBJECT *)node->object;
            cobj = (c_OBJECT *)child->object;
            mat_mul( obj->matrix, cobj->matrix, cobj->matrix );
          }
          if (child->type == clax_track_camera) {
            obj = (c_OBJECT *)node->object;
            cam = (c_CAMERA *)child->object;
            mat_mulvec (obj->matrix, &cam->pos, &cam->pos);
          }
          if (child->type == clax_track_cameratgt) {
            obj = (c_OBJECT *)node->object;
            cam = (c_CAMERA *)child->object;
            mat_mulvec (obj->matrix, &cam->target, &cam->target);
          }
          if (child->type == clax_track_light) {
            obj = (c_OBJECT *)node->object;
            light = (c_LIGHT *)child->object;
            mat_mulvec (obj->matrix, &light->pos, &light->pos);
          }
          if (child->type == clax_track_spotlight) {
            obj = (c_OBJECT *)node->object;
            light = (c_LIGHT *)child->object;
            mat_mulvec (obj->matrix, &light->pos, &light->pos);
          }
          if (child->type == clax_track_lighttgt) {
            obj = (c_OBJECT *)node->object;
            light = (c_LIGHT *)child->object;
            mat_mulvec (obj->matrix, &light->target, &light->target);
          }
        }
      }
      if (node->type == clax_track_camera) {
        for (child = node->child; child; child = child->brother) {
          if (child->type == clax_track_light) {
            cam = (c_CAMERA *)node->object;
            light = (c_LIGHT *)child->object;
            vec_add (&cam->pos, &light->pos, &light->pos);
          }
          if (child->type == clax_track_spotlight) {
            cam = (c_CAMERA *)node->object;
            light = (c_LIGHT *)child->object;
            vec_add (&cam->pos, &light->pos, &light->pos);
          }
        }
      }
    }
  }

  /* update camera matrices */
  for (node = clax_scene->keyframer; node; node = node->next)
    if (node->type == clax_track_camera) {
      cam = (c_CAMERA *)node->object;
      cam_update (cam);
    }

  /* do transformation if neccesary */
  if (clax_flags & clax_transform) do_transform ();
  return clax_err_ok;
}

