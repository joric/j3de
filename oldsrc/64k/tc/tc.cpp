#include <Windows.h>
#include <stdio.h>
#include <ddraw.h>
#include <math.h>
#include <mmsystem.h>


#define WIDTH 256
#define HEIGHT 256

	int buffer[256*256];
	int texture[256*256];
	int data[256*256*10];
	int quality;	


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


void DCT(float input[N][N],float output[N][N])
{
	int i,row,col;
	MatrixMul(input,output,C,Ct);

	for (i=0;i<(N*N);i++)
	{
		row=ZigZag[i].row;
		col=ZigZag[i].col;
		output[row][col] = ROUND( output[row][col] / Quantum[row][col] );
		//he-he, Beavis, hi says ROUND ;-)
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

void CompressTexture(int *input_texture, int *output_texture, int quality)
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
			DCT(input_array, output_array);
			Write_BOX(i, j, output, output_array, layer);
		}	
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



void pack(byte *input, byte *output)
{

}






	static LPDIRECTDRAW	m_DDraw = NULL;
	static LPDIRECTDRAWSURFACE m_FrontBuffer = NULL;
	static LPDIRECTDRAWSURFACE m_BackBuffer = NULL;
	static LPDIRECTDRAWCLIPPER m_Clipper = NULL;
	DDPIXELFORMAT PixelFormat;

	RECT rect;
	BOOL ModeFound;

	int m_Width, m_Height, m_Bpp;

	// color space masks
	static unsigned int	m_RedMask, m_RedRShift, m_RedLShift;
	static unsigned int	m_GreenMask, m_GreenRShift, m_GreenLShift;
	static unsigned int	m_BlueMask, m_BlueRShift, m_BlueLShift;

	#define MAX_MODES 64

	// list of avaliable modes
	static struct
	{
		int Width;
		int Height;
		int Bpp;
	} m_ModeList[ MAX_MODES ];
	static int m_NumberModes;


HWND			g_hWnd;
HINSTANCE		g_hInstance;
FARPROC			lpprocAbout;
HICON			hIcon;

// class description
static char				*m_AppName	= "64k";
static char				*m_AppTitle	= "64k";

 
/*
Color32_t *DDraw_GetAddress( void )
{
	return m_VPage;
}
*/
static int GetLowestBit( unsigned int Value )
{
	int Position;

	// sanity check
	if (Value == 0) return 0;

	Position = 0;
	while ( (Value&1) == 0)
	{
		Position++;
		Value = Value>> 1;
	}

	return Position;
}

static int GetHighestBit( unsigned int Value )
{
	int Position;

	// sanity check
	if (Value == 0) return 0;

	Position = 0;
	while ( (Value&0x80000000) == 0)
	{
		Position++;
		Value = Value<< 1;
	}

	return 32 - Position;
}

static HRESULT WINAPI DDraw_DisplayCallback( LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext )
{	
	// copy mode info into our structure
	m_ModeList[ m_NumberModes ].Width = lpDDSurfaceDesc->dwWidth;
	m_ModeList[ m_NumberModes ].Height = lpDDSurfaceDesc->dwHeight;
	m_ModeList[ m_NumberModes ].Bpp = lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
	m_NumberModes++;
	// next mode
	return DDENUMRET_OK;
}
 

int DDraw_Init()
{
	HRESULT hr;
	// create DDraw object
	hr = DirectDrawCreate( NULL, &m_DDraw, NULL );

	// search though all avaliable modes to find
	m_NumberModes = 0;
	hr = m_DDraw->EnumDisplayModes( NULL, NULL, NULL, DDraw_DisplayCallback);
	
	return 0;
}


int DDraw_Open()
{
	HRESULT hr;
	DDSURFACEDESC ddsd;
	DDSCAPS ddcs;
	int i;


	m_DDraw->SetCooperativeLevel( g_hWnd, DDSCL_NORMAL );
	
	// create front and back buffers
	memset( &ddsd, 0, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);


	ddsd.dwFlags = DDSD_CAPS; 


	ddsd.dwBackBufferCount = 1;

	ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_PRIMARYSURFACE;

	
	hr = m_DDraw->CreateSurface( &ddsd, &m_FrontBuffer, NULL );

/*--------*/
	memset( &PixelFormat, 0, sizeof(PixelFormat) );
	PixelFormat.dwSize = sizeof(PixelFormat);
	hr = m_FrontBuffer->GetPixelFormat( &PixelFormat );

	// setup color space shifts/masks
	m_RedMask = PixelFormat.dwRBitMask;
	m_RedRShift = 8 - (GetHighestBit( PixelFormat.dwRBitMask ) - GetLowestBit( PixelFormat.dwRBitMask ));
	m_RedLShift = GetLowestBit( PixelFormat.dwRBitMask );

	m_GreenMask = PixelFormat.dwGBitMask;
	m_GreenRShift = 8 - (GetHighestBit( PixelFormat.dwGBitMask ) - GetLowestBit( PixelFormat.dwGBitMask ));
	m_GreenLShift = GetLowestBit( PixelFormat.dwGBitMask );

	m_BlueMask = PixelFormat.dwBBitMask;
	m_BlueRShift = 8 - (GetHighestBit( PixelFormat.dwBBitMask ) - GetLowestBit( PixelFormat.dwBBitMask ));
	m_BlueLShift = GetLowestBit( PixelFormat.dwBBitMask );
/*----------*/

	// get back buffer
	memset( &ddcs, 0, sizeof(ddcs) );
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.dwWidth = m_Width;
	ddsd.dwHeight = m_Height;

	ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN;
	hr = m_DDraw->CreateSurface( &ddsd, &m_BackBuffer, NULL );
	// create clipper
	hr = m_DDraw->CreateClipper( 0, &m_Clipper, NULL);
	// attach clipper to window
	hr = m_Clipper->SetHWnd( 0, g_hWnd );
	// attach clipper to front buffer
	hr = m_FrontBuffer->SetClipper( m_Clipper );
		
	return 0;
	
}


void DDraw_Close( void )
{
	// release back buffer
	if (m_BackBuffer)
	{
		m_BackBuffer->Release();
		m_BackBuffer = NULL;
	}

	// relese front buffer
	if (m_FrontBuffer)
	{
		m_FrontBuffer->Release();
		m_FrontBuffer = NULL;
	}

	// release DDraw
	if (m_DDraw)
	{
		// reset coop
		m_DDraw->SetCooperativeLevel( g_hWnd, DDSCL_NORMAL );
		m_DDraw->RestoreDisplayMode();
		m_DDraw->Release();
		m_DDraw = NULL;
	}
}

void DDraw_Flip( void )
{
	
	HRESULT hr;
	DDSURFACEDESC ddsd;	
	RECT rect;
	
	unsigned char *Dest, *Src;	
	unsigned char *Scanline;

	int Pitch=ddsd.lPitch;
	int x, y;
	unsigned r,g,b;

	memset( &ddsd, 0, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);
	hr = m_BackBuffer->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	if (!FAILED(hr))
	{

		// setup pointers
		Dest = (unsigned char *)ddsd.lpSurface;		
		Src = (unsigned char*)buffer;

		/*
		// for all scanlines
		for (y=0; y < m_Height; y++)
		{
			// for all pixels in the scan
			memcpy(Dest,Src,m_Width*m_Bpp/8);			
			Dest += ddsd.lPitch;
			Src += m_Width*m_Bpp/8;
		}
		*/

/*-----flip-----*/

	int m_DesktopBpp = PixelFormat.dwRGBBitCount;

	Scanline = (unsigned char *)ddsd.lpSurface;

	if (m_DesktopBpp!=32)
	{
		for (y=0; y < m_Height; y++)
		{
			Dest = Scanline + ddsd.lPitch*m_Height;
			// for all pixels in the scan
			for (x=0; x < m_Width; x++)
			{
				// convert 888 to whatever format the display is in
				b = (((*Src++ >> m_BlueRShift) << m_BlueLShift) & m_BlueMask);
				g = (((*Src++ >> m_GreenRShift) << m_GreenLShift) & m_GreenMask);				
				r = (((*Src++ >> m_RedRShift) << m_RedLShift) & m_RedMask);
				*Src++;

				// write to back buffer
				
				// ugly data cast
				if (m_DesktopBpp == 16)
				{
					*((unsigned short *)Dest) = r+g+b;
					Dest += 2;
				}
				else if (m_DesktopBpp == 24)
				{
					*((unsigned int *)Dest) = r+g+b;
					Dest+=3;
				}
				else if (m_DesktopBpp == 32)
				{
					*((unsigned int *)Dest) = r+g+b;
					Dest += 4;
				}
			}

			Scanline -= ddsd.lPitch;
		}
	}
	else
	{
		Dest = Scanline;
		for (y=0; y < m_Height; y++)
		{
			memcpy(Dest, Src, WIDTH*4);
			Src+=WIDTH*4;
			Dest+=ddsd.lPitch;
		}		
	}
/*------------*/
		
		m_BackBuffer->Unlock(NULL);		
	}

	// page flip it

	GetClientRect(g_hWnd, &rect);
    POINT pt; pt.x = pt.y = 0;
    ClientToScreen( g_hWnd, &pt );
    OffsetRect(&rect, pt.x, pt.y);

		hr=m_FrontBuffer->Blt( &rect, m_BackBuffer, NULL, DDBLT_WAIT, NULL);

	if (hr==DDERR_SURFACELOST) { m_FrontBuffer->Restore(); m_BackBuffer->Restore();}
	    
}


BOOL AdjustWindow (HWND hwnd)
{
    RECT    rc;
    int     w, h;
    int     wScreen, hScreen, xNew, yNew;
    HDC     hdc;

	rc.top=0; 
	rc.left=0;
	rc.right=m_Width;
	rc.bottom=m_Height;

    w = rc.right - rc.left;
    h = rc.bottom - rc.top;

    hdc = GetDC (hwnd);
    wScreen = GetDeviceCaps (hdc, HORZRES);
    hScreen = GetDeviceCaps (hdc, VERTRES);
    ReleaseDC (hwnd, hdc);

    xNew = wScreen/2 - w/2;
    yNew = hScreen/2 - h/2;

    AdjustWindowRectEx(&rc, GetWindowLong(g_hWnd, GWL_STYLE),
			 GetMenu(g_hWnd) != NULL, GetWindowLong(g_hWnd, GWL_EXSTYLE));

    SetWindowPos(g_hWnd, NULL, xNew, yNew, rc.right-rc.left, rc.bottom-rc.top, SWP_NOZORDER|SWP_NOACTIVATE);

	return 0;
}


static long CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	
		case WM_MOVING:	
			DDraw_Flip();		
		break;

		case WM_DESTROY:
			PostQuitMessage(-1);
			break;

		case WM_PAINT:
			DDraw_Flip;
			DefWindowProc(hWnd,msg,wParam,lParam);
			break;
		
		case WM_KEYDOWN:		
			switch( wParam )
			{				
				case VK_ESCAPE:
					PostQuitMessage(-1);
					break;				
			}
			break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


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

void SaveTexture(char *filename, int *texture)
{
	FILE *fp;
	int i,c;
	unsigned char r,g,b;
	unsigned char *data;

	fp=fopen(filename,"wb");
	data=(unsigned char*)malloc(256*256*3);	
	unsigned char *d=data;

		for (i=0; i < 256*256; i++)
		{
			c=texture[i];
			r=(c>>16)&0xFF;
			g=(c>>8)&0xFF;
			b=(c)&0xFF;
			*d++=b;
			*d++=g;
			*d++=r;
		}
	nlTGAHeader header;

	memset(&header,0,sizeof(header));
	header.data_type=2;
	header.width=256;
	header.height=256;
	header.data_bits=0x18;


	fwrite((void*)&header,sizeof(nlTGAHeader),1,fp);		
	fwrite(data,256*256*3,1,fp);
	fclose(fp);
	free(data);

}

//--------


//---------



int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPTSTR lpCmdLine, int nCmdShow)
{

	int i,j,k;
	g_hInstance = hInstance;

	MSG msg;
	WNDCLASS wc;

	m_Width=WIDTH;
	m_Height=HEIGHT; 
	m_Bpp=32;
		
	
 	DDraw_Init();		
		   
	{
	g_hWnd = NULL;
	
	// create the Window Class for our window

	memset( &wc, 0, sizeof(wc) );
	wc.style = CS_VREDRAW | CS_HREDRAW;	
	wc.lpfnWndProc = WndProc;
	wc.hInstance = g_hInstance;
	wc.hIcon = hIcon;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszClassName = m_AppName;

	RegisterClass( &wc );

	g_hWnd = CreateWindow(m_AppName,m_AppTitle,WS_SYSMENU|WS_MINIMIZEBOX,//|WS_MAXIMIZEBOX,
		50,50,100,100,
		NULL,NULL,g_hInstance, NULL);

	AdjustWindow(g_hWnd);
	ShowWindow(g_hWnd, SW_SHOWNORMAL);
	DDraw_Open();

	LoadTexture("textures/1.tga",texture);
	memcpy(buffer, texture, 256*256*4);
	DDraw_Flip();



	quality=10;

	CompressTexture(texture, buffer, quality);
	DeCompressTexture(buffer, buffer, quality);


					// the main message loop
                   do
                   {
                           // Windows shit					   
                           if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                           {
                                   TranslateMessage(&msg);
                                   DispatchMessage(&msg);
		                   }						   			   						    
						   DDraw_Flip();

					char s[256]; wsprintf(s,"Quality: %i",quality);
					SetWindowText(g_hWnd,s);

                   } while(msg.message!=WM_QUIT);
	DDraw_Close();
	}

	SaveTexture("output.tga",buffer);
	CompressTexture(buffer, buffer, quality);
	SaveTexture("raw_dct.tga",buffer);

	return 0;
}




