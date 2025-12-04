#ifndef TAR_H
#define TAR_H

#include <fs/tempfs.h>
#include <stdbool.h>

#pragma pack(push, 1)
typedef struct tar_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
} tar_header_t;
#pragma pack(pop)

bool extract_tar(void* archive_address, size_t archive_size, inode_t* destination_dir);

#endif