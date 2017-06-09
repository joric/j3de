/*  load JPEG file (baseline)
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <stdio.h>
#include <malloc.h>

typedef struct {    // Huffman coding tables
    unsigned char bits[16];
    unsigned char hval[256];
    unsigned char size[256];
    unsigned short code[256];
} jpeg_huffman_table_t;

struct {            // Jpeg file parameter
    FILE    *file;  // file
    int     width;  // width image
    int     height; // height image
    unsigned char *data;   // image
    int     data_precision; // bit per component
    int     num_components; // number component
    int     restart_interval;   // restart interval
    struct {
        int     id; // identifier
        int     h;  // horizontal sampling factor
        int     v;  // vertical sampling factor
        int     t;  // Quantization table selector
        int     td; // DC table selector
        int     ta; // AC table selector
    } component_info[3];
    jpeg_huffman_table_t hac[4];    // AC table
    jpeg_huffman_table_t hdc[4];    // DC table
    int     qtable[4][64];  // Quantization table
    struct {
        int     ss,se;  // progressive jpeg spectral selection
        int     ah,al;  // progressive jpeg successive approx
    } scan;
    int     dc[3];
    int     curbit;
    unsigned char   curbyte;
} jpeg_file_s;

int jpeg_read_byte() {  // read byte
    fread(&jpeg_file_s.curbyte,1,1,jpeg_file_s.file);
    jpeg_file_s.curbit = 0;
    return jpeg_file_s.curbyte;
}

int jpeg_read_word() {  // read word
    unsigned short i;
    fread(&i,2,1,jpeg_file_s.file);
    i = ((i << 8) & 0xFF00) + ((i >> 8) & 0x00FF);
    return i;
}

int jpeg_read_bit() {   // read bit
    register int i;
    if(jpeg_file_s.curbit == 0) {
        jpeg_read_byte();
        if(jpeg_file_s.curbyte == 0xFF) {
            while(jpeg_file_s.curbyte == 0xFF) jpeg_read_byte();
            if(jpeg_file_s.curbyte >= 0xD0 && jpeg_file_s.curbyte <= 0xD7)
                memset(jpeg_file_s.dc,0,sizeof(int) * 3);
            if(jpeg_file_s.curbyte == 0) jpeg_file_s.curbyte = 0xFF;
            else jpeg_read_byte();
        }
    }
    i = (jpeg_file_s.curbyte >> (7 - jpeg_file_s.curbit++)) & 0x01;
    if(jpeg_file_s.curbit == 8) jpeg_file_s.curbit = 0;
    return i;
}

int jpeg_read_bits(int num) {   // read num bit
    register int i,j;
    for(i = 0, j = 0; i < num; i++) {
        j <<= 1;
        j |= jpeg_read_bit();
    }
    return j;
}

int jpeg_bit2int(int bit,int i) {   // convert bit code to int
    if((i & (1 << (bit - 1))) > 0) return i;
    return -(i ^ ((1 << bit) - 1));
}

int jpeg_huffmancode(jpeg_huffman_table_t *table) { // get Huffman code
    register int i,size,code;
    for(size = 1, code = 0, i = 0; size < 17; size++) {
        code <<= 1;
        code |= jpeg_read_bit();
        while(table->size[i] <= size) {
            if(table->code[i] == code) return table->hval[i];
            i++;
        }
    }
    return code;
}

void jpeg_idct(float *data) {   // aa&n algorithm inverse DCT
    float   t0,t1,t2,t3,t4,t5,t6,t7;
    float   t10,t11,t12,t13;
    float   z5,z10,z11,z12,z13;
    float   *dataptr;
    int i;
    dataptr = data;
    for(i = 0; i < 8; i++) {
        t0 = dataptr[8 * 0];
        t1 = dataptr[8 * 2];
        t2 = dataptr[8 * 4];
        t3 = dataptr[8 * 6];
        t10 = t0 + t2;
        t11 = t0 - t2;
        t13 = t1 + t3;
        t12 = - t13 + (t1 - t3) * 1.414213562;
        t0 = t10 + t13;
        t3 = t10 - t13;
        t1 = t11 + t12;
        t2 = t11 - t12;
        t4 = dataptr[8 * 1];
        t5 = dataptr[8 * 3];
        t6 = dataptr[8 * 5];
        t7 = dataptr[8 * 7];
        z13 = t6 + t5;
        z10 = t6 - t5;
        z11 = t4 + t7;
        z12 = t4 - t7;
        t7 = z11 + z13;
        t11 = (z11 - z13) * 1.414213562;
        z5 = (z10 + z12) * 1.847759065;
        t10 = - z5 + z12 * 1.082392200;
        t12 = z5 - z10 * 2.613125930;
        t6 = t12 - t7;
        t5 = t11 - t6;
        t4 = t10 + t5;
        dataptr[8 * 0] = t0 + t7;
        dataptr[8 * 7] = t0 - t7;
        dataptr[8 * 1] = t1 + t6;
        dataptr[8 * 6] = t1 - t6;
        dataptr[8 * 2] = t2 + t5;
        dataptr[8 * 5] = t2 - t5;
        dataptr[8 * 4] = t3 + t4;
        dataptr[8 * 3] = t3 - t4;
        dataptr++;
    }
    dataptr = data;
    for(i = 0; i < 8; i++) {
        t10 = dataptr[0] + dataptr[4];
        t11 = dataptr[0] - dataptr[4];
        t13 = dataptr[2] + dataptr[6];
        t12 = - t13 + (dataptr[2] - dataptr[6]) * 1.414213562;
        t0 = t10 + t13;
        t3 = t10 - t13;
        t1 = t11 + t12;
        t2 = t11 - t12;
        z13 = dataptr[5] + dataptr[3];
        z10 = dataptr[5] - dataptr[3];
        z11 = dataptr[1] + dataptr[7];
        z12 = dataptr[1] - dataptr[7];
        t7 = z11 + z13;
        t11 = (z11 - z13) * 1.414213562;
        z5 = (z10 + z12) * 1.847759065;
        t10 = - z5 + z12 * 1.082392200;
        t12 = z5 - z10 * 2.613125930;
        t6 = t12 - t7;
        t5 = t11 - t6;
        t4 = t10 + t5;
        dataptr[0] = t0 + t7;
        dataptr[7] = t0 - t7;
        dataptr[1] = t1 + t6;
        dataptr[6] = t1 - t6;
        dataptr[2] = t2 + t5;
        dataptr[5] = t2 - t5;
        dataptr[4] = t3 + t4;
        dataptr[3] = t3 - t4;
        dataptr += 8;
    }
}

int jpeg_readmarkers() {    // read jpeg markers
    int marker,length,i,j,k,l,m;
    jpeg_huffman_table_t *hptr;
    for(;;) {
        marker = jpeg_read_byte();
        if(marker != 0xFF) return 0;
        marker = jpeg_read_byte();
        if(marker != 0xD8) {
            length = jpeg_read_word();
            length -= 2;
            switch(marker) {
                case 0xC0:  // Baseline
                    jpeg_file_s.data_precision = jpeg_read_byte();
                    jpeg_file_s.height = jpeg_read_word();
                    jpeg_file_s.width = jpeg_read_word();
                    jpeg_file_s.num_components = jpeg_read_byte();
                    if(length - 6 != jpeg_file_s.num_components * 3) return 0;
                    for(i = 0; i < jpeg_file_s.num_components; i++) {
                        jpeg_file_s.component_info[i].id = jpeg_read_byte();
                        j = jpeg_read_byte();
                        jpeg_file_s.component_info[i].h = (j >> 4) & 0x0F;
                        jpeg_file_s.component_info[i].v = j & 0x0F;
                        jpeg_file_s.component_info[i].t = jpeg_read_byte();
                    }
                    break;
                case 0xC1:  // Extended sequetial, Huffman
                case 0xC2:  // Progressive, Huffman            
                case 0xC3:  // Lossless, Huffman
                case 0xC5:  // Differential sequential, Huffman
                case 0xC6:  // Differential progressive, Huffman
                case 0xC7:  // Differential lossless, Huffman
                case 0xC8:  // Reserved for JPEG extensions
                case 0xC9:  // Extended sequential, arithmetic
                case 0xCA:  // Progressive, arithmetic
                case 0xCB:  // Lossless, arithmetic
                case 0xCD:  // Differential sequential, arithmetic
                case 0xCE:  // Differential progressive, arithmetic
                case 0xCF:  // Differential lossless, arithmetic
                    return 0;
                    break;
                case 0xC4:  // Huffman table
                    while(length > 0) {
                        k = jpeg_read_byte();
                        if(k & 0x10) hptr = &jpeg_file_s.hac[k & 0x0F];
                        else hptr = &jpeg_file_s.hdc[k & 0x0F];
                        for(i = 0, j = 0; i < 16; i++) {
                            hptr->bits[i] = jpeg_read_byte();
                            j += hptr->bits[i];
                        }
                        length -= 17;
                        for(i = 0; i < j; i++)
                            hptr->hval[i] = jpeg_read_byte();
                        length -= j;
                        for(i = 0, k = 0, l = 0; i < 16; i++) {
                            for(j = 0; j < hptr->bits[i]; j++, k++) {
                                hptr->size[k] = i + 1;
                                hptr->code[k] = l++;
                            }
                            l <<= 1;
                        }
                    }
                    break;
                case 0xDB:  // Quantization table
                    while(length > 0) {
                        j = jpeg_read_byte();
                        k = (j >> 4) & 0x0F;
                        for(i = 0; i < 64; i++) {
                            if(k) jpeg_file_s.qtable[j][i] = jpeg_read_word();
                            else  jpeg_file_s.qtable[j][i] = jpeg_read_byte();
                        }
                        length -= 65;
                        if(k) length -= 64;
                    }
                    break;
                case 0xD9:  // End of image (EOI)
                    return 0;
                    break;
                case 0xDA:  // Start of scan (SOS)
                    j = jpeg_read_byte();
                    for(i = 0; i < j; i++) {
                        k = jpeg_read_byte();
                        m = jpeg_read_byte();
                        for(l = 0; l < jpeg_file_s.num_components; l++)
                            if(jpeg_file_s.component_info[l].id == k) {
                                jpeg_file_s.component_info[l].td = (m >> 4) & 0x0F;
                                jpeg_file_s.component_info[l].ta = m & 0x0F;
                            }
                    }
                    jpeg_file_s.scan.ss = jpeg_read_byte();
                    jpeg_file_s.scan.se = jpeg_read_byte();
                    k = jpeg_read_byte();
                    jpeg_file_s.scan.ah = (k >> 4) & 0x0F;
                    jpeg_file_s.scan.al = k & 0x0F;
                    return 1;
                    break;
                case 0xDD:  // Restart interval
                    jpeg_file_s.restart_interval = jpeg_read_word();
                    break;
                default:
                    fseek(jpeg_file_s.file,length,SEEK_CUR);
                    break;
            }
        }
    }
}

void jpeg_decompress() {// decompress jpeg file (Baseline algorithm)
    int x,y,i,j,k,l,c;
    int X,Y,H,V,plane,scaleh[3],scalev[3];
    static float vector[64],dct[64];
    static const int jpeg_zigzag[64] = {
         0, 1, 5, 6,14,15,27,28,
         2, 4, 7,13,16,26,29,42,
         3, 8,12,17,25,30,41,43,
         9,11,18,24,31,40,44,53,
        10,19,23,32,39,45,52,54,
        20,22,33,38,46,51,55,60,
        21,34,37,47,50,56,59,61,
        35,36,48,49,57,58,62,63 };
    static const float aanscale[8] = {  // 1.0, k = 0; cos(k * PI / 16) * sqrt(2), k = 1...7
        1.0, 1.387039845, 1.306562965, 1.175875602,
        1.0, 0.785694958, 0.541196100, 0.275899379 };
    scaleh[0] = 1;
    scalev[0] = 1;
    if(jpeg_file_s.num_components == 3) {
        scaleh[1] = jpeg_file_s.component_info[0].h / jpeg_file_s.component_info[1].h;
        scalev[1] = jpeg_file_s.component_info[0].v / jpeg_file_s.component_info[1].v;
        scaleh[2] = jpeg_file_s.component_info[0].h / jpeg_file_s.component_info[2].h;
        scalev[2] = jpeg_file_s.component_info[0].v / jpeg_file_s.component_info[2].v;
    }
    memset(jpeg_file_s.dc,0,sizeof(int) * 3);
    for(Y = 0; Y < jpeg_file_s.height; Y += jpeg_file_s.component_info[0].v << 3) {
        if(jpeg_file_s.restart_interval > 0) jpeg_file_s.curbit = 0;
        for(X = 0; X < jpeg_file_s.width; X += jpeg_file_s.component_info[0].h << 3) {
            for(plane = 0; plane < jpeg_file_s.num_components; plane++)
                for(V = 0; V < jpeg_file_s.component_info[plane].v; V++)
                    for(H = 0; H < jpeg_file_s.component_info[plane].h; H++) {
                        i = jpeg_huffmancode(&jpeg_file_s.hdc[jpeg_file_s.component_info[plane].td]);
                        i &= 0x0F;
                        vector[0] = jpeg_file_s.dc[plane] + jpeg_bit2int(i,jpeg_read_bits(i));
                        jpeg_file_s.dc[plane] = vector[0];
                        i = 1;
                        while(i < 64) {
                            j = jpeg_huffmancode(&jpeg_file_s.hac[jpeg_file_s.component_info[plane].ta]);
                            if(j == 0) while(i < 64) vector[i++] = 0;
                            else {
                                k = i + ((j >> 4) & 0x0F);
                                while(i < k) vector[i++] = 0;
                                j &= 0x0F;
                                vector[i++] = jpeg_bit2int(j,jpeg_read_bits(j));
                            }
                        }
                        k = jpeg_file_s.component_info[plane].t;
                        for(y = 0, i = 0; y < 8; y++)
                            for(x = 0; x < 8; x++, i++) {
                                j = jpeg_zigzag[i];
                                dct[i] = vector[j] * jpeg_file_s.qtable[k][j] *
                                    aanscale[x] * aanscale[y];
                            }
                        jpeg_idct(dct);
                        for(y = 0; y < 8; y++)
                            for(x = 0; x < 8; x++) {
                                c = ((int)dct[(y << 3) + x] >> 3) + 128;
                                if(c < 0) c = 0;
                                else if(c > 255) c = 255;
                                if(scaleh[plane] == 1 && scalev[plane] == 1) {
                                    i = X + x + (H << 3);
                                    j = Y + y + (V << 3);
                                    if(i < jpeg_file_s.width && j < jpeg_file_s.height)
                                        jpeg_file_s.data[((j * jpeg_file_s.width + i) << 2) + plane] = c;
                                }
                                else for(l = 0; l < scalev[plane]; l++)
                                    for(k = 0; k < scaleh[plane]; k++) {
                                        i = X + (x + (H << 3)) * scaleh[plane] + k;
                                        j = Y + (y + (V << 3)) * scalev[plane] + l;
                                        if(i < jpeg_file_s.width && j < jpeg_file_s.height)
                                            jpeg_file_s.data[((j * jpeg_file_s.width + i) << 2) + plane] = c;
                                }
                            }
                    }
        }
    }
}

void jpeg_ycbcr2rgb() { // convert YCbCr image to RGB
    int i,Y,Cb,Cr,R,G,B;
    for(i = 0; i < jpeg_file_s.width * jpeg_file_s.height << 2; i += 4) {
        Y = jpeg_file_s.data[i];
        Cb = jpeg_file_s.data[i + 1] - 128;
        Cr = jpeg_file_s.data[i + 2] - 128;
        R = Y                + 1.40200 * Cr;
        G = Y - 0.34414 * Cb - 0.71414 * Cr;
        B = Y + 1.77200 * Cb;
        if(R < 0) R = 0;
        else if(R > 255) R = 255;
        if(G < 0) G = 0;
        else if(G > 255) G = 255;
        if(B < 0) B = 0;
        else if(B > 255) B = 255;
        jpeg_file_s.data[i] = R;
        jpeg_file_s.data[i + 1] = G;
        jpeg_file_s.data[i + 2] = B;
        jpeg_file_s.data[i + 3] = 255;
    }
}

void jpeg_gray2rgb() {  // grayscale image to RGB
    int i,j;
    for(i = 0; i < jpeg_file_s.width * jpeg_file_s.height << 2; i += 4) {
        j = jpeg_file_s.data[i];
        jpeg_file_s.data[i + 1] = j;
        jpeg_file_s.data[i + 2] = j;
        jpeg_file_s.data[i + 3] = 255;
    }
}

unsigned char *LoadJPEG(char *name,int *width,int *height) {
    memset(&jpeg_file_s,0,sizeof(jpeg_file_s));
    jpeg_file_s.file = fopen(name,"rb");
    if(!jpeg_file_s.file) return 0;
    if(!jpeg_readmarkers()) return 0;    
    jpeg_file_s.data = (unsigned char*)malloc(jpeg_file_s.width * jpeg_file_s.height << 2);
    if(!jpeg_file_s.data) return 0;
    jpeg_decompress();
    if(jpeg_file_s.num_components == 1) jpeg_gray2rgb();        
    if(jpeg_file_s.num_components == 3) jpeg_ycbcr2rgb();
    fclose(jpeg_file_s.file);
    *width = jpeg_file_s.width;
    *height = jpeg_file_s.height;
    return jpeg_file_s.data;
}
