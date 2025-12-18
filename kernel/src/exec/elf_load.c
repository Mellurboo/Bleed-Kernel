#include <stdint.h>
#include <stdio.h>
#include <drivers/serial/serial.h>
#include <exec/elf_load.h>
#include <ansii.h>

int elf_verify(ELF32_EHDR *header){
    if (!header) return 0;
    if (header->e_ident[EI_MAG0] != ELFMAG0){
        kprintf(LOG_ERROR "ELF Header EI_MAG0 is invalid\n");
        return 0;
    }

    if (header->e_ident[EI_MAG1] != ELFMAG1){
        kprintf(LOG_ERROR "ELF Header EI_MAG1 is invalid\n");
        return 0;
    }

    if (header->e_ident[EI_MAG2] != ELFMAG2){
        kprintf(LOG_ERROR "ELF Header EI_MAG2 is invalid\n");
        return 0;
    }

    if (header->e_ident[EI_MAG3] != ELFMAG3){
        kprintf(LOG_ERROR "ELF Header EI_MAG3 is invalid\n");
        return 0;
    }

    return 1;
}

int elf_is_supported(ELF32_EHDR *header){
    if (!elf_verify(header)){
        kprintf(LOG_ERROR "This is not a valid ELF file!");
        return 0;
    }

    if (header->e_ident[EI_CLASS] != ELFCLASS32){
        kprintf(LOG_ERROR "Unsupported EFI Class");
    }

    return 0;
}