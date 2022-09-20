#include <stdio.h>
#include "bootpack.h"


void HariMain(void)
{   
    // 读显示的模式的数据
    struct BOOTINFO *binfo;
    binfo = (struct BOOTINFO *) ADR_BOOTINFO;
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




