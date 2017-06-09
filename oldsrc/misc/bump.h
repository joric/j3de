#define BBOUND 50
#define BMinVelosity 0.5
#define BMaxVelosity 0.5

struct tbump {
   float x,y,ax,ay,dx,dy,px,py;
};
tbump bp[1];
void initbump()
{
int i=0;
  bp[i].ax=random(256)-128;
  bp[i].dx=MinVelosity+random(256)*MaxVelosity/256;
  bp[i].ay=random(256)-128;
  bp[i].dy=MinVelosity+random(256)*MaxVelosity/256;

}

void bump()
{
  int c,x,y,z,ofs,nx,ny,lx,ly;
  float p,dx,dy;
  int i,j; int *pc; char *bb;
  char br,r,g,b,cmp;
  i=0;
  pc=(int*)picture;
  bb=buffer;
  float bright=0.5;

  lx=bp[i].x = BBOUND + (sin(bp[i].ax)+1)/2*(MAXX-BBOUND*2);
  ly=bp[i].y = BBOUND + (sin(bp[i].ay)+1)/2*(MAXY-BBOUND*2);
  bp[i].ax += bp[i].dx;
  bp[i].ay += bp[i].dy;

for (y=0;y<200;y++)
  for (x=0;x<320;x++)
  {
  ofs=x+y*320;
  {
  nx=bb[ofs+320*4]-bb[ofs-320*4];
  ny=bb[ofs+4]-bb[ofs-4];
  nx=0;
  ny=0;
  nx=nx+x-lx;
  ny=ny+y-ly;
  nx=nx+128;
  ny=ny+128;
  if ((nx<0) || (nx>255)) nx=0;
  if ((ny<0) || (ny>255)) ny=0;

_asm{
    mov esi,picture
    mov edi,buffer
    mov edx,ofs
    movq  mm1,0x1111111111111111    ; RGB-�ᢥ饭�����, qword
                               ; (4 ��㪨 0:9 fixedpoint)
    movd       mm0,[edx*4+esi] ; ��㧨� ���ᥫ
    punpcklbw  mm0,mm0         ; �ᯠ���뢠�� ���ᥫ
;    psrlw      mm0,1           ; ��� ⮣�, �⮡� �뫨 ���������� �᫠
;    pmulhw     mm0,mm1         ; 㬭����� RGB �� RGB-�ᢥ饭�����
    packuswb   mm0,mm0         ; ���㥬 ���ᥫ ���⭮
    movd       [edx*4+edi],mm0
}

//  br=pic[ny][nx];
//  for (cmp=0;cmp<3;cmp++) { c=picture[ofs*4+cmp]; if (br){ c=(c+br*(256-c)/256); } bb[ofs*4+cmp]=c; }

    }
  }
}
