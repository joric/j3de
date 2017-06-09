// маленькая видеобиблиотека для 320x200:

// Установка графического режима
// Вывод текста
// Установка RGB для заданного цвета
// Градиентная палитра
// Точка
// Прямая

#include <math.h>
#include <malloc.h>

#define MAXX 320
#define MAXY 200

int pic[256][256];

char tbuffer[64000*4];
char tpicture[64000*4];
char *buffer;
char *picture;

int x320[200];
void initvideo(){
int i,j,k,c; double r;
buffer = tbuffer;
picture=tpicture;

for (i=0;i<200;i++) x320[i]=i*320;
//calculate phong texture

for (j=0;j<256;j++)
  for (i=0;i<256;i++) {
    r=(sin(i*M_PI/256.0)*sin(j*M_PI/256.0));
    c=0+(255*r*r*r*r);
    c=c+c*256+c*65536;
    pic[i][j]=c;
   }
}

void _swap(char * a, char * b)
{
 char * c=a; a=b; b=c;
}

void _putpixel(int x,int y, char r,char g,char b){
char *p;
if (x>=0 & x<MAXX & y>=0 & y<MAXY){
p = buffer; p+=x+x320[y]<<2;
*p=r; p++;
*p=g; p++;
*p=b; }
}

void getpixel(int x,int y,int * c){
if (x>=0 & x<MAXX & y>=0 & y<MAXY){
char *p;
p = buffer; p+=x+x320[y]<<2;
*c=*(int*)p;
}}

void _addpixel(int x,int y, char r,char g,char b){
char *p; double _r, _g, _b;
if (x>=0 & x<MAXX & y>=0 & y<MAXY){
p = buffer; p+=x+x320[y]<<2;
if (*p+r<255) *p+=r;else *p=255; p++;
if (*p+g<255) *p+=g;else *p=255; p++;
if (*p+b<255) *p+=b;else *p=255;
}
}

void putpixel(int x,int y, int color){
char *p;
if (x>=0 & x<MAXX & y>=0 & y<MAXY){
p = buffer; p+=x+x320[y]<<2;
*p=color & 0xFF; p++;
*p=color >>8 & 0xFF; p++;
*p=color >>16 & 0xFF; }
}

// Процедура рисования линии по Брезенхейму
  int abs(int n) { if (n<0) {n=-n;} return(n); }
void line (int x1, int y1, int x2, int y2, int color) {
// никаких ограничений на входные данные, так что x1>x2 без проблем :)
  int d, ax, ay, sx, sy, dx, dy;
// Здесь мы будем использовать только целочисленную арифметику
  dx = x2-x1;  ax = abs (dx) << 1; if (dx<0) sx=-1; else sx=1;
  dy = y2-y1;  ay = abs (dy) << 1; if (dy<0) sy=-1; else sy=1;
  putpixel (x1, y1, color);
  if (ax>ay) {
    d = ay-(ax >> 1);
    while (x1!=x2) {
      if (d>=0) { y1+=sy; d-=ax;}
      x1+=sx;
      d+=ay;
      putpixel (x1, y1, color);
      }
    }
  else {
    d= ax-(ay >> 1);
    while (y1!=y2) {
      if (d>=0) { x1+=sx; d-=ay; }
      y1+=sy;
      d+=ax;
      putpixel (x1, y1, color);
    }
  }
}

//Функция ожидания обратного хода луча
void waitretrace(){
  while ( (inp(0x3DA) & 0x8) != 0);
  while ( (inp(0x3DA) & 0x8) == 0);
}

void copybuffer(char * buf){
 set_page(0); memcpy(buf,(char*)0xA0000,65536);
 set_page(1); memcpy(buf+65536,(char*)0xA0000,65536);
 set_page(2); memcpy(buf+65536*2,(char*)0xA0000,65536);
 set_page(3); memcpy(buf+65536*3,(char*)0xA0000,65536);
}
void free_all()
{
  free(buffer);
}

void suxx_blur(){
int dc=25;
char* b; int i;
b=buffer;
for(i=0;i<64000*4;i+=4) {
   b[i]=( b[i-4] + b[i+4] + b[i-320*4] + b[i+320*4] ) >> 2;
//     if (b[i]>=dc) b[i]-=dc;
  b[i+1]=( b[i+1-4] + b[i+1+4] + b[i+1-320*4] + b[i+1+320*4] ) >> 2;
//     if (b[i+1]>=dc) b[i+1]-=dc;
   b[i+2]=( b[i+2-4] + b[i+2+4] + b[i+2-320*4] + b[i+2+320*4] ) >> 2;
//    if (b[i+2]>=dc) b[i+2]-=dc;
}
}
// int BlurDec = 0x05050505;
// int BlurDec = 0x40404040;
 int BlurDec = 0x0;

void blur()
{
_asm{
        mov edi,buffer
        xor ecx,ecx
        pxor mm0,mm0
        movd mm7,BlurDec
        mov edx,64000
@lp:
        movd mm1,[ecx*4+edi]
        movd mm2,[ecx*4+4+edi]
        punpcklbw mm1,mm0
        punpcklbw mm2,mm0

        movd mm3,[ecx*4+2560+4+edi]  //2560=320*4*2 :( nasty compiler!
        movd mm4,[ecx*4+2560+8+edi]
        punpcklbw mm3,mm0
        punpcklbw mm4,mm0

        paddw mm3,mm4
        paddw mm1,mm2
        paddw mm1,mm3
        psrlw mm1,2    //mm1 div 4
        packuswb mm1,mm0
        psubusb mm1,mm7 // mm1 - BlurDec
        movd [ecx*4+1280+4+edi],mm1           //1280=320*4
        inc ecx
        dec edx
        jnz @lp
        emms
}
}
void copybuffer2txt(){
_asm{
            mov edi,0xB8000
            mov esi,buffer
            mov edx,50
@convtxt_loopy:
                mov ecx,80
@convtxt_loopx:
                mov ebx,[esi]
                mov eax,ebx
                rol ebx,16
                and ebx,255
                and eax,255
                add ax,bx
                ror ebx,16
                shr ebx,8
                and ebx,255
                add ax,bx
                shr eax,2
                mov ah,al

                mov bl,0
                cmp al,0
                jle @asc0
                cmp al,48
                jge @asc0
                mov bl,176
                jmp @ascout
                @asc0:

                cmp al,48
                jle @asc1
                cmp al,96
                jge @asc1
                mov bl,177
                jmp @ascout
    @asc1:
                cmp al,96
                jle @asc2
                cmp al,144
                jge @asc2
                mov bl,178
                jmp @ascout
    @asc2:
                cmp al,144
                jle @asc3
                mov bl,219
                jmp @ascout
    @asc3:
    @ascout:
                shr ah,4
                mov al,bl
//                stosw
                add esi,16
                dec ecx
                jnz @convtxt_loopx
            add esi,3840
            dec edx
            jnz @convtxt_loopy
            ret
}}


