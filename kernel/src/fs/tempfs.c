#include <fs/tempfs.h>
#include <mm/pmm.h>
#include <string.h>
#include <stdio.h>
#include <ansii.h>

#define CHUNK_DATA_SIZE (PAGE_SIZE - sizeof(struct chunk))

/// @brief allocate single page chunks for the linked list
static struct chunk* alloc_chunk_page(void){
    paddr_t p = alloc_pages(1);
    if (!p) return NULL;

    void* vpage = paddr_to_vaddr(p);
    if (!vpage) {
        free_pages(p, 1);
        return NULL;
    }

    memset(vpage, 0, PAGE_SIZE);
    struct chunk* chunk = (struct chunk*)vpage;
    chunk->next = NULL;
    return chunk;
}

/// @brief create a file
/// @param parent new files parent
/// @param name new files name
/// @return new file (as an inode)
inode_t* tempfs_create_file(inode_t* parent, const char* name){
    if (parent) {
        if (parent->type != NODE_DIRECTORY || parent->child_count >= MAX_CHILD_OBJS)
            return NULL;
    }

    paddr_t p = alloc_pages(1);
    kprintf("alloc_pages -> p = 0x%p\n", (void*)p);
    if (!p) return NULL;
    inode_t* file = (inode_t*)paddr_to_vaddr(p);
    if (!file) { free_pages(p, 1); return NULL; }
    memset(file, 0, sizeof(inode_t));

    file->type = NODE_FILE;
    file->size = 0;
    file->parent = parent;
    file->file_data = NULL;

    if (parent)
        parent->children[parent->child_count++] = file;

    size_t len = 0;
    while (name[len] != '\0' && len < MAX_NAME_LENGTH - 1) len++;
    memcpy(file->name, name, len);
    file->name[len] = '\0';

    return file;
}

/// @brief create a directory
/// @param parent parent of the new dir
/// @param name name of the new dir
/// @return directory inode
inode_t* tempfs_create_directory(inode_t* parent, const char* name){
    inode_t* dir = tempfs_create_file(parent, name);
    if (!dir) return NULL;

    dir->type = NODE_DIRECTORY;
    dir->child_count = 0;
    return dir;
}

/// @brief helper to get the chunk index and byte (from within the chunk given back) using the byte offset of the whole file
/// @param offset byte offset
/// @param chunk_index chunk index
/// @param chunk_offset where in the chunk the full byte offset points to
static void get_chunk_offset(size_t offset, size_t* chunk_index, size_t* chunk_offset){
    *chunk_index = offset / CHUNK_DATA_SIZE;
    *chunk_offset = offset % CHUNK_DATA_SIZE;
}

/// @brief gets the chunk in a file with an index, but if it doesnt exsist itll make it for us
static struct chunk* get_chunk(inode_t* file, size_t index){
    if (!file->file_data){
        file->file_data = (struct chunk*)alloc_chunk_page();
        if (!file->file_data) return NULL;
    }

    struct chunk* current_chunk = (struct chunk*)file->file_data;
    for (size_t i = 0; i < index; i++){
        if (!current_chunk->next){
            current_chunk->next = alloc_chunk_page();
            if (!current_chunk->next) return NULL;
        }
        current_chunk = current_chunk->next;
    }
    return current_chunk;
}

/// @brief write to a file
/// @param file target file
/// @param buffer in buffer
/// @param size size of write
/// @param offset start write offset in bytes
/// @return size that we wrote
long tempfs_write(inode_t* file, const void* buffer, size_t size, size_t offset){
    if (!file || file->type != NODE_FILE) return 0;
    if (size == 0) return 0;

    size_t written = 0;
    size_t chunk_idx, chunk_offset;
    get_chunk_offset(offset, &chunk_idx, &chunk_offset);

    struct chunk* current_chunk = get_chunk(file, chunk_idx);
    if (!current_chunk) return 0;

    const uint8_t* src = (const uint8_t*)buffer;
    while (written < size){
        uint8_t* data_ptr = (uint8_t*)current_chunk + sizeof(struct chunk);
        size_t space = CHUNK_DATA_SIZE - chunk_offset;
        size_t to_write = (size - written < space) ? (size - written) : space;

        memcpy(data_ptr + chunk_offset, src + written, to_write);
        written += to_write;
        chunk_offset = 0;

        if (written < size){
            if (!current_chunk->next){
                current_chunk->next = alloc_chunk_page();
                if (!current_chunk->next) break;
            }
            current_chunk = current_chunk->next;
        }
    }

    if (offset + written > file->size)
        file->size = offset + written;

    return written;
}

/// @brief read a file out to a buffer
/// @param file target file
/// @param out_buffer data out buffer
/// @param size size to read
/// @param offset start read offset (bytes)
/// @return size that was read
long tempfs_read(inode_t* file, void* out_buffer, size_t size, size_t offset){
    if (!file || file->type != NODE_FILE || !file->file_data) return 0;
    if (offset >= file->size) return 0;

    if (offset + size > file->size)
        size = file->size - offset;

    size_t read_total = 0;
    size_t chunk_idx, chunk_offset;
    get_chunk_offset(offset, &chunk_idx, &chunk_offset);

    struct chunk* current_chunk = (struct chunk*)file->file_data;
    for (size_t i = 0; i < chunk_idx; i++){
        if (!current_chunk) return 0;
        current_chunk = current_chunk->next;
    }
    if (!current_chunk) return 0;

    uint8_t* dst = (uint8_t*)out_buffer;
    while (read_total < size && current_chunk){
        uint8_t* data_ptr = (uint8_t*)current_chunk + sizeof(struct chunk);
        size_t space = CHUNK_DATA_SIZE - chunk_offset;
        size_t to_read = (size - read_total < space) ? (size - read_total) : space;

        memcpy(dst + read_total, data_ptr + chunk_offset, to_read);
        read_total += to_read;
        chunk_offset = 0;
        current_chunk = current_chunk->next;
    }

    return read_total;
}

inode_t* tempfs_get_child_by_name(inode_t* parent, const char* cname){
    if (!parent || parent->type != NODE_DIRECTORY) return NULL;
    for (size_t i = 0; i < parent->child_count; i++){
        inode_t* child = parent->children[i];
        if (strcmp(child->name, cname) == 0) return child;
    }

    return NULL;
}

inode_t* tempfs_create_directory_recursive(inode_t* root, const char* path, bool strip_last){
    char buffer[256];
    size_t length = 0;

    while(length < sizeof(buffer) - 1 && path[length]) {
        buffer[length] = path[length];
        length++;
    }

    buffer[length] = '\0';

    inode_t* current = root;
    char *pbuf = buffer;

    while(1){
        while (*pbuf == '/') pbuf++;
        if (*pbuf == '\0') return current;

        // next slash
        char *slash = pbuf;
        while(*slash && *slash != '/') slash++;

        bool is_last_slash = (*slash == '\0');
        if (is_last_slash && strip_last) return current;

        // isolate
        char save = *slash;
        *slash = '\0';

        // find or create
        inode_t* child = tempfs_get_child_by_name(current, pbuf);
        if (!child){
            child = tempfs_create_directory(current, pbuf);
            if (!child) return NULL; //shit
        }

        if (save == '\0') return child;

        *slash = save;
        pbuf = slash + 1;
        current = child;
    }
}

inode_t* tempfs_find_file(inode_t* root, const char* path) {
    if (!root || !path || path[0] == '\0') return root;
    inode_t* current = root;

    while (*path == '/') path++;

    char buffer[256];
    size_t i = 0;

    while (true) {
        i = 0;
        while (*path && *path != '/' && i < sizeof(buffer) - 1) {
            buffer[i++] = *path++;
        }
        buffer[i] = '\0';

        if (i == 0) break; // empty
        bool found = false;
        if (current->type != NODE_DIRECTORY) return NULL;

        for (int j = 0; j < current->child_count; j++) {
            inode_t* child = current->children[j];
            if (strcmp(child->name, buffer) == 0) {
                current = child;
                found = true;
                break;
            }
        }

        if (!found) return NULL;

        // Skip slashes
        while (*path == '/') path++;
        if (*path == '\0') break;
    }

    return current;
}

inode_t** tempfs_get_directory_nodes(inode_t* dir, size_t* out_count, paddr_t* out_paddr) {
    if (!dir || dir->type != NODE_DIRECTORY) {
        if (out_count) *out_count = 0;
        if (out_paddr) *out_paddr = 0;
        return NULL;
    }

    size_t count = dir->child_count;
    if (out_count) *out_count = count;
    if (count == 0) {
        if (out_paddr) *out_paddr = 0;
        return NULL;
    }

    size_t byte_size = count * sizeof(inode_t*);
    size_t pages = (byte_size + PAGE_SIZE - 1) / PAGE_SIZE;

    paddr_t p = alloc_pages(pages);
    if (!p) {
        if (out_paddr) *out_paddr = 0;
        return NULL;
    }

    inode_t** nodes = (inode_t**)paddr_to_vaddr(p);
    for (size_t i = 0; i < count; i++) {
        nodes[i] = dir->children[i];
    }

    if (out_paddr) *out_paddr = p;
    return nodes;
}

/// @brief basically just an ls helper to print the things in a directory
/// @param dir target dir
void tempfs_list(inode_t* dir) {
    if (!dir || dir->type != NODE_DIRECTORY) {
        kprintf("Not a directory\n");
        return;
    }

    kprintf("Directory Listing of %s%s%s:\n", BLUE_FG, dir->name, RESET);
    kprintf("\t");

    size_t count = 0;
    paddr_t nodes_paddr = 0;
    inode_t** nodes = tempfs_get_directory_nodes(dir, &count, &nodes_paddr);
    if (!nodes ||(!nodes && count == 0)) {
        kprintf("\n");
        return;
    }

    for (size_t i = 0; i < count; i++) {
        if (nodes[i]->type == NODE_DIRECTORY) {
            kprintf("%s%s/%s\t", BLUE_FG, nodes[i]->name, RESET);
        } else {
            kprintf("%s%s\t%s", GRAY_FG, nodes[i]->name, FG_RESET);
        }
    }

    kprintf("\n");

    if (nodes_paddr) {
        size_t byte_size = count * sizeof(inode_t*);
        size_t pages = (byte_size + PAGE_SIZE - 1) / PAGE_SIZE;
        free_pages(nodes_paddr, pages);
    }
}

static void tempfs_free_chunks(struct chunk* c) {
    while (c) {
        struct chunk* next = c->next;
        paddr_t p = vaddr_to_paddr((void*)c);
        memset(c, 0, PAGE_SIZE);
        free_pages(p, 1);
        c = next;
    }
}

static int tempfs_remove_from_parent(inode_t* parent, inode_t* node) {
    size_t idx = (size_t)-1;

    for (size_t i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == node) {
            idx = i;
            break;
        }
    }
    if (idx == (size_t)-1)
        return -1;

    for (size_t i = idx; i + 1 < parent->child_count; i++) {
        parent->children[i] = parent->children[i + 1];
    }
    parent->children[parent->child_count - 1] = NULL;

    parent->child_count--;
    return 0;
}

int tempfs_delete_node(inode_t* parent, const char* name) {
    if (!parent || parent->type != NODE_DIRECTORY)
        return -1;

    inode_t* target = tempfs_get_child_by_name(parent, name);
    if (!target)
        return -1;

    // fail if directory not empty
    if (target->type == NODE_DIRECTORY && target->child_count > 0)
        return -2;

    if (tempfs_remove_from_parent(parent, target) != 0)
        return -1;

    if (target->type == NODE_FILE && target->file_data)
        tempfs_free_chunks((struct chunk*)target->file_data);

    if (target->type == NODE_DIRECTORY && target->children) {
        memset(target->children, 0, target->child_count);
    }

    paddr_t p = vaddr_to_paddr((void*)target);
    memset(target, 0, sizeof(inode_t));
    free_pages(p, 1);

    return 0;
}

int tempfs_delete_node_recursive(inode_t* parent, const char* name) {
    if (!parent || parent->type != NODE_DIRECTORY)
        return -1;

    inode_t* target = tempfs_get_child_by_name(parent, name);
    if (!target)
        return -1;
 
    if (tempfs_remove_from_parent(parent, target) != 0)
        return -1;

    if (target->type == NODE_DIRECTORY) {
        while (target->child_count > 0) {
            inode_t* child = target->children[0];

            // Recursively delete by name
            tempfs_delete_node_recursive(target, child->name);
        }

        if (target->children)
            memset(target->children, 0, target->child_count);
    }

    //free chunk chain, wlak the linked list
    if (target->type == NODE_FILE && target->file_data)
        tempfs_free_chunks((struct chunk*)target->file_data);

    // Free inode itself
    paddr_t p = vaddr_to_paddr((void*)target);
    memset(target, 0, sizeof(inode_t));
    free_pages(p, 1);

    return 0;
}