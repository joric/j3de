unsigned char rsqr[128000];
long bbx=160,bby=100;
float time=0;


void average(int p1,int p2, int * clr)
{
int res;
int r1,g1,b1,r2,g2,b2,r,g,b;
int f=100;
char BlrDec=0x8;
int alpha=BlrDec<<16|BlrDec<<8|BlrDec;
  _asm{
        pxor mm0,mm0
        movd mm1,p1
        movd mm2,p2
        movd mm7,alpha
        punpcklbw mm1,mm0
        punpcklbw mm2,mm0
        paddw mm1,mm2
//        psrlw mm1,1    //mm1 div 2
        packuswb mm1,mm0
          psubusb mm1,mm7
//        paddusb mm1,mm7
        movd res,mm1
        emms
        }
  *clr=res;
}

void radialblurscreen()
{
//  bbx=blob[0].x;  bby=blob[0].y;
  bbx=160; bby=100;
  short x,y,vx,vy,px,py,n,tx,ty;
  long t; int p1,p2,clr;
  long tn=(long)(&rsqr);
  t=tn;
  for(y=bby-1;y>=0;y--)
  {
    for(x=bbx-1;x>=0;x--)
      {
     vx=*((char*)(t+64000));vy=*((char*)(t));
     getpixel(x,y, &p1);
     getpixel(x+vx,y+vy, &p2);
     average(p1,p2,&clr);
     putpixel(x,y,clr);
        t++;
      }
    t+=(320-bbx);
  }

  t=tn;
  for(y=bby-1;y>=0;y--)
  {
    for(x=bbx;x<320;x++)
      {
     vx=*((char*)(t+64000));vy=*((char*)(t));
     getpixel(x,y, &p1);
     getpixel(x-vx,y+vy, &p2);
     average(p1,p2,&clr);
     putpixel(x,y,clr);
        t++;
        tx++;
      }
    t+=(bbx);
  }

  t=tn;
  for(y=bby;y<200;y++)
  {
    for(x=bbx-1;x>=0;x--)
      {
        vx=*((char*)(t+64000));vy=*((char*)(t));
     getpixel(x,y, &p1);
     getpixel(x+vx,y-vy, &p2);
     average(p1,p2,&clr);
     putpixel(x,y,clr);
        t++;
      }
    t+=(320-bbx);
  }

  t=tn;
  for(y=bby;y<200;y++)
  {
    for(x=bbx;x<320;x++)
      {
        vx=*((char*)(t+64000));vy=*((char*)(t));
     getpixel(x,y, &p1);
     getpixel(x-vx,y-vy, &p2);
     average(p1,p2,&clr);
     putpixel(x,y,clr);
      t++;
      }
    t+=(bbx);
  }

}


void initsqr()
{
  short vx,vy;
  float n;
  for(short x=0;x<320;x++)
    for(short y=0;y<200;y++)
    {
        n=(sqrt(x*x+y*y)/4)+1;
        vx=x/n;
        vy=y/n;
        rsqr[x+y*320+64000]=vx;
        rsqr[x+y*320]=vy;
    }
}
