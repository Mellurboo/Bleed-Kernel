#include <stdint.h>
#include <tss/tss.h>
#include <gdt/gdt.h>
#include <stddef.h>

TSS tss = {0};

void tss_init(){
    TSSSegment* tss_segment = &gdt.tss;
    tss_segment->limit_low = sizeof(TSS)-1;
    uint64_t tss_ptr = (uint64_t)&tss;
    tss_segment->base_low    = tss_ptr;
    tss_segment->base_middle = tss_ptr>>16;
    tss_segment->base_high   = tss_ptr>>24;
    tss_segment->base_high2  = tss_ptr>>32;
    tss_segment->access = 0x89;
    tss_segment->_reserved = 0;
    tss_segment->limit_flags = 0x20;
    __asm__ volatile(
        "ltr %0"
        :
        : "r" ((uint16_t)offsetof(gdt_t, tss)) // Offset within the GDT
    );
}