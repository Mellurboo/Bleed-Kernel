#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
    functions prefixed inode_ are operations where 
    as functions prefixed VFS are part of the filesystem
    itself
*/

typedef struct INode INode_t;

typedef enum {
    INODE_DIRECTORY,
    INODE_FILE
} inode_type;

typedef struct INodeOps{
    int    (*create)  (INode_t* parent, const char* name, size_t namelen, INode_t** result, inode_type node_type);
    long   (*read)    (INode_t* inode, void* out_buffer, size_t size, size_t offset);
    long   (*write)   (INode_t* inode, const void* in_buffer, size_t size, size_t offset);
    int    (*lookup)  (INode_t* dir, const char* name, size_t name_length, INode_t** result);
    void   (*drop)    (INode_t* inode);
    int    (*readdir) (INode_t* dir, size_t index, INode_t** result);
    size_t (*size)    (INode_t* inode);
} INodeOps_t;

typedef struct INode {
    long  shared;
    int     type;
    const   INodeOps_t* ops;
    void*   internal_data;
} INode_t;

typedef struct filesystem {
    int (*mount)(INode_t** root);
} filesystem;

typedef struct path{
    INode_t    *root, *start;
    const char *data;
    size_t      data_length;
} path_t;

/// @brief mount the root directory of the fs
/// @param root root node
/// @return success?
int tempfs_mount_root(INode_t** root);

/// @brief decrement shared, min 0, cannot drop VFS root
/// @param inode target
INode_t* vfs_get_root();

/// @brief VFS Mount the root directory
/// @return success?
int vfs_mount_root();

/// @brief decrement shared, min 0, cannot drop VFS root
/// @param inode target
void vfs_drop(INode_t* inode);

/// @brief get an inode at a path
/// @param path target path
/// @param inode OUT inode
/// @return success?
int vfs_lookup(const path_t* path, INode_t** inode);

long vfs_read_exact(INode_t *inode, void *out_buffer, size_t exact_count, size_t offset);


int vfs_create(const path_t* path, INode_t** result, inode_type node_type);
int vfs_readdir (INode_t* dir, size_t index, INode_t** result);
size_t vfs_filesize(INode_t* inode);
path_t vfs_path_from_abs(const char* pstring);
path_t vfs_parent_path(const path_t* path);

int inode_create(INode_t* parent, const char* name, size_t namelen, INode_t** result, inode_type node_type);
long inode_read(INode_t* inode, void* buf, size_t count, size_t offset);
long inode_write(INode_t* inode, const void* buf, size_t count, size_t offset);
int inode_lookup(INode_t* dir, const char* name, size_t name_len, INode_t** ret);
void inode_drop(INode_t* inode);
