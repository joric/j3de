
#include <math.h>
#include <stdio.h>

#include "clax.h"
#include "main.h"

#define MAXLIGHTS  64

typedef struct {
  c_VECTOR   pos;
  float      innerRange, outerRange;
  float      invRange;
  float      intensity;
  int        flags;
} c_DLIGHT;

c_DLIGHT   lightList[MAXLIGHTS];
int        nLights;

void
initLights( c_SCENE* scene, c_MATRIX camMat )
{
  w_NODE*    node;
  c_LIGHT*   light;

  nLights = 0;

  for ( node = scene->world; node; node = node->next ) {
    if ( node->type == clax_obj_light ) {
      light = (c_LIGHT *)node->object;

      mat_mulvec( camMat, &light->pos, &lightList[nLights].pos );

      lightList[nLights].intensity = 0.301f * light->color.r +
                                     0.586f * light->color.g +
                                     0.113f * light->color.b;

      lightList[nLights].innerRange = light->innerrange;
      lightList[nLights].outerRange = light->outerrange;
      lightList[nLights].invRange = 1.0f / (light->outerrange - light->innerrange);

      lightList[nLights].flags = light->flags;
      
      nLights++;
      if ( nLights >= MAXLIGHTS )
        break;

    }

	//add default light and attach it to camera	
  }
}



float
calcLight( c_VECTOR* vert, c_VECTOR* norm )
{
  c_DLIGHT*  light;
  c_VECTOR   vec;
  float      dot, lenght, intensity;
  
  intensity = 0.0;
/*  
  for ( int i = 0; i < nLights; i++ ) 
  {
    light = &lightList[i];
    vec_sub( vert, &light->pos, &vec );
    lenght = vec_length( &vec );                                     // light -> vertex

    if ( (light->flags & clax_light_attenuate) && (lenght > light->outerRange) ) {
      // light has dimmed out before it hits this vertex
      continue;
  }

    vec_scale( &vec, 1.0f / lenght, &vec );                           // normalize
    dot = vec_dotunit( &vec, norm );

    if ( (light->flags & clax_light_attenuate) && (lenght > light->innerRange) )
      dot *= (light->outerRange - lenght) * light->invRange;         // attenuate

    if ( dot > 0.0 )
      intensity += light->intensity * dot;
  }
*/
  intensity=norm->z;

  if ( intensity < 0 )
    intensity = 0;

  if ( intensity > 1.0 )
    intensity = 1.0;

  return intensity;    
}
