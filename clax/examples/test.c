#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "clax.h"

void mode(int mode);
#pragma aux mode = \
 "int 10h" \
 parm [eax];

void main ()
{
  c_SCENE  *scene;
  c_OBJECT *obj;
  w_NODE   *node;
  k_NODE   *kn;
  c_LIGHT  *lit;
  c_CAMERA *cam;
  c_VECTOR  vec;
  c_OBJECT *o1;
  char     *myscreen;
  int       err;
  int       i, x, y;
  float     frames, frame;

  myscreen = (char *)malloc(65536);
  clax_init (clax_transform | clax_hierarchy | clax_domorph);
  clax_alloc_scene (&scene);
  err = clax_load_world ("1.3ds",scene);
  if (err != clax_err_ok)
    printf("error(after):%s\n",clax_geterror(err));
      else printf("ok!\n");

  err = clax_load_motion ("1.3ds",scene);
  if (err != clax_err_ok)
    printf("error(after): %s\n",clax_geterror(err));
      else printf("ok!\n");
  clax_setactive_scene (scene);

  clax_byname ("Camera01", &node);
  if (node) cam = (c_CAMERA *)node->object; else {
    clax_byname("rolli",&node);
    cam = (c_CAMERA *)node->object;
  }
  cam->sizeX = 320.0;
  cam->sizeY = 200.0;
  cam->aspectratio = 0.75;
  mode(0x13);
  wah:
  clax_getframes(&frame, &frames);
  clax_setactive_camera (cam);
  for (;frame<frames;frame += 1) {
    if (inp (0x60)==1) {
      mode(0x03);
      exit (0);
    }
    clax_setframe(frame);
    clax_update();
    memset(myscreen,0,64000);

    for (node = scene->world; node; node=node->next) {
      if (node->type == clax_obj_object) {
        obj = (c_OBJECT *)node->object;
        if (((obj->flags & clax_obj_hidden) == 0)&&
             (obj->flags & clax_obj_chidden) == 0)
        for (i=0;i<obj->numverts;i++) {
//          mat_mulvec (cam->matrix, &obj->vertices[i].pvert, &vec);
          vec_copy(&obj->vertices[i].pvert, &vec);
          x = 160+vec.x*cam->perspX/vec.z;
          y = 100-vec.y*cam->perspY/vec.z;
          if (x>=0 && x<320 && y>=0 && y<200 && vec.z > 1.0)
            myscreen[y*320+x] = 15;
        }
      } else if (node->type == clax_obj_light) {
        lit = (c_LIGHT *)node->object;
        mat_mulvec (cam->matrix, &lit->pos, &vec);
        x = 160+vec.x*cam->perspX/vec.z;
        y = 100-vec.y*cam->perspY/vec.z;
        if (x>=0 && x<320 && y>=0 && y<200 && vec.z > 1.0) {
          myscreen[y*320+x] = 9;
          myscreen[y*320+x+1] = 9;
          myscreen[y*320+x+320] = 9;
          myscreen[y*320+x+321] = 9;
        }
      }
    }
    memcpy((char *)0xA0000, myscreen, 64000);
  }
  goto wah;
  mode(0x03);
  printf("freeing world\n");
  clax_free_world(scene);
  printf("freeing motion\n");
  clax_free_motion(scene);
  printf("OK!\n");
}

