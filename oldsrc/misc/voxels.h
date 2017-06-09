//Some voxel space

#define VIEW_HEIGHT 400
#define XDOTS       256
#define YDOTS       256
#define MAXCOLOR    199
#define MINCOLOR    20
#define BPL         256

#define MAXDEPTH    20
#define K0          3
#define K2          128

char* buf = buffer;

int invtab[MAXDEPTH];
unsigned char *height, *color;
int oldy[XDOTS];
int oldc[XDOTS];
int lasty[XDOTS];
int ix,iy,u,v,x,y,basey,col,adj,tmp,count,step;
char *ptr;

volatile int retraces;

int val(int x1, int y1, int x2, int y2, int len)
{
    int n;
    n = (((int)height[(x1&0xff)+(y1&0xff)*256] +
        (int)height[(x2&0xff)+(y2&0xff)*256])>>1) + len*(random(4)-1);
    return ((n<1) ? 1 : ((n>255) ? 255 : n));
}

void makeplasma(int x1,int y1,int x2,int y2)
{
    int xc,yc,len;
    if ((len = x2-x1) < 2) return;
    xc = (x1+x2)>>1;
    yc = (y1+y2)>>1;
    if (!height[xc+(y1&0xff)*256]) height[xc+(y1&0xff)*256] = val(x1,y1,x2,y1,len);
    if (!height[xc+(y2&0xff)*256]) height[xc+(y2&0xff)*256] = val(x1,y2,x2,y2,len);
    if (!height[(x1&0xff)+yc*256]) height[(x1&0xff)+yc*256] = val(x1,y1,x1,y2,len);
    if (!height[(x2&0xff)+yc*256]) height[(x2&0xff)+yc*256] = val(x2,y1,x2,y2,len);
    height[xc+yc*256] = (val(xc,y1,xc,y2,len) + val(x1,yc,x2,yc,len))>>1;
    makeplasma(x1,y1,xc,yc);
    makeplasma(xc,y1,x2,yc);
    makeplasma(x1,yc,xc,y2);
    makeplasma(xc,yc,x2,y2);
}

// voxel routines
void initdraw(void)
{
    int i,j,k;

    // make inverse "1/x" table
    for (i = j = 0; i < MAXDEPTH; i++) {
        invtab[i] = 256/(i+1);
    }

    // make surface height table

    for (i=0;i<256*256;i++) height[i]=0;

    makeplasma(0,0,256,256);

    // remove noise in the surface height table
    for (i = 0; i < 256; i++) {
        for (j = 0; j < 256; j++) {
            height[i+j*256] = ((int)height[i+j*256] +
                (int)height[((i+1)&0xff)+j*256] + (int)height[i+((j+1)&0xff)*256] +
                (int)height[((i+1)&0xff)+((j+1)&0xff)*256])>>2;
        }
    }


    // make surface color table
    for (i = 0; i < 256; i++) {
        for (j = 0; j < 256; j++) {
            k = (((int)(height[i+j*256] - height[((i+3)&0xff)+((j+0)&0xff)*256]))<<2)+128;
            k = ((k<0) ? 0 : ((k>255) ? 255 : k));
            k = MINCOLOR+k*(MAXCOLOR-MINCOLOR)/256;
            color[i+j*256] = k;
        }
    }

    for (i = 0; i < XDOTS; i++)
        lasty[i] = YDOTS;
}

void draw(int posx, int posy, int posz)
{
    int du;
    for (x = 0; x < XDOTS; x++) {
        oldy[x] = YDOTS;
        oldc[x] = 0;
    }
    for (v = 0; v < MAXDEPTH; v++, posy += 256) {
        adj = invtab[v];
        basey = (YDOTS/2) + ((posz*adj)>>16);
        adj += K2;
        u = (posx) - (XDOTS*K0/2)*v;
        du = K0*v;
        for (x = 0; x < XDOTS; x++) {
            ix = (unsigned char)(u>>8);
            iy = (unsigned char)(posy>>8);


            y = height[ix+iy*256];
            y += ((height[(unsigned char)(ix+1)+iy*256]-y)*((unsigned char)u))>>8;
            y = basey - ((y*adj)>>8);

            if (y < 0) y = 0;


            col = color[ix+iy*256];
            col += ((color[(unsigned char)(ix+1)+iy*256]-col)*((unsigned char)u))>>8;

            if (oldy[x] > y) {
                tmp = oldy[x];
                oldy[x] = y;
                y = tmp;
                tmp = oldc[x];
                oldc[x] = col;
                col = tmp;
                count = y-oldy[x];

                step = ((oldc[x]-col)<<8)/count;
                col <<= 8;
                y--;
                ptr = buf + x + BPL*y;
                if (count & 7) {
                    register int c = count&7;
                    for ( ; c-- > 0; ptr -= BPL) {
                        *ptr = col>>8;
                        col += step;
                    }
                }
                count >>= 3;
                for (; count-- > 0; ) {

                    ptr -= 8*BPL;
                    ptr[8*BPL] = col>>8;
                    col += step;
                    ptr[7*BPL] = col>>8;
                    col += step;
                    ptr[6*BPL] = col>>8;
                    col += step;
                    ptr[5*BPL] = col>>8;
                    col += step;
                    ptr[4*BPL] = col>>8;
                    col += step;
                    ptr[3*BPL] = col>>8;
                    col += step;
                    ptr[2*BPL] = col>>8;
                    col += step;
                    ptr[1*BPL] = col>>8;
                    col += step;

                }
            }
            else {
                oldc[x] = col;
            }
            u += du;
        }
    }

    for (x = 0; x < XDOTS; x++) {
        y = oldy[x];
        count = y - lasty[x];
        lasty[x] = y;
        ptr = buf + x + BPL*y;
        for ( ; count-- > 0; ptr -= BPL)
            *ptr = 0;
    }
}

void initvoxels(){
    height=(unsigned char*)malloc(256*256);
    color=(unsigned char*)malloc(256*256);
    initdraw();
}

void voxels(){
int i,j;
     retraces=counter;
     i = 12*retraces;
     j =256*retraces;
     draw(i,j,(VIEW_HEIGHT+height[((i>>8)&0xff)+((j>>8)&0xff)*256])<<8);
}
