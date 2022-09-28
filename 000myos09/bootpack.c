#include <stdio.h>
#include "bootpack.h"




// 测试使用
struct BOOTINFO *testbinfo;

void HariMain(void)
{   
    // 读显示的模式的数据
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    // 测试使用
    testbinfo = binfo;
    int i ,mx, my;
    char keybuf[32],mousebuf[128], mcursor[256],s[40];
    // 鼠标信息
    struct MOUSE_DEC mdec;
    // 初始化GDT,IDT
    init_gdtidt();
    //初始化PIC
    init_pic();
    io_sti(); /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */
    
    fifo8_init(&keyfifo, 32, keybuf);
    fifo8_init(&mousefifo, 128, mousebuf);
    io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) */
    // 初始化键盘和鼠标的控制电路
    init_keyboard();
    // 初始化面板
    init_palette();
    // 初始化屏幕显示的内容
    init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
 
   
    // 计算画面的中心
    mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram,binfo->scrnx, 16, 16, mx, my, mcursor, 16);
    // 再屏幕上随便显示一点字符串

    sprintf(s, "(%d, %d)", mx, my);
    putfont8_asc(binfo->vram, binfo->scrnx, 0,0,COL8_FFFFFF,s);

    // 激活鼠标
    enable_mouse(&mdec);

    // boxfill8(binfo->vram, binfo->scrnx, COL8_FF0000,  0, 10, 30, 40);
    // 显示内存
    i = memtest(0x00400000, 0xbfffffff) / (1024*1024);
    sprintf(s, "memory %dMB", i);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF,s);
    
    for(;;){
        io_cli();       // 关闭中断
        if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
            io_stihlt();        // 打开中断并睡眠
        }else{
            if (fifo8_status(&keyfifo) != 0){
                i = fifo8_get(&keyfifo);
                io_sti();       // 打开中断
                sprintf(s, "%02X", i);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
                putfont8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
            } else if (fifo8_status(&mousefifo) != 0){
                i = fifo8_get(&mousefifo);
                io_sti();       // 打开中断
                if(mouse_decode(&mdec, i) != 0){
                    sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
                    if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
                    // 打印鼠标位置信息
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32+ 15*8 -1, 31);
                    putfont8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
                    // 显示鼠标
                    // 先隐藏信息
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx+15, my+15);
                    mx += mdec.x;
                    my += mdec.y;
                    if(mx<0){
                        mx = 0;
                    }
                    if(my<0){
                        my = 0;
                    }
                    if(mx > binfo->scrnx - 16){
                        mx = binfo->scrnx - 16;
                    }
                    if(my > binfo->scrny - 16){
                        my = binfo->scrny - 16;
                    }
                    sprintf(s, "(%3d, %3d)", mx, my);
                    // 隐藏旧鼠标坐标
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                    // 显示新鼠标坐标
                    putfont8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
                    // 在新的位置画鼠标
                    putblock8_8(binfo->vram,binfo->scrnx, 16, 16, mx, my, mcursor, 16);
                    
                }
                
            }

        }

    }

}



#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000


unsigned int memtest(unsigned int start, unsigned int end) {
    char flg486 = 0;
    unsigned int eflg, cr0, i;
    // 确定CPUT是386还是486以上的
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;      
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if((eflg & EFLAGS_AC_BIT) != 0){
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT;     // 将eflg寄存器的ac置0，为什莫还没讲
    io_store_eflags(eflg);
    // 如果是486就禁用缓存
    if(flg486 != 0){
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;   // 禁用缓存
        store_cr0(cr0);   
    }
    i = memtest_sub(start, end);
    if(flg486 != 0){
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;  // 允许缓存
        store_cr0(cr0);
    }
    return i;

}






