#define NB 100
#define BOUND 10
#define MinVelosity 0.01
#define MaxVelosity 0.05

struct tblob {
   float x,y,ax,ay,dx,dy,px,py,ddx,ddy;
   int color;
   int size;
};

tblob blob[NB];

void initparticles() {
int i;
  for (i=0;i<NB;i++){
  blob[i].ax=random(256)-128;   blob[i].ay=random(256)-128;
  blob[i].dx=MinVelosity+random(256)*MaxVelosity/256;
  blob[i].dy=MinVelosity+random(256)*MaxVelosity/256;
  blob[i].color=random(0xFFFF)*random(0xFFFF);

    if (i==0) blob[i].color=0x0000FF00;
     if (i==1) blob[i].color=0x00FF0000;
        if (i==2) blob[i].color=0x00FFFF00;
           if (i==3) blob[i].color=0x00FFFFFF;
  }
}

void draw2scanlines( int x, int y, int xc, int yc, int n )
  { line(xc+x, yc+y,xc-x, yc+y,n);
    line(xc+x, yc-y,xc-x, yc-y,n); }

void particle( int xc, int yc, int a0, int b0, int color ){

  int x = 0;
  int y = b0;
  int a = a0;
  int b = b0;
  int Asquared = a * a;
  int TwoAsquared = 2 * Asquared;
  int Bsquared = b * b;
  int TwoBsquared = 2 * Bsquared;
  int d;
  int dx,dy;
  int _y=-1;
  d = Bsquared - Asquared*b + Asquared/4L;
  dx = 0;
  dy = TwoAsquared * b;
  while (dx<dy)
  {
    draw2scanlines( x, y, xc, yc, color );
    if (d > 0L)
    {
      --y;
      dy -= TwoAsquared;
      d -= dy;
    }
    ++x;
    dx += TwoBsquared;
    d += Bsquared + dx;
  }

  d += (3L*(Asquared-Bsquared)/2L - (dx+dy)) / 2L;
  while (y>=0)
  {

    draw2scanlines( x, y, xc, yc, color );
    if (d < 0L)
    {
      ++x;
      dx += TwoBsquared;
      d += dx;
    }

    --y;
    dy -= TwoAsquared;
    d += Asquared - dy;
  }
}

void particles() {
  int x,y,z,i,c;
  char *p;
  float dx,dy;
   for (i=0;i<NB;i++) {
     x=blob[i].x = BOUND + (sin(blob[i].ax)+1)/2*(MAXX-BOUND*2);
     y=blob[i].y = BOUND + (sin(blob[i].ay)+1)/2*(MAXY-BOUND*2);
     if (i<1){
     blob[i].ax += blob[i].dx;
     blob[i].ay += blob[i].dy;
}
     if (i==0) blob[i].color=0;
     c=blob[i].color;
       particle(x,y,blob[i].size,blob[i].size*0.9,c);
     }
}

