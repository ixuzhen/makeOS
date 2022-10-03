
#include "bootpack.h"

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

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size){
    unsigned int a;
    size = (size + 0xfff) & 0xfffff000;
    a = memman_alloc(man, size);
    return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size){
    int i;
    size = (size + 0xfff) & 0xfffff000;
    i = memman_free(man, addr ,size);
    return i;
}



