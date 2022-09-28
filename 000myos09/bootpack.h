// asmhead.nas
struct BOOTINFO {
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    char *vram;
};
#define ADR_BOOTINFO	0x00000ff0

// naskfunc.nas
void io_hlt(void);
void io_cli(void);
void io_stihlt(void);
void io_out8(int port, int data);
int io_in8(int port);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
// void write_mem8(int addr, int data);

void asm_inthandler27(void);
void store_cr0(int cr0);
int load_cr0(void);
unsigned int memtest_sub(unsigned int start, unsigned int end);



// graphic.c
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen8(char *vram, int xsize, int ysize);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
// 打印字符串，vram是显存地址，xsize是每行最大的字符数，x是字符串起始的x坐标，y是坐标y，c是字符颜色，s是要显示的字符串
void putfont8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
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
// 008484是背景色
#define COL8_008484		14
#define COL8_848484		15

// dsctal.c


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

// GDT 和 IDT 相关的变量
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e


/* int.c */

void init_pic(void);
void inthandler21(int *esp);
void inthandler2c(int *esp);
void inthandler27(int *esp);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1



// fifo.c
// 初始化缓冲区
// 键盘缓冲区
struct FIFO8 {
    unsigned char *buf;
    /*
        p:代表下一个写入的地址
        q:代表下一个读入的地址
        size:代表缓冲的总字节数,容量
        free:代表缓冲区里没有数据的字节数
        flag:用来记录是否溢出
    */
    int p,q,size,free,flags;
};
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
// 往缓冲区写数据,return 0表示成功,return -1表示缓冲区溢出
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
// 从缓冲区读数据,会把数据return出去,如果return -1,表示缓冲区没有数据.
int fifo8_get(struct FIFO8 *fifo);
// 查看缓冲区里数据的个数
int fifo8_status(struct FIFO8 *fifo);



// keyboard.c
#define PORT_KEYDAT				0x0060
#define PORT_KEYCMD				0x0064
extern struct FIFO8 keyfifo;
void asm_inthandler21(void);
void wait_KBC_sendready(void);
void init_keyboard(void);


// mouse.c
struct MOUSE_DEC {
    unsigned char buf[3], phase;
    int x, y, btn;
};
extern struct FIFO8 mousefifo;
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
void asm_inthandler2c(void);
