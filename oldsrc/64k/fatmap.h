#ifndef  __FATMAP_HH__
#define  __FATMAP_HH__

#include "render.h"

#define ZCOR 16777216

extern int textureoffset;
extern bool USE_AFFINE_MAPPING;
extern int m_intensity;

void DrawPkTexturedPoly( c_DVERTEX* vtx, int vertices, long didx, long dudx, long dvdx);

#endif