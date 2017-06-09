#include <Windows.h>
#include <stdio.h>
#include <ddraw.h>
#include <math.h>
#include <mmsystem.h>
#include "res/resource.h"

#include "main.h"
	
	long m_counter;
	long m_rendermode;


	static LPDIRECTDRAW	m_DDraw = NULL;
	static LPDIRECTDRAWSURFACE m_FrontBuffer = NULL;
	static LPDIRECTDRAWSURFACE m_BackBuffer = NULL;
	static LPDIRECTDRAWCLIPPER m_Clipper = NULL;
	DDPIXELFORMAT PixelFormat;

	RECT rect;
	BOOL ModeFound;

	int m_Width, m_Height, m_Bpp;

	int m_windowed;	

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

void TimerFunction(void)
{
	m_counter++;
}

static volatile MMRESULT timerding;

static void CALLBACK DoTimers(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
        TimerFunction();
        return;
}

int Timer_Start(int speed)
{      
      timerding=timeSetEvent(speed,0,DoTimers,0,TIME_PERIODIC);
	  return 0;
}

int Timer_Stop()
{
      timeKillEvent(timerding);
	  return 0;
}

 
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

	if (!m_windowed) 
	{		
		// search for our desired mode
		ModeFound=0;
		for (i=0; i < m_NumberModes; i++)
		{
			if ( (m_ModeList[i].Width == m_Width) && (m_ModeList[i].Height == m_Height) 				
				&& (m_ModeList[i].Bpp >=16) )
			{								
				m_Bpp=m_ModeList[i].Bpp;
			    m_DDraw->SetCooperativeLevel( g_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
				m_DDraw->SetDisplayMode(m_Width, m_Height, m_Bpp);
				ModeFound=1;				
			}
		}
		if (!ModeFound) 
		{
			//MessageBox(g_hWnd,"Sorry, we have to goto to windowed mode =(","System Message",MB_OK);
		    m_windowed=true;		
		}
	}	

	if (m_windowed)	m_DDraw->SetCooperativeLevel( g_hWnd, DDSCL_NORMAL );
	
	// create front and back buffers
	memset( &ddsd, 0, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);

	if (m_windowed) 
	ddsd.dwFlags = DDSD_CAPS; 
	else ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;

	ddsd.dwBackBufferCount = 1;

	if (m_windowed)	
	ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_PRIMARYSURFACE;
	else ddsd.ddsCaps.dwCaps = DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY | DDSCAPS_PRIMARYSURFACE;
	
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

	if (m_windowed)
	{
	ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN;
	hr = m_DDraw->CreateSurface( &ddsd, &m_BackBuffer, NULL );
	// create clipper
	hr = m_DDraw->CreateClipper( 0, &m_Clipper, NULL);
	// attach clipper to window
	hr = m_Clipper->SetHWnd( 0, g_hWnd );
	// attach clipper to front buffer
	hr = m_FrontBuffer->SetClipper( m_Clipper );
	} 
	else
	{
	ddcs.dwCaps = DDSCAPS_BACKBUFFER;
	hr = m_FrontBuffer->GetAttachedSurface( &ddcs, &m_BackBuffer);
	}
		
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
			Dest = Scanline;
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

			Scanline += ddsd.lPitch;
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

	if (m_windowed) 
		hr=m_FrontBuffer->Blt( &rect, m_BackBuffer, NULL, DDBLT_WAIT|DDBLT_ASYNC, NULL);
	else
		hr= m_FrontBuffer->Flip(NULL, DDFLIP_WAIT);

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



BOOL APIENTRY AboutDlgProc(HWND hDlg, WORD msg, WORD wParam, LONG lParam)
{
	int i, index;
	switch (msg) 
	{

	case WM_PAINT:		
		break;
	case WM_INITDIALOG:  

		SendDlgItemMessage(hDlg,IDC_RADIO1,BM_SETCHECK,BST_CHECKED,0);
		SendMessage(hDlg,WM_SETICON,ICON_SMALL,(long)hIcon);
		SendMessage(hDlg,WM_SETICON,ICON_BIG,(long)hIcon);

		char str[256];

		index=-1;
		for (i=0; i < m_NumberModes; i++)
		{	
			if ( (m_ModeList[i].Width == m_Width) && (m_ModeList[i].Height == m_Height) && (m_ModeList[i].Bpp >= 16) )
			{					
				index++;				
				sprintf(str,"%ix%ix%i",m_ModeList[i].Width,m_ModeList[i].Height,m_ModeList[i].Bpp);
				SendDlgItemMessage (hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (DWORD)(LPSTR)str);
			}
		}

		SendDlgItemMessage (hDlg, IDC_COMBO1, CB_SETCURSEL, index, 0);

		return(TRUE);
		break;   
	
	case WM_COMMAND:            

            switch (LOWORD(wParam)) 
			{
	            case IDOK:                

				m_windowed = (SendDlgItemMessage (hDlg,IDC_FULLSCREEN, BM_GETSTATE , 0, 0)!=BST_CHECKED);
					EndDialog(hDlg, TRUE);					
					return(TRUE);
					break;

                case IDCANCEL:                    
					EndDialog(hDlg, FALSE);
					return(TRUE);
					break;
			}
	}
	return(FALSE);
}

void SwitchMode()
{
	m_windowed=!m_windowed;	
	DDraw_Close();
	if (m_windowed) AdjustWindow(g_hWnd);
	DDraw_Init();				
	DDraw_Open();	
}

static long CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	
		case WM_MOVING:	
//			DDraw_Flip();		
		break;

		case WM_DESTROY:
			PostQuitMessage(-1);
			break;

		case WM_PAINT:
//			DDraw_Flip();
			DefWindowProc(hWnd,msg,wParam,lParam);
			break;
		
        case WM_SETCURSOR:
        {
            // hide cursor
            if (!m_windowed) SetCursor(0);
        }
        break;

		case WM_KEYDOWN:		
			switch( wParam )
			{				
				case VK_ESCAPE:
					PostQuitMessage(-1);
					break;				
			}
			break;

		case WM_CHAR:
			switch( (TCHAR)wParam )
			{				
				case 49: m_rendermode=0;break;
				case 50: m_rendermode=1;break;
				case 51: m_rendermode=2;break;
				case 52: m_counter+=500;break;
				case 96: m_console=!m_console;break;
			}
			//char str[256]; sprintf(str,"%i",wParam); MessageBox(g_hWnd,str,"",MB_OK);

			break;                		
/*	
		case WM_SYSCOMMAND:            
            switch (wParam) 
			{
	            case (SC_MAXIMIZE):                
					SwitchMode();
					break;
	            case (SC_RESTORE):                
						SwitchMode();
					break;
			}

*/		

		case (WM_SYSKEYUP):
        switch( wParam )
        {
			case (VK_RETURN):				
/*				if (m_windowed) 
					SendMessage(hWnd,WM_SYSCOMMAND,SC_MAXIMIZE,NULL);
				else 			
					SendMessage(hWnd,WM_SYSCOMMAND,SC_RESTORE,NULL);
*/				SwitchMode();
			break;
        }

        break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPTSTR lpCmdLine, int nCmdShow)
{
					
	g_hInstance = hInstance;

	MSG msg;
	WNDCLASS wc;

	m_windowed=true;

	m_Width=WIDTH;
	m_Height=HEIGHT; 
	m_Bpp=32;
		
	hIcon=LoadIcon(g_hInstance,MAKEINTRESOURCE(IDI_ICON1));

 	DDraw_Init();		
		   
	if (DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG1), g_hWnd, (DLGPROC)AboutDlgProc))
//	m_windowed=true;
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

	g_hWnd = CreateWindow(m_AppName,m_AppTitle,WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX,
		50,50,m_Width,m_Height,
		NULL,NULL,g_hInstance, NULL);

	//AdjustWindow(g_hWnd);
	ShowWindow(g_hWnd, SW_SHOWNORMAL);


	//initalise
	//DDraw_Open();


	Init(); // General init routine

	
	Timer_Start(1000/timerrate);
		
					// the main message loop
                   do
                   {
                           // Windows shit
					   
                           if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                           {
                                   TranslateMessage(&msg);
                                   DispatchMessage(&msg);
		                   }						   			   						    
//						   UpdateFrame();						   
//							DDraw_Flip();
                   } while(msg.message!=WM_QUIT);


	MessageBox(NULL,"exit","",NULL); exit(1);

	Timer_Stop();
	//DDraw_Close();
	CloseAll();
	}
	return 0;
}



