#include <stdio.h>
#include "bootpack.h"


void HariMain(void)
{   
    // 读显示的模式的数据
    struct BOOTINFO *binfo;
    binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    // 初始化GDT,IDT
    init_gdtidt();
    //初始化PIC
    init_pic();
    io_sti(); /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */
    // 初始化面板
    init_palette();
    // 初始化屏幕显示的内容
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    
    // 初始化鼠标
    char mcursor[256];
    int mx, my;
    // 计算画面的中心
    mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram,binfo->scrnx, 16, 16, mx, my, mcursor, 16);
    // 再屏幕上随便显示一点字符串
    char s[40];
    sprintf(s, "(%d, %d)", mx, my);
    putfont8_asc(binfo->vram, binfo->scrnx, 0,0,COL8_FFFFFF,s);

    // boxfill8(binfo->vram, binfo->scrnx, COL8_FF0000,  0, 10, 30, 40);

    io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) */
    
    for(;;){
        io_hlt();       // 执行naskfunc.nas的_io_hlt()
    }

}




