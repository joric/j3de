// ������⥪� ��� �뢮�� ᮮ�饭�� � ����᪮� ०���
// ���� 6x6 ᨬ���� 32..95 )
// �㭪樨:
// textpos(x,y)
// textcolor(color)
// print(char*)
// printn(int)
// printz(int,formatted_length);
// printfl(float,before,after);

int TEXT_X=0;
int TEXT_Y=0;
int TEXT_COLOR = 255;

unsigned char fonttable[]={
  0,  0,  0,  0,  0,  0, 32, 32, 32,  0, 32,  0,160,160,  0,  0,  0,
  0, 80,248, 80,248, 80,  0,120,160,112, 40,240,  0,200,208, 32, 88,
152,  0, 48, 80, 96,112,152,224,128,128,  0,  0,  0,  0, 16, 32, 32,
 32, 16,  0, 64, 32, 32, 32, 64,  0, 80, 32,248, 32, 80,  0, 32, 32,
248, 32, 32,  0,  0,  0,  0, 32, 32, 64,  0,  0,248,  0,  0,  0,  0,
  0,  0,  0, 32,  0,  8, 16, 32, 64,128,  0,248,136,136,136,248,  0,
224, 32, 32, 32,248,  0,248,  8,248,128,248,  0,248,  8,120,  8,248,
  0,136,136,248,  8,  8,  0,248,128,248,  8,248,  0,248,128,248,136,
248,  0,248,  8,  8,  8,  8,  0,248,136,248,136,248,  0,248,136,248,
  8,248,  0,  0, 32,  0, 32,  0,  0,  0, 32,  0,  0, 32, 64, 16, 32,
 64, 32, 16,  0,  0,248,  0,248,  0,  0, 64, 32, 16, 32, 64,  0,224,
 32, 96,  0, 64,  0,112,136,184,184,128,120,248,136,248,136,136,  0,
248,136,240,136,248,  0,248,136,128,136,248,  0,240,136,136,136,248,
  0,248,128,248,128,248,  0,248,128,248,128,128,  0,248,128,184,136,
248,  0,136,136,248,136,136,  0,248, 32, 32, 32,248,  0,  8,  8,  8,
136,248,  0,136,144,224,144,136,  0,128,128,128,128,248,  0,136,216,
168,136,136,  0,136,200,168,152,136,  0,248,136,136,136,248,  0,248,
136,248,128,128,  0,248,136,136,144,232,  0,248,136,248,144,136,  0,
248,128,248,  8,248,  0,248, 32, 32, 32, 32,  0,136,136,136,136,248,
  0,136,136,136, 80, 32,  0,136,136,168,216,136,  0,136, 80, 32, 80,
136,  0,136, 80, 32, 32, 32,  0,248, 16, 32, 64,248,  0, 48, 32, 32,
 32, 48,  0,128, 64, 32, 16,  8,  0, 96, 32, 32, 32, 96,  0, 32, 80,
136,  0,  0,  0,  0,  0,  0,  0,248,  0, 77, 90,  0,  1, 23,  0,  0,
  0,  4,  0,180,  6,255,255,215,  3

};

void textpos(int x, int y){
TEXT_X = x; TEXT_Y = y;
}

void textcolor(int c){
TEXT_COLOR=c;
}
// �뢮� ��ப�
void print(char* s){
int i,j,k,p,xp,yp;
  for(i=0; *s!=0; s++){ yp=TEXT_Y; p= (*s-32) * 6;
    for (j=0;j<6;j++) { xp=TEXT_X;
      for (k=0;k<8;k++){ if (fonttable[p] & (128>>k)) putpixel(xp,yp,TEXT_COLOR); xp++; }
       yp++; p++; }
     TEXT_X+=6;
     if (TEXT_X>MAXX) {TEXT_X=0; TEXT_Y += 6; }
    }
     if (TEXT_Y>MAXY) {TEXT_X=0; TEXT_Y = 0;  }
}
//��������� 楫�� �᫮ � ��ப� � �������� ��ﬨ �� l, �᫨ l<>0
char __s[255];
char *int2str(int n, int l, int pos){   //pos - �᭮����� �᫠ (10 ��� 16)
 int i,j,k,dl; char f;
 char sb[255]; char table[]={"0123456789ABCDEF-"};
 k=n; i=0; f=1; if (k<0) {k = -k;}
 while (f){ sb[i]=table[k % pos]; i++; k/=pos; if (k<1) f=0; }
 if (l!=0) {dl=l-i; for (k=0;k<dl;k++) {sb[i]=table[0]; i++; }}
 if (n<0) { sb[i] = table[16]; i++; }
 for (j=0; j<i; j++) __s[j]=sb[(i-j-1)]; __s[i]=0;
 return (__s);
}
// �뢮� 楫��� �᫠
void printn(int n){
 print(int2str(n,0,10));
}
//�뢮� 楫��� �᫠ � �������� ������ ��ப�, ������塞�� ��ﬨ
void printz(int n, int l){
 print(int2str(n,l,10));
}
//�뢮� ��⭠����筮�� �᫠ 00..FF
void printh(int n, int l){
 print(int2str(n,l,16));
}

//�뢮� �᫠ � ������饩 �窮�
//���� ��ࠬ��� - ������⢮ ������ �� ����⮩ (0-���ଠ�஢���� �뢮�)
//��ன ��ࠬ��� - ������⢮ ������ ��᫥ ����⮩
void printfl(float n, int l, int k){
  int i,f;
  i=(int)n;
  f=(int)((n-i)*pow(10.0f,k));
  printz(i,l);
  print(".");
  if (f<0) f=-f;
  printz(f,k);
}







