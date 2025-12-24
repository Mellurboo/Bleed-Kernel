#pragma once
#include <fs/vfs.h>

int elf_load(INode_t *elf_file, paddr_t cr3, uintptr_t* entry);