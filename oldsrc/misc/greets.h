#define LastGreet 52
#define RecycleMsg 8

int NEWMES = 0;
int OLDMES = -1;

typedef struct {
	int x, y;
	float size;
	int x1, y1, x2, y2;
	int start_time, show_time;
	float start_size, end_size;
	char *s;
} msg;

msg m[LastGreet] = {
	0, 0, 0, 160, 100, 160, 100, 10000000, 560, 1, 0, "Y2K",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "THE 64K INTRO",
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "THE GREAT LAME",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "BY",
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "JORIC",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "IN 2000",
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "BASED ON",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "TINYPLAY",
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "GREETS TO",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "CRG",
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "EAGLE",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "AND DX69",	//10
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "OH FELLOWS",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "IT IS COMING",
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "YEAR 2000",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "AND PROBLEMS",
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "WOULD BE",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "GREAT",
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "NUCLEAR",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "ROCKETS",
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "FORCED BY",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "CRAZY",	//20
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "COMPUTERS",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "WILL FALL",
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "RIGHT",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "ON OUR",
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "CITY",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "WHERE",
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "WE LIVE",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "HAPPY",
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "MILLENIUM",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "I WROTE",	//10
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "THIS",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "INTRO",
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "IN WATCOM C",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "32BIT PMODE",
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "WITH TINY",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "STARTUP AND",
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "LIBRARYS",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "SO IT CAN BE",
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "VERY SLOW",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "BTW I HAVE",	//40
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "NO TIME",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "FOR OPTIMIZE",
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "JUST BUY",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "PENTIUM III",	//44
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "AND NO PROBLEM",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "BTW CHECK",
	0, 0, 0, 260, 150, 160, 100, 10000000, 180, 1, 0, "THIS INTRO",
	0, 0, 0, 260, 50, 160, 100, 10000000, 180, 1, 0, "FOR BUGS",
	0, 0, 0, 80, 150, 160, 100, 10000000, 180, 1, 0, "SEE YA",
	0, 0, 0, 80, 50, 160, 100, 10000000, 180, 1, 0, "ONCE MORE",	//50
};

void poll_msg()
{
	int i;
	i = NEWMES;
	m[i].start_time = real_time;
	NEWMES++;
	if (NEWMES >= LastGreet)
		NEWMES = RecycleMsg;
}

void greets()
{
	int i, len, hgt, x0, y0;
	float k, size;
	int color;


	for (i = 0; i < LastGreet; i++)
	{
		k = (float)(real_time - m[i].start_time) / (m[i].show_time);
		color= ((1-k)*0xFF);
		//color = 0xFFFFFF;
		if (k > 0 && k < 1)
		{
			m[i].x = int(m[i].x1 + (float)k *(m[i].x2 - m[i].x1));
			m[i].y = int(m[i].y1 + (float)k *(m[i].y2 - m[i].y1));
			m[i].size = m[i].start_size + (float)k *(m[i].end_size - m[i].start_size);

			len = text2buf(m[i].s, 255);
			hgt = ch_hgt;
			size = m[i].size;

			x0 = m[i].x;
			y0 = m[i].y;
			x0 -= int(len * size / 2);
			y0 -= int(hgt * size / 2);
			paste_pbuf(x0, y0, int(len * size), int(hgt * size), 0, 0, len, hgt, color);
		}
	}
}
