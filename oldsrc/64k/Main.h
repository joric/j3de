#ifndef _MAIN_H_
#define _MAIN_H_

#include <string.h>
#include <stdio.h>
#include <math.h>

#define WIDTH 320
#define HEIGHT 240

#define SIZE WIDTH*HEIGHT

#define timerrate 1000 //per second

void Init();
void UpdateFrame();
void CloseAll();

extern int m_shade;
extern float curFrame;
extern bool m_console;
extern int * outBuffer;
extern int buffer[SIZE*4];
extern int texture[256*256*5];
extern long invtable[256];
extern long count;
extern unsigned char shadetable[256*256]; 
extern int vfaces;
extern int afaces;
extern long m_counter;
extern long m_rendermode;
extern int obj_count;
extern float zfar,zmax,zmin;
extern int m_fx;

typedef struct Vertex
{
	float x,y,z,u,v;
}Vertex;

#endif /* _MAIN_H_ */

