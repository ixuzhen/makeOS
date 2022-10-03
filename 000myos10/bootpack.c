#include <stdio.h>
#include "bootpack.h"


#define MEMMAN_ADDR 0x003c0000

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

    // 内存管理
    unsigned int memtotal;
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

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
    // 激活鼠标
    enable_mouse(&mdec);

    // 初始化管理内存的东西
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);


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

    

    // boxfill8(binfo->vram, binfo->scrnx, COL8_FF0000,  0, 10, 30, 40);
    // 显示内存
    sprintf(s, "memory %dMB free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
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

// 内存管理
#define MEMMAN_FREES 4090       // 4090项，32KB

// 可用信息
struct FREEINFO {
    unsigned int addr, size;
};

// 管理内存
struct MEMMAN{
    int frees, maxfrees, lostsize, losts;
    struct FREEINFO free[MEMMAN_FREES];
};

void memman_init(struct MEMMAN *man){
    man->frees = 0;    /* 可用信息数目 */
	man->maxfrees = 0; /* 用于观察可用状况：frees的最大值 */
	man->lostsize = 0; /* 释放失败的内存的大小总和 */
	man->losts = 0;    /* 释放失败次数 */
	return;
}

unsigned int memman_total(struct MEMMAN *man){
    unsigned int i, t = 0;
    for(i=0; i<man->frees; i++){
        t += man->free[i].size;
    }
    return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size){
    unsigned int i, a;
    for (i=0; i<man->frees; i++){
        if(man->free[i].size >= size){
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if (man->free[i].size == 0){
                man->frees--;
                for(; i<man->frees; i++){
                    man->free[i] = man->free[i+1];      // 将表往前移
                }
            }
            return a;
        }
    }
    return 0;
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size){
    int i, j;
    // free[]按照内存顺序管理内存
    for(i=0; i<man->frees; i++){
        if(man->free[i].addr > addr){
            break;
        }
    }
    if(i>0){
        // 如果可以和前边的合并内存
        if(man->free[i-1].addr + man->free[i-1].size == addr){
            man->free[i-1].size += size;
            if(i < man->frees){
                // 看看后边可不可以合并
                if(addr + size == man->free[i].addr){
                    man->free[i-1].size += man->free[i].size;
                    man->frees--;
                    for(; i<man->frees; i++){
                        man->free[i] = man->free[i+1];
                    }
                }
            }
            return 0;
        }
    }
    // 如果不可以和前边合并，就看看可不可以和后边合并
    if(i < man->frees){
        if(addr + size == man->free[i].addr){
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0;
        }
    }
    // 既不能和前边合并也不能和后边合并
    if(man->frees < MEMMAN_FREES){
        for(j = man->free; j>i ;j--){
            man->free[j] = man->free[j-1];
        }
        man->frees++;
        if(man->maxfrees < man->frees){
            man->maxfrees = man->frees;
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }
    // 放不进去了
    man->losts++;
    man->lostsize += size;
    return -1;
}






