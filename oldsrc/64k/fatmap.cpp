#include "render.h"
#include "main.h"
#include "fatmap.h"

extern  int  bufferWidth, bufferHeight;

int textureoffset;
int m_intensity;
bool USE_AFFINE_MAPPING;

static c_DVERTEX * max_vtx;                   // Max y vertex (ending vertex)
static c_DVERTEX * start_vtx, * end_vtx;      // First and last vertex in array
static c_DVERTEX * right_vtx, * left_vtx;     // Current right and left vertex

static long right_height, left_height;
static long right_x, right_dxdy, left_x, left_dxdy;

static long left_i, left_didy;
static long left_u, left_dudy;
static long left_v, left_dvdy;

static long _didx, _dudx, _dvdx;

inline long fxCeil(long x)
{
    x +=  0xffff;
    return (x >> 16);
}


inline long imul16(long x, long y)        
{
	__asm
	{
		mov eax,x
		mov edx,y
		imul  edx       
		shrd  eax,edx,16
	}
}

static long i10000=0x10000;
inline long idiv16(long x, long y)
{	
	long result;	
	_asm
	{
		fild [i10000]
		fild x
		fild y
		fdiv
		fmul
		fistp result
		mov eax,result
	}
}

inline int shade_px(int i, int c)
{
	unsigned char *ct = shadetable + (i<<8);
	if (textureoffset >= 256*256) return c;
		return ct[(c>>16)&0xFF]<<16|ct[(c>>8)&0xFF]<<8|ct[c&0xFF];
}

void affine_tline(int *dst, int *zb, int width, long u, long v, long w,
		   long du, long dv, long dw, int *tex)
{

   int tx,ty;

   while (width--)
   {
	if (w>*zb)
	{		 	
		tx=(u & 0xFF0000)>>17;
		ty=(v & 0xFF0000)>>17;

			*dst=shade_px(m_intensity,
//				*(tex + ((u & 0xFF0000)>>16) + ((v & 0xFF0000)>>8))
				*(tex + tx + (ty<<8))
				);

		*zb=w;	
	}
    u+=du;
    v+=dv;
	w+=dw;
	dst++;
	zb++;
   }

}

void _inner(int * dst, int width, long i, long u, long v)
{			
	int *zb=dst+SIZE;	
	affine_tline(dst, zb, width, u, v, i, _dudx, _dvdx, _didx, texture + textureoffset);
};


#define SUB_DIVIDE_SHIFT 4
#define SUB_DIVIDE_SIZE  (1<<SUB_DIVIDE_SHIFT)

void pk_inner(int * dst, int width, long i, long u, long v)
{
	
	int k;
	int *zb=dst+SIZE;
	long end_i;

	//check z-buffer
	//end_i= i + imul16(_didx,width);
	//if ( (i < *zb)  && ( end_i < *(zb+width))) return;

        long du, dv;
        long U, V, U1, V1, U2, V2;
		long iZ;
		long w,dw;
		int x;

		w=i;
		dw=_didx;

		//start of span		
		iZ=idiv16(ZCOR,i);
		U1=imul16(u,iZ);
		V1=imul16(v,iZ);

                while (width>=SUB_DIVIDE_SIZE)
                {
						i+=_didx<<SUB_DIVIDE_SHIFT;
						u+=_dudx<<SUB_DIVIDE_SHIFT;
						v+=_dvdx<<SUB_DIVIDE_SHIFT;
						
						//end of span
						iZ=idiv16(ZCOR,i);						
						U2=imul16(u,iZ);
						V2=imul16(v,iZ);

                        du = (U2 - U1) >> SUB_DIVIDE_SHIFT;
                        dv = (V2 - V1) >> SUB_DIVIDE_SHIFT;
						
                        U=U1;
                        V=V1;

                        x = SUB_DIVIDE_SIZE;

						affine_tline(dst, zb, x, U, V, w, du, dv, dw, texture + textureoffset);

						dst+=x; zb+=x; w+=dw<<SUB_DIVIDE_SHIFT;  //correct some vars

						//start of the next span
						U1=U2;
						V1=V2;
					
                        width-=SUB_DIVIDE_SIZE;
                }

                if (width>0)
                {
                        u+=imul16(_dudx, width<<16);
                        v+=imul16(_dvdx, width<<16);
                        i+=imul16(_didx, width<<16);
						
						iZ=idiv16(ZCOR,i); 						
						U2=imul16(u,iZ);
						V2=imul16(v,iZ);

						du = imul16((U2 - U1), invtable[width]);
                        dv = imul16((V2 - V1), invtable[width]); 						

                        U=U1;
                        V=V1;

						affine_tline(dst, zb, width, U, V, w, du, dv, dw, texture + textureoffset);
                }
			
}


static void pkRightSection( void )
{
    // Walk backwards trough the vertex array

    c_DVERTEX * v2, * v1 = right_vtx;
    if(right_vtx > start_vtx) v2 = right_vtx-1;     
    else                      v2 = end_vtx;         // Wrap to end of array
    right_vtx = v2;

    // v1 = top vertex
    // v2 = bottom vertex 

    // Calculate number of scanlines in this section

    right_height = fxCeil(v2->y) - fxCeil(v1->y);
    if(right_height <= 0) return;

    long height = v2->y - v1->y;
    right_dxdy  = idiv16(v2->x - v1->x, height);
     
	//float invHeight = 65536.0f / (float)(v2->y - v1->y);
    //long height = v2->y - v1->y;
    //right_dxdy  = (long)((float)(v2->x - v1->x) * invHeight);

    // Prestep initial values
    long prestep = (fxCeil(v1->y) << 16) - v1->y;
    right_x = v1->x + imul16(prestep, right_dxdy);
}

static void pkLeftSection( void )
{
    // Walk forward trough the vertex array

    c_DVERTEX * v2, * v1 = left_vtx;
    if(left_vtx < end_vtx) v2 = left_vtx+1;
    else                   v2 = start_vtx;      // Wrap to start of array
    left_vtx = v2;

    // v1 = top vertex
    // v2 = bottom vertex 

    // Calculate number of scanlines in this section

    left_height = fxCeil(v2->y) - fxCeil(v1->y);
    if(left_height <= 0) return;

    long height = v2->y - v1->y;  

    left_dxdy = idiv16(v2->x - v1->x, height);
    left_didy = idiv16(v2->i - v1->i, height);  
	left_dudy = idiv16(v2->u - v1->u, height);  
	left_dvdy = idiv16(v2->v - v1->v, height);

    // Prestep initial values
    long prestep = (fxCeil(v1->y) << 16) - v1->y;

    left_x = v1->x + imul16(prestep, left_dxdy);
    left_i = v1->i + imul16(prestep, left_didy);
	left_u = v1->u + imul16(prestep, left_dudy);    
	left_v = v1->v + imul16(prestep, left_dvdy);
}


void DrawPkTexturedPoly( c_DVERTEX* vtx, int vertices, long didx, long dudx, long dvdx)
{
    start_vtx = vtx;        // First vertex in array

    // Search trough the vtx array to find min y, max y
    // and the location of these structures.

    c_DVERTEX * min_vtx = vtx;
    max_vtx = vtx;

    long min_y = vtx->y;
    long max_y = vtx->y;

    vtx++;

    for(int n=1; n<vertices; n++) {
        if(vtx->y < min_y) {
            min_y = vtx->y;
            min_vtx = vtx;
        }
        else
        if(vtx->y > max_y) {
            max_y = vtx->y;
            max_vtx = vtx;
        }
        vtx++;
    }

    // OK, now we know where in the array we should start and
    // where to end while scanning the edges of the polygon

    left_vtx  = min_vtx;    // Left side starting vertex
    right_vtx = min_vtx;    // Right side starting vertex
    end_vtx   = vtx-1;      // Last vertex in array

    // Search for the first usable right section

    do {
        if(right_vtx == max_vtx) return;
        pkRightSection();
    } while(right_height <= 0);

    // Search for the first usable left section

    do {
        if(left_vtx == max_vtx) return;
        pkLeftSection();
    } while(left_height <= 0);
   
	int *  destptr_i = buffer + fxCeil(min_y) * WIDTH;

    _didx = didx;
	_dudx = dudx;
	_dvdx = dvdx;

    for ( ; ; )
    {
        long x1 = fxCeil(left_x);
        long width = fxCeil(right_x) - x1;

        if ( width > 0 ) {

            // Prestep
            long prestep = (x1 << 16) - left_x;

            long i = left_i + imul16( prestep, didx );
			long u = left_u + imul16( prestep, dudx );
			long v = left_v + imul16( prestep, dvdx );

            if (!USE_AFFINE_MAPPING)
			pk_inner( destptr_i + x1, width, i, u, v);
			else
			_inner( destptr_i + x1, width, i, u, v);

        }

        destptr_i  += bufferWidth;

        // Scan the right side

        if(--right_height <= 0) {               // End of this section?
            do {
                if(right_vtx == max_vtx) return;
                pkRightSection();
            } while(right_height <= 0);
        }
        else 
            right_x += right_dxdy;

        // Scan the left side

        if(--left_height <= 0) {                // End of this section?
            do {
                if(left_vtx == max_vtx) return;
                pkLeftSection();
            } while(left_height <= 0);
        }
        else {
            left_x += left_dxdy;
            left_i += left_didy;
			left_u += left_dudy;
			left_v += left_dvdy;
        }
    }
}

