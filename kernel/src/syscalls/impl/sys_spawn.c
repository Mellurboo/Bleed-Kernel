#include <stdint.h>
#include <mm/heap.h>
#include <sched/scheduler.h>
#include <fs/vfs.h>
#include <user/user_copy.h>
#include <exec/elf_load.h>
#include <gdt/gdt.h>
#include <mm/paging.h>

uint64_t sys_spawn(uint64_t user_path_ptr) {
    char *kpath = user_copy_string((const char *)user_path_ptr, 256);
    if (!kpath) return -1;

    INode_t *file = NULL;
    path_t filepath = vfs_path_from_abs(kpath);

    vfs_lookup(&filepath, &file);
    paddr_t cr3 = paging_create_address_space();

    uintptr_t entry;
    if (file != NULL) elf_load(file, cr3, &entry);
    return sched_create_task(cr3, entry, USER_CS, USER_SS)->id;
}
