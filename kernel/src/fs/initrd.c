#include <lib/limine/limine.h>
#include <fs/tempfs.h>
#include <stdio.h>
#include <ansii.h>
#include <panic.h>
#include <fs/archive/tar.h>
#include <fs/tempfs.h>

extern volatile struct limine_module_request module_request;

inode_t* init_initrd(inode_t* root){
    if (!module_request.response) kpanic("No Modules Loaded by Limine");

    struct limine_file* mod = module_request.response->modules[0];
    if (!mod) kpanic("initrd not found");

    kprintf(LOG_INFO "Loading initrd: %s (size %llu)\n", mod->path, mod->size);
    if(extract_tar(mod->address, mod->size, root)){
        kprintf(LOG_OK "Tar File Extracted\n\t- %s\n\t- size %llu\n", mod->path, mod->size);
    }else{
        kprintf(LOG_WARN "Failure Extracting Tar File %s", mod->path);
    }

    inode_t* initrd = tempfs_find_file(root, "initrd");
    inode_t* namefile = tempfs_find_file(root, "initrd/initrd_test.txt");

    char buffer[64];
    tempfs_read(namefile, buffer, sizeof(buffer) - 1, 0);
    buffer[sizeof(buffer)] = '\0';
    kprintf(LOG_INFO "%s /%s/\n", buffer, initrd->name);

    return initrd;
}
