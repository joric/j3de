// Установка графического режима
void setmode (short c);
#pragma aux setmode = "int 0x10" \
                       parm[ax] modify [eax];

// Установка графического режима VESA
void set_vesa_mode (short c);
#pragma aux set_vesa_mode =\
"mov ax,0x4f02" \
"int 0x10" \
parm[bx] modify [eax ebx];

void set_page (short gran);
#pragma aux set_page =\
"mov ax,0x4f05" \
"xor bx,bx" \
"xor cx,cx" \
"int 0x10" \
parm[dx] modify [eax ebx];

#define    ESC_KEY 1
#define   LEFT_KEY 75
#define  RIGHT_KEY 77
#define     UP_KEY 72
#define   DOWN_KEY 80
#define INSERT_KEY 82
#define DELETE_KEY 83
#define   HOME_KEY 71
#define    END_KEY 79
#define   PGUP_KEY 73
#define   PGDN_KEY 81
#define  SPACE_KEY 57
#define  ENTER_KEY 28
#define  SHIFT_KEY 54
#define   CTRL_KEY 29
#define    ALT_KEY 56

// функция чтения клавиши
char scancode(void);
#pragma aux scancode = " in al,0x60 " \
                       value[al] modify[eax];
char kbHit (){
//Почистим буфер клавиатуры
*(short *)(0x041a) = *(short *)(0x041c);
return(scancode());
}

// функция записи в порт
void outp(int port,int value);
#pragma aux outp = " out dx, al " \
                   parm [edx][al] modify[eax edx];

// функция чтения из порта
char inp(int port);
#pragma aux inp = " in al, dx " \
                  value[al] parm [edx] modify[eax edx];

// функция копирования блока памяти
void memcpy(void *Destination, void *Source, int Length);
#pragma aux memcpy = \
  " mov eax,ecx " \
  " shr ecx,2   " \
  " rep movsd   " \
  " mov ecx,eax " \
  " and ecx,3   " \
  " rep movsb   " \
  parm [esi][edi][ecx] modify [eax esi edi ecx];

// функция заполнения блока памяти
void memset(void *dest, unsigned char fill, int Length);
#pragma aux memset   = \
  "   cld            " \
  "   mov ah,al      " \
  "   shr ecx,1      " \
  "   rep stosw      " \
  "   adc ecx,ecx    " \
  "   rep stosb      " \
  parm [edi] [al] [ecx] modify [eax ecx edi];
