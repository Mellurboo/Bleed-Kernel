#ifndef TEMPFS_H
#define TEMPFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_NAME_LENGTH 256
#define MAX_CHILD_OBJS  64

typedef enum{
    NODE_DIRECTORY,
    NODE_FILE
} inode_type_t;

struct chunk {
    struct chunk* next;
};

typedef struct inode {
    inode_type_t type;
    char    name[MAX_NAME_LENGTH];
    size_t  size;
    struct chunk* file_data;

    struct inode* parent;
    struct inode* children[MAX_CHILD_OBJS];

    size_t child_count;
} inode_t;

inode_t* tempfs_create_file(inode_t* parent, const char* name);
inode_t* tempfs_create_directory(inode_t* parent, const char* name);
inode_t* tempfs_create_directory_recursive(inode_t* root, const char* path, bool strip_last);
long tempfs_write(inode_t* file, const void* buffer, size_t size, size_t offset);
long tempfs_read(inode_t* file, void* out_buffer, size_t size, size_t offset);
inode_t* tempfs_find_file(inode_t* root, const char* path);
void tempfs_list(inode_t* dir);

inode_t* init_initrd(inode_t* root);
#endif
