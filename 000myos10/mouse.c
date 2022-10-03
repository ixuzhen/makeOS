#include "bootpack.h"

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

// 鼠标中断
struct FIFO8 mousefifo;
void inthandler2c(int *esp){
    unsigned char data;
	io_out8(PIC1_OCW2, 0x64);		// 通知PIC的"IRQ-12已经受理完毕
	io_out8(PIC0_OCW2, 0x62);		// 通知PIC的"IRQ-02已经受理完毕
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
    return;
}


