#include "bootpack.h"


/*
    初始化GDT和IDT表
    GDT地址:0x270000 ~ 0x27ffff
        0x27ffff - 0x270000 + 1 = 10000 = 25536
    GDT:有8192个段号,一个段号信息有8字节
        8192 * 8 = 25536
    IDT表地址:0x26f800 ~ 0x26ffff
        0x26ffff - 0x26f800 + 1 = 800 = 2048
    IDT:有256个中断号,一个中断号信息有8字节
        256 * 8 = 2048
*/
void init_gdtidt(void){
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) ADR_IDT;
    int i;
    // GDT表都初始化为0
    for (i=0; i<8192; i++){
        set_segmdesc(gdt+i,0,0,0);
    }
    // 初始化GDT的前两项,0xffffffff是段大小,0x00000000是段地址,0x4092是段的属性
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
    set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
    load_gdtr(LIMIT_GDT, ADR_GDT);

    // IDT初始化
    for (i=0; i<256; i++){
        set_gatedesc(idt+i, 0,0,0);
    }
    load_idtr(LIMIT_IDT, ADR_IDT);
    // IDT设置
    set_gatedesc(idt+0x21, (int) asm_inthandler21, 2*8, AR_INTGATE32);
    set_gatedesc(idt+0x27, (int) asm_inthandler27, 2*8, AR_INTGATE32);
    set_gatedesc(idt+0x2c, (int) asm_inthandler2c, 2*8, AR_INTGATE32);

    return;

}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar){
    if (limit>0xfffff){
        ar |= 0x8000;       // G_bit = 1
        limit /= 0x1000;
    }
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) &0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
    return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar){
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar>>8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}
