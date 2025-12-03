#ifndef TEMPFS_H
#define TEMPFS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_NAME_LENGTH 256
#define MAX_CHILD_OBJS  64

typedef enum{
    NODE_DIRECTORY,
    NODE_FILE
} inode_type_t;

typedef struct inode {
    inode_type_t type;
    char    name[MAX_NAME_LENGTH];
    size_t  size;
    void*   file_data;

    struct inode* parent;
    struct inode* children[MAX_CHILD_OBJS];

    size_t child_count;
} inode_t;


inode_t* tempfs_create_file(inode_t* parent, const char* name);
inode_t* tempfs_create_directory(inode_t* parent, const char* name);
size_t tempfs_write(inode_t* file, const void* buffer, size_t size, size_t offset);
size_t tempfs_read(inode_t* file, void* out_buffer, size_t size, size_t offset);

#endif