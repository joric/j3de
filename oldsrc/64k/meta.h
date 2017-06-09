#include "main.h"
#include "clax.h"

void InitTables(void);
void NextFrame(float time);

#define num_meta_balls 5
#define max_triangles 2000

#define X_LEFT  (-100.0f)
#define X_RIGHT 100.0f
#define Y_LEFT  (-100.0f)
#define Y_RIGHT 100.0f
#define Z_LEFT  (-100.0f)
#define Z_RIGHT 100.0f

const float charge=800.50f;

#define RES_X   16
#define RES_Y   16
#define RES_Z   16

extern c_VERTEX mv[max_triangles*3];
extern c_FACE mf[max_triangles];
extern float map(float x);

extern int triangles_tesselated;
