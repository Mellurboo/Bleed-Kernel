#include <fs/vfs.h>
#include <stdio.h>
#include <ansii.h>

void list_directory(const char* path_str) {
    path_t path = path_from_abs(path_str);

    INode_t* dir = NULL;
    if (vfs_lookup(&path, &dir) < 0) {
        kprintf(LOG_ERROR "ls: cannot access '%s'\n", path_str);
        return;
    }

    if (dir->type != INODE_DIRECTORY) {
        kprintf(LOG_ERROR "%s is not a directory\n", path_str);
        vfs_drop(dir);
        return;
    }

    kprintf("Directory Listing of %s%s%s\n\t", CYAN_FG, path.data, RESET);
    for (size_t i = 0;; i++) {
        INode_t* child = NULL;
        int r = vfs_readdir(dir, i, &child);
        if (r < 0) break;

        kprintf("%s%s  ", child->type == INODE_DIRECTORY ? CYAN_FG : "", child->internal_data);
        if (child->type == INODE_DIRECTORY) kprintf(CYAN_FG);
    }
    kprintf("\n");
    vfs_drop(dir);
}