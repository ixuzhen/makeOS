#include "bootpack.h"


/*
    该数据结构表示一个图层的信息
    buf：是用来记录图层上所描画的内容的地址
    bxsize*bysize:表示该图层的大小
    vx0和vy0：表示图层在画面上的位置坐标
    col_inv:表示透明色色号
    height：表示图层高度
    flags:用于存放有关图层的各种设定信息
*/
struct SHEET {
    unsigned char *buf;
    int bxsize, bysize, vx0, vy0, col_inv, height, flags;
};

/*
    管理图层的数据结构sheet control
    MAX_SHEETS是能够管理的最大图层数
    vram、xsize、ysize代表VRAM的地址和画面的大小
    top代表最上面图层的高度
    
*/
#define MAX_SHEETS        256
struct SHTCTL {
    unsigned char *vram;
    int xsize, ysize, top;
    struct SHEET *sheets[MAX_SHEETS];
    struct SHEET sheets0[MAX_SHEETS];
};

// 初始化SHEET control,并分配内存
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize) {
    struct SHTCTL *ctl;
    int i;
    ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if (ctl == 0) {
        goto err;
    }
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1;      // 一个SHEET都没有
    for (i = 0; i < MAX_SHEETS; i++) {
        ctl->sheets0[i].flags = 0;      // 标记为可以使用
    }

    err:
    return ctl;
}

// 这道一个没有使用的sheet
#define SHEET_USE 1
struct SHEET *sheet_alloc(struct SHTCTL *ctl) {
    struct SHEET *sht;
    int i;
    for (int i = 0; i < MAX_SHEETS; i++) {
        if (ctl->sheets0[i].flags == 0) {
            sht = &(ctl->sheets0[i]);
            sht->flags = SHEET_USE;     // 标记为正在使用
            sht->height = -1;
            return sht;
        }
    }
    return 0;
}

// 初始化这个sheet
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv) {
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
    return;
}

// 设定底板的高度
void sheet_updown(struct SHRCTL *ctl, struct SHEET *sht, int height){
    
}




