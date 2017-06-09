#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "system_w32.h"
#include "rgbvideo_w32.h"
#else
#include <tinyfile.h>
#include <tinyheap.h>
#include <tinymath.h>
#include "system.h"
#include "rgbvideo.h"
#include "xmplayer.h"
#include "module.h"
#define MODULE module
#endif

char *p;
int i, j;
int sync, _sync, bebe, real_time, _real_time, counter;

#include "random.h"
#include "print.h"
//#include "particle.h"
#include "font.h"
#include "greets.h"
//#include "bump.h"
//#include "rad_blur.h"
//#include "poll.h"
//#include "meta.h"
#include "voxels.h"

#include <time.h>

void update()
{
	memset(buffer, 0, MAXX * MAXY * 4);
	real_time++;
	counter=real_time;
	poll_msg();
	greets();
	textcolor(0x00ff00);
	textpos(5, 5);
	char tmpbuf[128];
	_strtime(tmpbuf);
	print(tmpbuf);
	outtext(50, 50, "HELLO WORLD", 0.5f, 0xffffff, PM_ALIGN_LEFT);
	copybuffer(buffer);
//	voxels();
}

int main(int argc, char **argv)
{
	initvoxels();
	real_time=1;
	set_update_function(update);
	main_loop();
	return 0;
}
