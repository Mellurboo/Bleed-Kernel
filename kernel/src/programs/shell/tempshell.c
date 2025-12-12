/*
    THIS WILL SOON BE REPLACED WITH AN ACTUAL SHELL, THIS IS JUST A BULLSHIT THING TO HELP TEST
    BEFORE I GET THERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#include <drivers/ps2/ps2_keyboard.h>
#include <stdio.h>
#include <string.h>
#include <fs/fsutils.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <ansii.h>

#define LINE_BUF    256
#define CMND_PFX    "boot$>" 
static char line[LINE_BUF];
static int pos = 0;

static void run_command(const char *cmd) {
    if (strcmp(cmd, "help") == 0) {
        kprintf("%sShitty Shell (tm)%s Commands:\n", RGB_FG(133, 204, 100), RESET);
        kprintf("%shelp:%s display this help message\n", RGB_FG(133, 204, 100), RESET);
        kprintf("%secho:%s print the message following\n", RGB_FG(133, 204, 100), RESET);
        kprintf("%shalt:%s disables interrupts and stops the cpu\n", RGB_FG(133, 204, 100), RESET);
        kprintf("%sls:%s lists the contents of a directory\n", RGB_FG(133, 204, 100), RESET);
        kprintf("%sclear:%s clears the screen\n", RGB_FG(133, 204, 100), RESET);
        kprintf("%scat:%s displays the contents of a file\n", RGB_FG(133, 204, 100), RESET);
        kprintf("%sfault:%s manually trigger a page fault exception\n", RGB_FG(133, 204, 100), RESET);
    }
    else if (strncmp(cmd, "echo ", 5) == 0) {
        kprintf("%s\n", cmd + 5);
    }
    else if (strcmp(cmd, "halt") == 0) {
        kprintf("%sYour Computer has stopped\nFrom here nothing will happen until you hard-reset%s\n",RED_FG, RESET);
        __asm__ volatile("cli");
        for(;;) __asm__ volatile("hlt");
    }
    else if (strncmp(cmd, "ls", 2) == 0) {
        const char *path = "/";
        if (strlen(cmd) > 3) path = cmd + 3;
        list_directory(path);
    }
    else if (strcmp(cmd, "clear") == 0) {
        kprintf("\x1B[2J\x1B[H");
    }
    else if (strncmp(cmd, "cat ", 4) == 0) {
        const char *path_str = cmd + 4;
        path_t path = path_from_abs(path_str);
        INode_t* inode = NULL;
        if (vfs_lookup(&path, &inode) < 0) {
            kprintf("cat: %s: No such file\n", path_str);
        } else {
            if (inode->type == INODE_DIRECTORY){
                kprintf("cat: you cannot cat a directory, to view its contents use 'ls'\n");
                return;
            }
            size_t size = vfs_filesize(inode);
            if (size == 0) {
                kprintf("cat: %s: Empty file or read error\n", path_str);
                inode_drop(inode);
                return;
            }

            char *buffer = kmalloc(size + 1);
            if (!buffer) {
                kprintf("cat: Memory allocation failed\n");
                inode_drop(inode);
                return;
            }

            long r = inode_read(inode, buffer, size, 0);
            if (r > 0) {
                buffer[r] = 0;
                kprintf("%s", buffer);
            }

            kfree(buffer, size);
            inode_drop(inode);
        }
    }
    else if (strncmp(cmd, "fault", 5) == 0){
        *(volatile int*)0 = 1;  //#pf
    }
    else {
        kprintf("Unknown command: %s\n", cmd);
    }
}

static void handle_key(char c) {
    if (c == '\n') {
        line[pos] = 0;
        kprintf("\n");
        run_command(line);
        pos = 0;
        kprintf("%sbleed-kernel%s$ ", RGB_FG(133, 204, 100), RESET);
        return;
    }

    if (c == '\b') {
        if (pos > 0) {
            pos--;
            kprintf("\b \b");
        }
        return;
    }

    if (pos < LINE_BUF - 1) {
        line[pos++] = c;
        kprintf("%c", c);
    }
}

// im so lazy with this extern
void shell_start(void) {
    kprintf(LOG_WARN "This is a very primitive shell that will be removed very soon, thanks for trying out bleed!\n");
    keyboard_set_callback(handle_key);
    kprintf("%sbleed-kernel%s$ ", RGB_FG(133, 204, 100), RESET);
}