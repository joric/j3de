#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include <time.h>
#include "res/resource.h"

#include "main.h"
#include "render.h"
#include "clax.h"
#include "print.h"

#include "meta.h"

int buffer[SIZE*4];   //e.g linked backbuffer, z-buffer, a-buffer
int texture[256*256*5];  // linked textures

long invtable[256]; // inverse span lenght table for pk_inner

unsigned char shadetable[256*256]; // shade table

int m_shade;
int obj_count;
float zmax,zmin;

static int index;
float frame, frames, zfar, curFrame, fps;
int faces, vfaces, afaces,lastcount;
long count;
bool m_console=true;
int m_fx;

c_SCENE*   scene;

#pragma pack(1)
struct nlTGAHeader
{
	char text_size;
	char map_type;
	char data_type;
	short map_org;
	short map_length;
	char cmap_bits;
	short x_offset;
	short y_offset;
	short width;
	short height;
	char data_bits;
	char im_type;
};
#pragma pack()


void blur(int *buf)
{
_asm{
		emms
        mov edi, buf
        xor ecx,ecx
        pxor mm0,mm0
	    mov edx,SIZE
lp:

        movd mm1,[ecx*4+edi]
        movd mm2,[ecx*4+4+edi]
        punpcklbw mm1,mm0
        punpcklbw mm2,mm0

        movd mm3,[ecx*4+(WIDTH*4*2)+4+edi]
        movd mm4,[ecx*4+(WIDTH*4*2)+8+edi]
        punpcklbw mm3,mm0
        punpcklbw mm4,mm0

        paddw mm3,mm4
        paddw mm1,mm2
        paddw mm1,mm3
        psrlw mm1,2    //mm1 div 4
        packuswb mm1,mm0
        movd [ecx*4+(WIDTH*4)+4+edi],mm1   
        inc ecx
        dec edx
        jnz lp
        emms
}
}

#define TWIDTH 256
#define TSIZE 256*256

void blurtx(int *buf)
{
_asm{
		emms
        mov edi, buf
        xor ecx,ecx
        pxor mm0,mm0
	    mov edx,TSIZE
lp1:

        movd mm1,[ecx*4+edi]
        movd mm2,[ecx*4+4+edi]
        punpcklbw mm1,mm0
        punpcklbw mm2,mm0

        movd mm3,[ecx*4+(TWIDTH*4*2)+4+edi]
        movd mm4,[ecx*4+(TWIDTH*4*2)+8+edi]
        punpcklbw mm3,mm0
        punpcklbw mm4,mm0

        paddw mm3,mm4
        paddw mm1,mm2
        paddw mm1,mm3
        psrlw mm1,2    //mm1 div 4
        packuswb mm1,mm0
        movd [ecx*4+(TWIDTH*4)+4+edi],mm1   
        inc ecx
        dec edx
        jnz lp1
        emms
}
}

void LoadTexture(char *filename, int* texture)
{
	FILE *fp;
	int width,height,i;
	unsigned char r,g,b;
	unsigned char *data;

	if (fp=fopen(filename,"rb"))
	{
	nlTGAHeader header;
	fread((void*)&header,sizeof(nlTGAHeader),1,fp);		
	fseek(fp,header.text_size, SEEK_CUR);
	data=(unsigned char*)malloc(256*256*3);
	fread(data,256*256*3-1,1,fp);
	fclose(fp);
		for (i=0; i < 256*256; i++)
		{
			b = *data++;
			g = *data++;
			r = *data++;
			texture[i]=r<<16|g<<8|b;
		}
	free(data);

	}

}


#define PI 3.14159265358979323846
#define ROUND(a) 	(((a)<0)?(int)((a)-0.5):(int)((a)+0.5))
#define NORMALIZE(n) ((n<0) ? 0 : ((n>255) ? 255 : n));

#define ROWS 	256
#define COLS 	256
#define N 8

//globals

float C[N][N];
float Ct[N][N];
int Quantum[N][N];
int InputRunLength;
int OutputRunLength;

float input_array[N][N];
float output_array[N][N];


struct zigzag
{
	int row;
	int col;
};

zigzag ZigZag[N*N];

int ZigZag8[8*8]=
{
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};


void Initalise(int quality)
{
	int i,j;

//subsampling matrix
	for (i=0;i<N;i++) 
		for (j=0;j<N;j++)
			Quantum[i][j] = 16 + ( 1 + i + j ) * quality;

// define DCT and transposed DCT matrix:
// C(i,j) = 1/sqrt(N), i=0 
// C(i,j) = sqrt(2/N) * cos(((2*j+1)*i*PI)/(2*N)), i<>0

	for(j=0;j<N;j++)
	{
		C[0][j]= 1.0 / sqrt((float) N);
		Ct[j][0]=C[0][j];	
	}

	for (i=1;i<N;i++)
		for (j=0;j<N;j++)
	      	{
	      		C[i][j]= sqrt(2.0/N) * cos( ((2*j+1)*i*PI)/(2.0*N) );
				Ct[j][i]=C[i][j];
	      	}

	int x,y,dx,dy;
	x,y,dy=0; dx=1;

	for (i=0;i<N*N;i++)
	{	
		ZigZag[i].row = i%N;//ZigZag8[i]%N;
		ZigZag[i].col = i/N;//ZigZag8[i]/N;		
	}

}

void MatrixMul(float input[N][N], float output[N][N],
		float C[N][N],float Ct[N][N])
{
	float temp[N][N];
	float temp1;
	int i,j,k;

	//output = C * input * Ct;

	for (i=0;i<N;i++)
		for (j=0;j<N;j++)		
		{
			temp[i][j]=0.0;
			for(k=0;k<N;k++)	
				temp[i][j]+=input[i][k] * Ct[k][j];
		}

	for (i=0;i<N;i++)
		for (j=0;j<N;j++)			
		{
			temp1=0.0;
			for (k=0;k<N;k++)
				temp1+=C[i][k]*temp[k][j];
					output[i][j]=temp1;
		}		
}


void iDCT(float input[N][N],float output[N][N])
{
	int i,j,row,col;
	float result;
	for (i=0;i<(N*N);i++)
	{
		row=ZigZag[i].row;
		col=ZigZag[i].col;		
		input[row][col] = (signed char)input[row][col] * Quantum[row][col];
	}

	MatrixMul(input,output,Ct,C);

	for (i=0;i<N;i++)
		for (j=0;j<N;j++)		
			output[i][j]=NORMALIZE(output[i][j]);

}


void Read_BOX(int x, int y, byte *input, float array[N][N], int layer)
{
	int i,j;
	for (i=0;i<N;i++)
		for (j=0;j<N;j++)
			array[i][j] = input[ ( (x+i) + (y+j)*256 )*4+layer ];
}

void Write_BOX(int x, int y, byte *output, float array[N][N], int layer)
{
	int i,j;
	for (i=0;i<N;i++)
		for (j=0;j<N;j++)
			output[ ( (x+i) + (y+j)*256 )*4+layer ] =  array[i][j];
}


void DeCompressTexture(int *input_texture, int *output_texture, int quality)
{
	int i,j,k,layer;

	byte *input=(byte*)input_texture;
	byte *output=(byte*)output_texture;
	Initalise(quality);
	
	for (layer=0;layer<3;layer++)
	for (i=0;i<256;i+=N)
		for (j=0;j<256;j+=N)
		{
			Read_BOX(i, j, input, input_array, layer);
			iDCT(input_array, output_array);
			Write_BOX(i, j, output, output_array,layer);
		}	

}


void LoadIntTexture(int ID, int* texture)
{
	
    byte* lpRes;
    HANDLE hResInfo, hRes; 
    // Find the resource. 
    hResInfo=FindResource(NULL, MAKEINTRESOURCE(ID), "TEXTURES");
	hRes = LoadResource(NULL, (HRSRC__ *)hResInfo);		
	lpRes=(byte*)LockResource(hRes);
	int width,height,i;
	byte r,g,b;
	byte *data = lpRes+sizeof(nlTGAHeader);

		for (i=0; i < 256*256; i++)
		{
			b = *data++;
			g = *data++;
			r = *data++;
			texture[i]=r<<16|g<<8|b;
		}
	UnlockResource(hRes);
	DeCompressTexture(texture, texture, 10);
	for (i=0;i<2;i++) blurtx(texture);
}


void FLARE(int *texture)
{
	int i,j,dx,dy;	
	unsigned char c,k,rays[256];
	float r,a,v,w,rx,light,glow;	
	for (i=0;i<256;i++) rays[i]=rand()%255;

	light=-0.02;
	glow=-0.015;

	  for (i=0;i<256;i++)
	  for (j=0;j<256;j++)
	  {		
		dx=i-128; dy=j-128;
		r=sqrt(dx*dx+dy*dy); 
		v=(atan2(dx,dy)+PI)*128.0/PI;
		a=exp(light*r*r)+exp(glow*r);	

		k=v; v=(rays[k]+(rays[k+1]-rays[k])*(v-k));
		//a+=pow(v/256.0,5.0) / (1 + r*0.4);

		rx =fabs(r - 40)/2;	
		if(rx>0 && rx<=1) 
		a+=(1-rx*rx*(3-2*rx))/20;
        
		if (a>1) a=1;
		c=a*255;
		texture[i/2+j/2*256]=c<<16|c<<8|c;
	  }	
}

void Init()
{
	c_CAMERA*  animCam;
	c_OBJECT*  obj;
	w_NODE*    node;
	int        err;
	char*      file;
	c_VIEWPORT   viewport;
	int i,j,k;

	//memset(texture,0,256*256*4);
	//memcpy(texture,hRes,256*256*3);

	//LoadTexture("textures/raw_dct.tga",texture+256*256*0);
	LoadIntTexture(IDR_TEXTURES1,texture);
	FLARE(texture+256*256);	

//	LoadTexture("textures/tex00001.tga",texture+256*256*1);
//	LoadTexture("textures/tex00002.tga",texture+256*256*2);
//	LoadTexture("textures/tex00003.tga",texture+256*256*3);
//	LoadTexture("textures/tex00004.tga",texture+256*256*4);
//	LoadTexture("textures/tex00005.tga",texture+256*256*5);
//	LoadTexture("textures/tex00006.tga",texture+256*256*6);
//	LoadTexture("textures/tex00007.tga",texture+256*256*7);

	zfar = 8000.0;

    byte* lpRes;
    HANDLE hResInfo, hRes; 
    // Find the resource. 
    hResInfo=FindResource(NULL, MAKEINTRESOURCE(IDR_SCENES1), "SCENES");
	hRes = LoadResource(NULL, (HRSRC__ *)hResInfo);		
	lpRes=(byte*)LockResource(hRes);

	char s[256];
	GetTempPath( 256,s );

	file=strcat(s,"1.3ds");

	FILE* fp = fopen ( file, "wb" );
	fwrite(lpRes,SizeofResource(NULL,(HRSRC__ *)hResInfo),1,fp);
	
	fclose(fp);

	//file = "scenes/1.3ds";
  
	clax_init (clax_transform | clax_hierarchy | clax_domorph );
	clax_alloc_scene( &scene );

	// Load mesh
	err = clax_load_world( file, scene );
	// load keyframes
	err = clax_load_motion( file, scene );

	// *** INIT VIEWPORT ***

	// count faces
	faces = 0;

	for ( node = scene->world; node; node = node->next ) {
		if (node->type == clax_obj_object) {
			obj = (c_OBJECT *)node->object;
			faces += obj->numfaces;
		}
	}	

	err = viewportOpen( WIDTH, HEIGHT, MAXFACES, &viewport );

	viewportSetactive( &viewport );
  
	memset( viewport.outBuffer, 1, (viewport.iWidth * viewport.iHeight) );
	memset( viewport.trBuffer, 1, (viewport.iWidth * viewport.iHeight) );

	clax_setactive_scene( scene );

	// search camera
	animCam = 0;
	for (node = scene->world; node; node=node->next) {
		if (node->type == clax_obj_camera) {
			animCam = (c_CAMERA *)node->object;
			break;
		}
	}

	animCam->xSize = WIDTH;
	animCam->ySize = HEIGHT;
	animCam->pixelHeight = 1;
	animCam->znear = 1.0;
	animCam->zfar  = zfar;
	clax_setactive_camera( animCam );

	clax_setactive_camera( animCam );
	cam_update( animCam );

	clax_getframes( &frame, &frames );

	curFrame = frame;

	zmin=1000000;
	zmax=-zmin;

	for (i=1;i<256;i++) invtable[i]=(1<<16)/i;

	for (i=0;i<256*256;i++) shadetable[i]=((i>>8)*(i&0xFF))>>8;


//	clax_print_world ();
	count=0;
	InitTables();
}

float _persent(float x, float y)
{
	if (y!=0) return 100*x/y;
	else return 0;
}


void UpdateFrame()
{


	int i,j,k;
	long oldsec,sec;

//	j=0;for (i=0;i<SIZE;i++) { if (i%WIDTH) j=!j; buffer[i]=-j&0x555555;}

	memset(buffer,0,SIZE*4); //clear buffer
	memset(buffer+SIZE,0,SIZE*4); //clear zbuffer

		vfaces=0;
		afaces=0;
		clax_setframe( curFrame );
		clax_update();
		flushPolys(); 
					
if (m_rendermode==1)
{
// ----motion blur--------------------------
	//blur(buffer+SIZE*2);
	int *src = buffer+SIZE+SIZE;
	int *dest = buffer; i=SIZE;
	while (i--) *dest++ = ((*dest & 0xFEFEFE) + (*src++ & 0xFEFEFE))>>1; 
	memcpy(buffer+SIZE*2,buffer,SIZE*4);
// ----eof motion blur----------------------
}


//-----------console-----------		
	if (m_console)
	{
		textcolor(0x00FF00);
		textpos(5,5);
		print("FACES:");printn(vfaces);print("/");printn(faces);
		print("[");printn(_persent(vfaces,faces));print("%]");

		textpos(5,5+8);
		print("AFFINE:");printn(afaces);print("/");printn(vfaces);
		print("[");printn(_persent(afaces,vfaces));print("%]");

    
		//fps
		fps=m_counter - lastcount;
		lastcount=m_counter;	

		textpos(5,5+8+8);
		//print("TICKS:"); printfl(fps,2,1);

		print("TESSELATED:"); printn(triangles_tesselated);
		triangles_tesselated=0;
	
		textpos(5,HEIGHT-8);
		print("FRAME:"); printn(curFrame);print("/"); printn(frames);
		print("[");printn(_persent(curFrame,frames));print("%]");
		
		j=(WIDTH - TEXT_X)/6;
		k=j*_persent(curFrame,frames)/100;
		for (i=0;i<j;i++) { if (i!=k) print("-"); else print ("="); }		
/*
		textpos(5,5+8+8);
		print("ZMAX:"); printfl(zmax,5,10);
		textpos(5,5+8+8+8);
		print("ZMIN:"); printfl(zmin,5,10);
*/
		char tmpbuf[128]; _tzset();	_strtime( tmpbuf ); 
		textpos(WIDTH-6*9,5);print(tmpbuf);    

	}
// ----------eof console---------

	float framerate=30.0;
	
	curFrame=(float)m_counter*framerate/timerrate;

	if (m_rendermode==2) curFrame=250;
	if( curFrame > frames ) { curFrame = frame; m_counter=0;}

//	count++;
}

void CloseAll()
{
	clax_free_world( scene );
	clax_free_motion( scene );
}