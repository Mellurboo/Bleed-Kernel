#pragma once

#include <stdint.h>
#include <fs/vfs.h>

typedef struct tarheader{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
} tar_header_t;

/// @brief extract a tar files contents to the filesystem
/// @param tar_data .tar file data
/// @param tar_size .tar file size
/// @return success
int tar_extract(const void* tar_data, size_t tar_size);