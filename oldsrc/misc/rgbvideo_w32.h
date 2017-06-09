// маленькая видеобиблиотека для 320x200:

// Установка графического режима
// Вывод текста
// Установка RGB для заданного цвета
// Градиентная палитра
// Точка
// Прямая

#include <malloc.h>

#include <windows.h>

#define _USE_MATH_DEFINES
#include <math.h>

#pragma comment(lib,"gdi32")
#pragma comment(lib,"user32")

#define	WINDOW_WIDTH 640
#define	WINDOW_HEIGHT 400

#define	BMP_WIDTH 320
#define	BMP_HEIGHT 200

#define CLASSNAME "Main"

#define MAXX 320
#define MAXY 200

HINSTANCE g_hInstance;
HBITMAP g_hBitmap;
HDC g_hDC;
unsigned long g_dwPalette[256];
void *g_pBits = NULL;

int pic[256][256];

char tbuffer[64000 * 4];
char tpicture[64000 * 4];
char *buffer;
char *picture;

int x320[200];
void initvideo()
{
	int i, j, c;
	double r;
	buffer = tbuffer;
	picture = tpicture;

	for (i = 0; i < 200; i++)
		x320[i] = i * 320;

//calculate phong texture

	for (j = 0; j < 256; j++)
		for (i = 0; i < 256; i++)
		{
			r = (sin(i * M_PI / 256.0) * sin(j * M_PI / 256.0));
			c = int(255 * r * r * r * r);
			c = c + c * 256 + c * 65536;
			pic[i][j] = c;
		}
}

void _swap(char *a, char *b)
{
	char *c = a;
	a = b;
	b = c;
}

void _putpixel(int x, int y, char r, char g, char b)
{
	char *p;
	if (x >= 0 && x < MAXX && y >= 0 && y < MAXY)
	{
		p = buffer;
		p += (x + x320[y]) << 2;
		*p = r;
		p++;
		*p = g;
		p++;
		*p = b;
	}
}

void getpixel(int x, int y, int *c)
{
	if (x >= 0 && x < MAXX && y >= 0 && y < MAXY)
	{
		char *p;
		p = buffer;
		p += (x + x320[y]) << 2;
		*c = *(int *)p;
	}
}

void _addpixel(int x, int y, char r, char g, char b)
{
	unsigned char *p;
	if (x >= 0 && x < MAXX && y >= 0 && y < MAXY)
	{
		p = (unsigned char*)buffer;
		p += (x + x320[y]) << 2;
		if (*p + r < 255)
			*p += r;
		else
			*p = 255;
		p++;
		if (*p + g < 255)
			*p += g;
		else
			*p = 255;
		p++;
		if (*p + b < 255)
			*p += b;
		else
			*p = 255;
	}
}

void putpixel(int x, int y, int color)
{
	char *p;
	if (x >= 0 && x < MAXX && y >= 0 && y < MAXY)
	{
		p = buffer;
		p += (x + x320[y]) << 2;
		*p = color & 0xFF;
		p++;
		*p = color >> 8 & 0xFF;
		p++;
		*p = color >> 16 & 0xFF;
	}
}

void line(int x1, int y1, int x2, int y2, int color)
{
// никаких ограничений на входные данные, так что x1>x2 без проблем :)
	int d, ax, ay, sx, sy, dx, dy;
// Здесь мы будем использовать только целочисленную арифметику
	dx = x2 - x1;
	ax = abs(dx) << 1;
	if (dx < 0)
		sx = -1;
	else
		sx = 1;
	dy = y2 - y1;
	ay = abs(dy) << 1;
	if (dy < 0)
		sy = -1;
	else
		sy = 1;
	putpixel(x1, y1, color);
	if (ax > ay)
	{
		d = ay - (ax >> 1);
		while (x1 != x2)
		{
			if (d >= 0)
			{
				y1 += sy;
				d -= ax;
			}
			x1 += sx;
			d += ay;
			putpixel(x1, y1, color);
		}
	}
	else
	{
		d = ax - (ay >> 1);
		while (y1 != y2)
		{
			if (d >= 0)
			{
				x1 += sx;
				d -= ay;
			}
			y1 += sy;
			d += ax;
			putpixel(x1, y1, color);
		}
	}
}

//Функция ожидания обратного хода луча
void waitretrace()
{
	while ((inp(0x3DA) & 0x8) != 0);
	while ((inp(0x3DA) & 0x8) == 0);
}

void copybuffer(char *buf)
{
// set_page(0); memcpy(buf,(char*)0xA0000,65536);
// set_page(1); memcpy(buf+65536,(char*)0xA0000,65536);
// set_page(2); memcpy(buf+65536*2,(char*)0xA0000,65536);
// set_page(3); memcpy(buf+65536*3,(char*)0xA0000,65536);

	memcpy(g_pBits, buf, MAXX * MAXY * 4);
}

void free_all()
{
	free(buffer);
}

void blur()
{
	int dc = 25;
	char *b;
	int i;
	b = buffer;
	for (i = 0; i < 64000 * 4; i += 4)
	{
		b[i] = (b[i - 4] + b[i + 4] + b[i - 320 * 4] + b[i + 320 * 4]) >> 2;
//     if (b[i]>=dc) b[i]-=dc;
		b[i + 1] = (b[i + 1 - 4] + b[i + 1 + 4] + b[i + 1 - 320 * 4] + b[i + 1 + 320 * 4]) >> 2;
//     if (b[i+1]>=dc) b[i+1]-=dc;
		b[i + 2] = (b[i + 2 - 4] + b[i + 2 + 4] + b[i + 2 - 320 * 4] + b[i + 2 + 320 * 4]) >> 2;
//    if (b[i+2]>=dc) b[i+2]-=dc;
	}
}

// int BlurDec = 0x05050505;
// int BlurDec = 0x40404040;
int BlurDec = 0x0;

void suxx_blur()
{
}

void copybuffer2txt()
{
}

typedef void (*cb_t) ();

cb_t update_function;

void set_update_function(cb_t function)
{
	update_function = function;
}

void CreateBmp(HWND hWnd)
{
	BITMAPINFO binfo;
	HDC hdc = GetDC(hWnd);
	binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	binfo.bmiHeader.biWidth = BMP_WIDTH;
	binfo.bmiHeader.biHeight = BMP_HEIGHT;
	binfo.bmiHeader.biPlanes = 1;
	binfo.bmiHeader.biBitCount = 32;
	binfo.bmiHeader.biCompression = 0;
	binfo.bmiHeader.biSizeImage = BMP_WIDTH * BMP_HEIGHT * 4;
	g_hBitmap = CreateDIBSection(hdc, &binfo, DIB_RGB_COLORS, &g_pBits, 0, 0);
	g_hDC = CreateCompatibleDC(hdc);
	SelectObject(g_hDC, g_hBitmap);
	ReleaseDC(hWnd, hdc);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	switch (message)
	{

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			update_function();
			GetClientRect(hWnd, &rect);
			SetStretchBltMode(hdc, HALFTONE);
			StretchBlt(hdc, 0, 0, rect.right, rect.bottom, g_hDC, 0, BMP_HEIGHT - 1, BMP_WIDTH, -BMP_HEIGHT, SRCCOPY);
			EndPaint(hWnd, &ps);
			break;
		case WM_KEYDOWN:
			if ((wParam == VK_ESCAPE) || (wParam == 'C' && GetAsyncKeyState(VK_CONTROL)))
				SendMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
			break;
		case WM_CREATE:
			CreateBmp(hWnd);
//              InitPalette();
			initvideo();
			break;
		case WM_TIMER:
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int main_loop()
{

	MSG msg;
	HWND hWnd;
	WNDCLASS wc;

	g_hInstance = GetModuleHandle(NULL);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC) WndProc;
	wc.hInstance = g_hInstance;
	wc.hbrBackground = NULL;
	wc.lpszClassName = CLASSNAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	RegisterClass(&wc);

	int w = WINDOW_WIDTH;
	int h = WINDOW_HEIGHT;

	if (!(hWnd = CreateWindow(wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW, 0, 0, w, h, NULL, NULL, wc.hInstance, NULL)))
		return FALSE;

	RECT rc;

	SetRect(&rc, 0, 0, w, h);

	AdjustWindowRectEx(&rc, GetWindowLong(hWnd, GWL_STYLE), GetMenu(hWnd) != NULL, GetWindowLong(hWnd, GWL_EXSTYLE));

	w = rc.right - rc.left;
	h = rc.bottom - rc.top;

	SetWindowPos(hWnd, NULL, (GetSystemMetrics(SM_CXSCREEN) - w) / 2, (GetSystemMetrics(SM_CYSCREEN) - h) / 2, w, h, SWP_NOZORDER | SWP_NOACTIVATE);

	ShowWindow(hWnd, true);
	UpdateWindow(hWnd);

	SetTimer(hWnd, 0, 1, NULL);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
