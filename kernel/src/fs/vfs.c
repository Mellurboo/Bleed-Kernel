#include <fs/vfs.h>
#include <string.h>
#include <ansii.h>
#include <stdio.h>
#include <status.h>
#include <stdbool.h>

extern const filesystem tempfs;

INode_t* vfs_root = NULL;

int vfs_mount_root(){
    int r = tempfs.mount(&vfs_root);
    if (r < 0) {
        //kprintf(LOG_ERROR "vfs_mount_root: tempfs.mount failed: %d\n", r);
    }
    //kprintf(LOG_OK "VFS Root Mounted\n");
    return r;
}

INode_t* vfs_get_root(){
    return vfs_root;
}

void vfs_drop(INode_t* inode){
    if (!inode) return;
    if (inode == vfs_get_root()) return;
    if (inode->shared == 0) return;
    inode->shared--;
}

int vfs_lookup(const path_t* path, INode_t** inode){
    INode_t* current_inode = path->start;
    const char* head = path->data, *head_end = path->data + path->data_length;
    while(head < head_end) {
        while (head < head_end && *head == '/') head++;
        if (head >= head_end) break;

        const char* comp_start = head;
        while (head < head_end && *head != '/') head++;
        size_t comp_len = head - comp_start;

        INode_t* next = NULL;
        long r = inode_lookup(current_inode, comp_start, comp_len, &next);
        if (r < 0) return -FILE_NOT_FOUND;

        current_inode = next;
    }
    if (current_inode) current_inode->shared++;
    *inode = current_inode;
    return 0;
}

int vfs_create(const path_t* path, INode_t** result, bool is_directory){
    INode_t* parent_inode = NULL;
    path_t parent = parent_path(path);

    int e = vfs_lookup(&parent, &parent_inode);
    if (e < 0) return e; // file doesnt exsist (probably)

    const char* name = parent.data + parent.data_length;
    size_t namelen = path->data_length - parent.data_length;

    e = inode_create(parent_inode, name, namelen, result, is_directory);
    vfs_drop(parent_inode);
    return e;
}

path_t parent_path(const path_t* path){
    const char* end = path->data + path->data_length;
    while(end > path->data && *(end-1) == '/') end--;
    while(end > path->data && *(end-1) != '/') end--;
    return(path_t){
        .root = path->root,
        .start = path->start,
        .data = path->data,
        .data_length = end - path->data,
    };
}

path_t path_from_abs(const char* path){
    return (path_t){
        .root = vfs_root,
        .start = vfs_root,
        .data = path,
        .data_length = strlen(path),
    };
}

/// @brief read the files into a buffer and count the size
/// @param inode target
/// @return unsigned size
size_t vfs_filesize(INode_t* inode) {
    if (!inode || !inode->ops->read) return 0;

    size_t total = 0;
    size_t offset = 0;
    char buffer[4096];
    long r;

    while ((r = inode_read(inode, buffer, sizeof(buffer), offset)) > 0) {
        total += r;
        offset += r;
    }

    return total;
}

int inode_create(INode_t* parent, const char* name, size_t namelen, INode_t** result, bool is_directory){
    if (parent->ops->create == NULL) return -UNIMPLEMENTED;
    return parent->ops->create(parent, name, namelen, result, is_directory);
}

int inode_lookup(INode_t* dir, const char* name, size_t name_len, INode_t** result){
    if (dir->ops->lookup == NULL) return -UNIMPLEMENTED; // Unsupported Operation
    return dir->ops->lookup(dir, name, name_len, result);
}

void inode_drop(INode_t* inode){
    if (inode->ops->drop == NULL) return;
    inode->ops->drop(inode);
}

long inode_write(INode_t* inode, const void* in_buffer, size_t count, size_t offset){
    if (inode->ops->write == NULL) return -UNIMPLEMENTED;
    return inode->ops->write(inode, in_buffer, count, offset);
}

long inode_read(INode_t* inode, void* out_buffer, size_t count, size_t offset){
    if (inode->ops->read == NULL) return -UNIMPLEMENTED;
    return inode->ops->read(inode, out_buffer, count, offset);
}

int vfs_readdir (INode_t* dir, size_t index, INode_t** result){
    if(!dir || !dir->ops->readdir) return -UNIMPLEMENTED;
    return dir->ops->readdir(dir, index, result);
}