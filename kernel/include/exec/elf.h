#pragma once
#define EI_NIDENT   16

#include <stdint.h>

typedef uint64_t ELF64_ADDR;
typedef uint64_t ELF64_OFFS;
typedef uint16_t ELF64_HALF;
typedef uint32_t ELF64_WORD;
typedef uint64_t ELF64_LONG;
typedef int64_t  ELF64_SLONG;

#define PT_NULL     0
#define PT_LOAD     1
#define PT_INTERP   3

#define ET_EXEC     2

#define PF_N        0x0
#define PF_X        0x1
#define PF_W        0x2
#define PF_R        0x4

#define EI_CLASS64  2

#define EI_LITTLE_ENDIAN    1
#define EI_BIG_ENDIAN       2

typedef struct {
    unsigned char   e_ident[EI_NIDENT];
    ELF64_HALF      e_type;
    ELF64_HALF      e_machine;
    ELF64_WORD      e_version;
    ELF64_ADDR      e_entry;
    ELF64_OFFS      e_phoff;
    ELF64_OFFS      e_shoff;
    ELF64_WORD      e_flags;
    ELF64_HALF      e_ehsize;
    ELF64_HALF      e_phentsize;
    ELF64_HALF      e_phnum;
    ELF64_HALF      e_shentsize;
    ELF64_HALF      e_shnum;
    ELF64_HALF      e_shstrndx;
} ELF64_EHDR;

typedef struct {
	ELF64_WORD	    p_type;
	ELF64_WORD	    p_flags;
	ELF64_OFFS	    p_offset;
	ELF64_ADDR	    p_vaddr;
	ELF64_ADDR	    p_paddr;
	ELF64_LONG	    p_filesz;
	ELF64_SLONG	    p_memsz;
	ELF64_SLONG	    p_align;
} ELF64_Phdr;