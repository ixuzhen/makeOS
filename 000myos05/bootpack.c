#include <stdio.h>

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
// void write_mem8(int addr, int data);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(char *vram, int xsize, int ysize);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
// 打印字符串，vram是显存地址，xsize是每行最大的字符数，x是字符串起始的x坐标，y是坐标y，c是字符颜色，s是要显示的字符串
void putfont8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
// 准备鼠标指针
// mouse 是鼠标位置（左上角），bc是背景颜色
void init_mouse_cursor8(char *mouse, char bc);
// 将buf中的数据放到vram显存中去
// vram是显存地址，vxsize是一整行的全部像素数，pxsize和pysize是想要的图像的大小，px0和py0是想要图形显示的位置
// buf 是要显示的东西存放的地址，bxize是图像一行的大小，bxize和pxsize大体相同，有其他用处
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);


#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

struct BOOTINFO {
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    char *vram;
};

/*
    GDT的数据结构：全局段号记录表，8字节，cpu规定的
*/
struct SEGMENT_DESCRIPTOR{
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};

// //IDT的数据结构：中断记录表
struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);



void HariMain(void)
{   
    // 读显示的模式的数据
    struct BOOTINFO *binfo;
    binfo = (struct BOOTINFO *)0x0ff0;
    // 初始化GDT,IDT
    init_gdtidt();
    // 初始化面板
    init_palette();
    // 初始化屏幕显示的内容
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    // 再屏幕上随便显示一点字符串
    char s[40];
    sprintf(s, "scrnx = %d", binfo->scrnx);
    putfont8_asc(binfo->vram, binfo->scrnx, 8,8,COL8_FF0000,s);
    putfont8_asc(binfo->vram, binfo->scrnx, 31,31,COL8_000000,"Hello OS .");
    putfont8_asc(binfo->vram, binfo->scrnx, 30,30,COL8_FFFFFF,"Hello OS .");
    // 初始化鼠标
    char mcursor[256];
    int mx, my;
    // 计算画面的中心
    mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram,binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    // boxfill8(binfo->vram, binfo->scrnx, COL8_FF0000,  0, 10, 30, 40);
    
    for(;;){
        io_hlt();       // 执行naskfunc.nas的_io_hlt()
    }

}


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


void init_screen(char *vram, int xsize, int ysize)
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
    char d;
    char *p;
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


// 打印字符串，vram是显存地址，xsize是每行最大的字符数，x是字符串起始的x坐标，y是坐标y，c是字符颜色，s是要显示的字符串
void putfont8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s){
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
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize){
    char *p;
    int i, j;
    p = vram + vxsize * py0 + px0;
    for(i=0; i<pysize; i++){
        p += vxsize;
        for(j=0; j<pxsize; j++){
            p[j] = *buf;
            buf++;
        }
    }
}

/*
    初始化GDT和IDT表
    GDT地址:0x270000 ~ 0x27ffff
        0x27ffff - 0x270000 + 1 = 10000 = 25536
    GDT:有8192个段号,一个段号信息有8字节
        8192 * 8 = 25536
    IDT表地址:0x26f800 ~ 0x26ffff
        0x26ffff - 0x26f800 + 1 = 800 = 2048
    IDT:有256个中断号,一个中断号信息有8字节
        256 * 8 = 2048
*/
void init_gdtidt(void){
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) 0x0026f800;
    int i;
    // GDT表都初始化为0
    for (i=0; i<8192; i++){
        set_segmdesc(gdt+i,0,0,0);
    }
    // 初始化GDT的前两项,0xffffffff是段大小,0x00000000是段地址,0x4092是段的属性
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
    load_gdtr(0xffff, 0x00270000);

    // IDT初始化
    for (i=0; i<256; i++){
        set_gatedesc(idt+i, 0,0,0);
    }
    load_idtr(0x7ff, 0x0026f800);


}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar){
    if (limit>0xfffff){
        ar |= 0x8000;       // G_bit = 1
        limit /= 0x1000;
    }
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) &0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
    return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar){
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar>>8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}


