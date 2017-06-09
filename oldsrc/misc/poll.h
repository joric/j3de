//а здесь у нас все, что мы хотим привязать на таймер

int _songpos;
void wc_poll()
{
int i;
char *p;
sync++;
real_time=sync;
//   poll_msg();
p=(char*)(fxdata);
}

void setpoll(void * offset_myhandler,int frq)
{
  _asm {
  mov eax,frq
  mov edx,offset_myhandler
  call newhandler
  }
}
void removepoll(void * offset_myhandler)
{
  _asm {
  mov eax,0
  mov edx,offset_myhandler
  call newhandler
  }

}
