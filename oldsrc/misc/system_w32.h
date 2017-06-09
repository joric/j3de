#include <malloc.h>
#include <memory.h>

void setmode(short c)
{

}

void set_page(short gran)
{
}

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
char scancode(void)
{
	return 0;
}

char kbHit()
{
	return (scancode());
}

// функция записи в порт
void outp(int port, int value)
{
}

// функция чтения из порта
char inp(int port)
{
	return 0;
}
