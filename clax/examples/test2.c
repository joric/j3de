#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "clax.h"

void main ()
{
  c_SCENE  *scene;

  clax_init (clax_transform | clax_hierarchy | clax_domorph);
  clax_allocscene (&scene);
  printf ("world: %d\n", clax_loadworld ("1.clx",scene));
  printf ("motion:%d\n", clax_loadmotion("1.clx",scene));
  clax_setactive (scene);
  clax_savescene ("2.clx",scene);
  clax_print_world ();
  clax_print_keyframer ();
  clax_free_scene (scene);
}

