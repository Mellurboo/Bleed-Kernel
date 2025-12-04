#include <fs/archive/tar.h>
#include <fs/tempfs.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <ansii.h>

static inline uint64_t tar_octal_to_uint(const char* str, size_t n){
    uint64_t value = 0;

    for (size_t i = 0; i < n && str[i] != '\0' && str[i] != ' '; i++){
        char c = str[i];
        if (c < '0' || c > '7') break;
        value = (value << 3) + (c - '0');
    }

    return value;
}

static inline bool is_zero_block(const void* block){
    const uint8_t* b = block;
    for (size_t i = 0; i < 512; i++){
        if (b[i] != 0) return false;
    }
    return true;
}

static void tar_build_path(char *out, size_t out_size, const tar_header_t* header){
    if (header->prefix[0]){
        size_t prefix_length = 0;
        while(prefix_length < sizeof(header->prefix) && header->prefix[prefix_length]) prefix_length++;
        size_t name_length = 0;
        while(name_length < sizeof(header->name) && header->name[name_length]) name_length++;

        size_t total = prefix_length + 1 + name_length;
        if (total >= out_size){
            size_t copy_prefix = (out_size - 1) / 2;
            size_t copy_name = out_size - 1 - copy_prefix - 1;
            
            memcpy(out, header->prefix, copy_prefix);
            out[copy_prefix] = '/';
            memcpy(out + copy_prefix + 1, header->name, copy_name);
            out[copy_prefix + 1 + copy_name] = '\0';
            return;
        }

        memcpy(out, header->prefix, prefix_length);
        out[prefix_length] = '/';
        memcpy(out + prefix_length + 1, header->name, name_length);
        out[total] = '\0';
    }else{
        size_t name_length = 0;
        while (name_length < sizeof(header->name) && header->name[name_length]) name_length++;
        if (name_length >= out_size) name_length = out_size - 1;
        memcpy(out, header->name, name_length);
        out[name_length] = '\0';
    }
}

static char* tar_find_last_slash(char *string){
    char* last = NULL;
    for (char* p = string; *p; p++) if (*p == '/') last = p;
    return last;
}

/// @brief extract a tar archive to a directory
/// @param archive_address address to limine module
/// @param archive_size size of module
/// @param destination_dir mount destination
/// @return success
bool extract_tar(void* archive_address, size_t archive_size, inode_t* destination_dir){
    uint8_t* ptr = (uint8_t*)archive_address;
    uint8_t* end = ptr + archive_size;
    
    while (ptr + 512 <= end){
        tar_header_t* header = (tar_header_t*)ptr;

        if (is_zero_block(header)) break;

        char fullpath[256];
        tar_build_path(fullpath, sizeof(fullpath), header);

        if (fullpath[0] == '\0') break;

        char pathnorm[256];
        size_t pn = 0;
        const char *s = fullpath;
        while ((*s == '/') || (*s == '.' && s[1] == '/')) {
            if (*s == '/') s++;
            else if (*s == '.' && s[1] == '/') s += 2;
            else break;
        }
        while (*s && pn + 1 < sizeof(pathnorm)) {
            if (s[0] == '.' && s[1] == '.' && (s[2] == '/' || s[2] == '\0')) {
                pathnorm[0] = '\0';
                break;
            }
            pathnorm[pn++] = *s++;
        }
        pathnorm[pn] = '\0';
        if (pathnorm[0] == '\0'){ ptr += 512; continue; }

        uint64_t filesize = tar_octal_to_uint(header->size, sizeof(header->size));

        char type = header->typeflag;
        if (type == '\0') type = '0';

        if (type == '5'){
            tempfs_create_directory_recursive(destination_dir, pathnorm, 0);
        }else if (type == '0' || type == '7'){
            char temp[256];
            strncpy(temp, pathnorm, sizeof(temp));
            temp[sizeof(temp) - 1] = '\0';

            char *last = tar_find_last_slash(temp);
            inode_t* parent = destination_dir;
            const char* file_name = temp;

            if (last){
                *last = '\0';
                parent = tempfs_create_directory_recursive(destination_dir, temp, 0);
                file_name = last + 1;
                if (!parent){
                    kprintf(LOG_ERROR "Tar failed to make parent for archive destination\n");
                    return false;
                }
            }

            inode_t* file = tempfs_create_file(parent, file_name);
            if (!file){
                kprintf(LOG_ERROR "failed to make file\n");
                return false;
            }

            uint8_t *data = ptr + 512;
            if (data + filesize > end){
                kprintf(LOG_WARN "Data Was Truncated for: %s", file_name);
                return false;
            }

            tempfs_write(file, data, filesize, 0);
        }else{
            kprintf(LOG_WARN "skipping type %c for %s\n", type, pathnorm);
        }

        uint64_t data_blocks = (filesize + 511) / 512;
        uint64_t advance = 512 + data_blocks * 512;
        ptr += advance;
    }

    return true;
}
