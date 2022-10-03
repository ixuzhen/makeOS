#include "bootpack.h"

#define FLAGS_OVERRUN 0x0001

// 初始化缓冲区
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf){
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->p = 0;
    fifo->q = 0;
}

// 往缓冲区写数据,return 0表示成功,return -1表示缓冲区溢出
int fifo8_put(struct FIFO8 *fifo, unsigned char data){
    if (fifo->free == 0){
        // 说明没空间了,溢出
        fifo->flags |= FLAGS_OVERRUN;
        return -1;
    }
    fifo->buf[fifo->p] = data;
    fifo->p++;
    if(fifo->p == fifo->size){
        fifo->p = 0;
    }
    fifo->free--;
    return 0;
}

// 从缓冲区读数据,会把数据return出去,如果return -1,表示缓冲区没有数据.
int fifo8_get(struct FIFO8 *fifo){
    int data;
    if(fifo->free == fifo->size){
        // 缓冲区没数据
        return -1;
    }
    data = fifo->buf[fifo->q];
    fifo->q++;
    if(fifo->q == fifo->size){
        fifo->q = 0;
    }
    fifo->free++;
    return data;
}

// 查看缓冲区里数据的个数
int fifo8_status(struct FIFO8 *fifo){
    return fifo->size - fifo->free;
}


