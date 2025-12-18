#ifndef ELF_LOAD_H
#define ELF_LOAD_H

#include <stdint.h>

#define ELF_NIDENT  16

#define ELFMAG0 0x7F
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFDATA2LSB (1)     // Little Endian
#define ELFCLASS32  (1)     // 32 bit

typedef uint16_t ELF32_HALF;
typedef uint32_t ELF32_OFF;
typedef uint32_t ELF32_ADDR;
typedef uint32_t ELF32_WORD;
typedef int32_t  ELF32_SWORD;

enum ELF_TYPE {
    ET_NONE     = 0,
    ET_RELO     = 1,    // relocatable
    ET_EXEC     = 2,
};

enum ELF_IDENTITY {
    EI_MAG0     = 0,
    EI_MAG1     = 1,
    EI_MAG2     = 2,
    EI_MAG3     = 3,
    EI_CLASS    = 4,
    EI_DATA     = 5,
    EI_VERSION  = 6,
    EI_OSABI    = 7,
    EI_ABIVER   = 8,
    EI_PAD      = 9,
};

typedef struct {
    uint8_t     e_ident[ELF_NIDENT];
    ELF32_HALF  e_type;
    ELF32_HALF  e_machine;
    ELF32_WORD  e_version;
    ELF32_ADDR  e_entry;
    ELF32_OFF   e_phoff;
    ELF32_OFF   e_shoff;
    ELF32_WORD  e_flags;
    ELF32_HALF  e_ehsize;
    ELF32_HALF  e_phentsize;
    ELF32_HALF  e_phnum;
    ELF32_HALF  e_shentsize;
    ELF32_HALF  e_shnum;
    ELF32_HALF  e_shstrndx;
} ELF32_EHDR;

#endif