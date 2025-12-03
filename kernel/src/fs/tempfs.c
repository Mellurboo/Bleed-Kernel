#include <fs/tempfs.h>
#include <mm/pmm.h>
#include <string.h>
#include <stdio.h>
#include <ansii.h>

void* alloc_file(size_t size){
    if (size == 0) size = 1;
    size_t page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    paddr_t paddr = alloc_pages(page_count);
    if (!paddr){
        kprintf(LOG_ERROR "Failure to allocate file of size %llu bytes\n", size);
        return NULL;
    }
    return paddr_to_vaddr(paddr);
}

inode_t* tempfs_create_file(inode_t* parent, const char* name){
    // only do the child limit if theres a parent becuase if not, this breaks the root directory
    if (parent) {
        if (parent->type != NODE_DIRECTORY || parent->child_count >= MAX_CHILD_OBJS)
            return NULL;
    }

    inode_t* file = (inode_t*)alloc_file(sizeof(inode_t));
    if (!file) return NULL;

    file->type = NODE_FILE;
    file->size = 0;
    file->parent = parent;
    file->file_data = NULL;

    if (parent)
        parent->children[parent->child_count++] = file;

    // imagine we had strncpy, ill get round to it
    size_t len = 0;
    while (name[len] != '\0' && len < MAX_NAME_LENGTH - 1) len++;
    memcpy(file->name, name, len);
    file->name[len] = '\0';

    return file;
}

inode_t* tempfs_create_directory(inode_t* parent, const char* name){
    inode_t* dir = tempfs_create_file(parent, name);
    if (!dir) return NULL;

    dir->type = NODE_DIRECTORY;
    dir->child_count = 0;
    return dir;
}

size_t tempfs_write(inode_t* file, const void* buffer, size_t size, size_t offset){
    if (!file || file->type != NODE_FILE) return 0;

    if (!file->file_data){
        file->file_data = alloc_file(size + offset);
        if (!file->file_data) return 0;
    }

    memcpy((uint8_t*)file->file_data + offset, buffer, size);

    if (offset + size > file->size)
        file->size = offset + size;

    return size;
}

size_t tempfs_read(inode_t* file, void* out_buffer, size_t size, size_t offset){
    if (!file || file->type != NODE_FILE || !file->file_data) return 0;
    if (offset >= file->size) return 0;

    if (offset + size > file->size)
        size = file->size - offset;

    memcpy(out_buffer, (uint8_t*)file->file_data + offset, size);
    return size;
}
