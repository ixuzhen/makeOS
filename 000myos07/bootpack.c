#include <stdio.h>
#include "bootpack.h"


extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

struct MOUSE_DEC {
    unsigned char buf[3], phase;
    int x, y, btn;
};

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




#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

// 等待键盘控制电路准备完毕
void wait_KBC_sendready(void){
    for(;;){
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
    }
    return;
}

// 初始化键盘的控制电路
void init_keyboard(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

// 激活鼠标,
void enable_mouse(struct MOUSE_DEC *mdec)
{

	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    // 激活顺利的话,键盘控制会返回ACK(0xfa)
    mdec->phase = 0;
    
	return;
}

// 解码鼠标传来的数据, return 0表示鼠标数据还没读完,鼠标数据3个一组
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat){
    // putfont8_asc(binfo->vram, binfo->scrnx, 100, 100, COL8_FFFFFF, "test01");
    if (mdec->phase == 0) {
        
        // 等待鼠标发送0xfa,表示鼠标已经准备完毕
        if (dat == 0xfa){
            mdec->phase = 1;
            
        }
        return 0;
    }
    if(mdec->phase == 1){
        
        // 等待鼠标的第一个字节
        // 只有dat符合00xx 1xxx 这样才是符合规范的第一个字节,
        if((dat & 0xc8) == 0x08){
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return 0;
    }
    if(mdec->phase == 2){
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return 0;
    }
    if(mdec->phase == 3){
        mdec->buf[2] = dat;
        mdec->phase = 1;
        mdec->btn = mdec->buf[0] & 0x07;
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];
        if((mdec->buf[0] & 0x10) != 0){
            mdec->x |= 0xffffff00;
        }
        if((mdec->buf[0] & 0x20) != 0){
            mdec->y |= 0xffffff00;
        }
        // 鼠标的y方向与画面的符号方向相反.
        if(mdec->y = -mdec->y);
        return 1;
    }
    return -1;
}

