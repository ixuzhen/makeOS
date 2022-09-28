#include "bootpack.h"
// 绘图相关的处理

void init_palette(void)
{
    static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:黑 */
		0xff, 0x00, 0x00,	/*  1:梁红 */
		0x00, 0xff, 0x00,	/*  2:亮绿 */
		0xff, 0xff, 0x00,	/*  3:亮黄 */
		0x00, 0x00, 0xff,	/*  4:亮蓝 */
		0xff, 0x00, 0xff,	/*  5:亮紫 */
		0x00, 0xff, 0xff,	/*  6:浅亮蓝 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:亮灰 */
		0x84, 0x00, 0x00,	/*  9:暗红 */
		0x00, 0x84, 0x00,	/* 10:暗绿 */
		0x84, 0x84, 0x00,	/* 11:暗黄 */
		0x00, 0x00, 0x84,	/* 12:暗青 */
		0x84, 0x00, 0x84,	/* 13:暗紫 */
		0x00, 0x84, 0x84,	/* 14:浅暗蓝 */
		0x84, 0x84, 0x84	/* 15:暗灰 */
	};
    set_palette(0, 15, table_rgb);
    return;
}

void set_palette(int start, int end, unsigned char *rgb){
    int i, eflags;
    eflags = io_load_eflags();
    io_cli();
    io_out8(0x03c8,start);
    for (i=start; i<=end; i++){
        io_out8(0x03c9, rgb[0]/4);
        io_out8(0x03c9, rgb[1]/4);
        io_out8(0x03c9, rgb[2]/4);
        rgb += 3;
    }
    io_store_eflags(eflags);
    return;
}


void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1){
    int x, y;
    for (y=y0; y<=y1; y++){
        for (x=x0; x<=x1; x++){
            vram[y*xsize + x] = c;
        }
    }
}


// void init_screen(char *vram, int xsize, int ysize)
// {
//     boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
// 	//boxfill8(vram, xsize, COL8_00FFFF,  0,         0,          xsize -  1, ysize - 29);
// 	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
// 	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
// 	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

// 	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
// 	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
// 	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
// 	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
// 	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
// 	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

// 	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
// 	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
// 	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
// 	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);

// }

void init_screen8(char *vram, int xsize, int ysize)
{
    boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
	//boxfill8(vram, xsize, COL8_00FFFF,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);

}


void putfont8(char *vram, int xsize, int x, int y, char c, char *font){
    int i;
    char d,*p;
    for (i=0; i<16; i++){
        p = vram + xsize*(y+i) + x;
        d = font[i];
        if((d&0x80)!=0){ p[0]=c; }
        if((d&0x40)!=0){ p[1]=c; }
        if((d&0x20)!=0){ p[2]=c; }
        if((d&0x10)!=0){ p[3]=c; }
        if((d&0x08)!=0){ p[4]=c; }
        if((d&0x04)!=0){ p[5]=c; }
        if((d&0x02)!=0){ p[6]=c; }
        if((d&0x01)!=0){ p[7]=c; }
    }
    return;
}

// 我写错了,少写了一个s,看下面的
// 打印字符串，vram是显存地址，xsize是每行最大的字符数，x是字符串起始的x坐标，y是坐标y，c是字符颜色，s是要显示的字符串
void putfont8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s){
    extern char hankaku[4096];
    for(; *s!=0x00; s++){
        putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
        x += 8;
    }
    return;
}

// 原代码
// 打印字符串，vram是显存地址，xsize是每行最大的字符数，x是字符串起始的x坐标，y是坐标y，c是字符颜色，s是要显示的字符串
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s){
    extern char hankaku[4096];
    for(; *s!=0x00; s++){
        putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
        x += 8;
    }
    return;
}

// 准备鼠标指针
// mouse 是鼠标位置（左上角），bc是背景颜色
void init_mouse_cursor8(char *mouse, char bc)
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
    int x, y;
    for (y = 0; y < 16; y++) {
        for( x = 0; x < 16; x++) {
            if (cursor[y][x]=='*') {
                mouse[y * 16 + x] = COL8_000000;
            }
            if (cursor[y][x]=='O') {
                mouse[y * 16 + x] = COL8_FFFFFF;
            }
            if (cursor[y][x]=='.') {
                mouse[y * 16 + x] = bc;
            }
        }
    }
    return;
}


// 将buf中的数据放到vram显存中去
// vram是显存地址，vxsize是一整行的全部像素数，pxsize和pysize是想要的图像的大小，px0和py0是想要图形显示的位置
// buf 是要显示的东西存放的地址，bxize是图像一行的大小，bxize和pxsize大体相同，有其他用处

void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}

// void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize){
//     char *p;
//     int i, j;
//     p = vram + vxsize * py0 + px0;
//     for(i=0; i<pysize; i++){
//         p += vxsize;
//         for(j=0; j<pxsize; j++){
//             p[j] = *buf;
//             buf++;
//         }
//     }
// }












