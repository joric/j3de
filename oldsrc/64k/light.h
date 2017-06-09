#ifndef  __MOE_LIGHT_HH__
#define  __MOE_LIGHT_HH__

#include "clax.h"

void   initLights( c_SCENE* scene, c_MATRIX camMat );
float  calcLight( c_VECTOR* vert, c_VECTOR* norm );

#endif