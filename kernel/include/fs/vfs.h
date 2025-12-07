#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct INode INode_t;

typedef struct INodeOps{
    int  (*create)  (INode_t* parent, const char* name, size_t namelen, INode_t** result, bool is_directory);
    long (*read)    (INode_t* inode, void* out_buffer, size_t size, size_t offset);
    long (*write)   (INode_t* inode, const void* in_buffer, size_t size, size_t offset);
    int  (*lookup)  (INode_t* dir, const char* name, size_t name_length, INode_t** result);
    void (*drop)    (INode_t* inode);
} INodeOps_t;

enum {
    INODE_DIRECTORY,
    INODE_FILE
};

typedef struct INode {
    size_t  shared;
    int     type;
    const   INodeOps_t* ops;
    void*   internal_data;
} INode_t;

typedef struct filesystem {
    int (*mount)(INode_t** root);
} filesystem;

typedef struct path{
    INode_t *root, *start;
    const char* data;
    size_t data_length;
} path_t;

int tempfs_mount_root(INode_t** root);

int inode_create(INode_t* parent, const char* name, size_t namelen, INode_t** result, bool is_directory);
long inode_read(INode_t* inode, void* buf, size_t count, size_t offset);
long inode_write(INode_t* inode, const void* buf, size_t count, size_t offset);
int inode_lookup(INode_t* dir, const char* name, size_t name_len, INode_t** ret);
void inode_drop(INode_t* inode);

int vfs_lookup(const path_t* path, INode_t** inode);
int vfs_create(const path_t* path, INode_t** result, bool is_directory);
void vfs_drop(INode_t* inode);
path_t path_from_abs(const char* pstring);
path_t parent_path(const path_t* path);

INode_t* vfs_get_root();
int vfs_mount_root();

#endif