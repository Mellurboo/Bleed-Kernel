#include <panic.h>
#include <drivers/ps2/PS2_keyboard.h>
#include <drivers/framebuffer/framebuffer.h>
#include <sched/scheduler.h>
#include <stdio.h>
#include <string.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <mm/pmm.h>
#include <ansii.h>
#include <threads/exit.h>
#include <drivers/serial/serial.h>
#include <idt/idt.h>

#define LINE_BUF        256
#define CURSOR_COLOR    0xBBBBBB;
static char line[LINE_BUF];
static int pos = 0;

extern size_t physical_memory_total;
extern paddr_t highest_free_paddr;

static void print_directory_list(const char* path_str);
static void run_command(const char* cmd);

void neofetch(void) {
    INode_t* splash_inode = NULL;
    path_t splash_path = vfs_path_from_abs("initrd/etc/splash.txt");

    char* logo_lines[32];
    size_t logo_count = 0;
    size_t logo_width = 0;

    if (vfs_lookup(&splash_path, &splash_inode) == 0) {
        size_t splash_size = vfs_filesize(splash_inode);
        char* splash_buffer = kmalloc(splash_size + 1);
        inode_read(splash_inode, splash_buffer, splash_size, 0);
        splash_buffer[splash_size] = 0;

        char* ptr = splash_buffer;
        while (*ptr && logo_count < 32) {
            char* line_end = strchr(ptr, '\n');
            if (!line_end) line_end = ptr + strlen(ptr);

            size_t len = line_end - ptr;
            logo_lines[logo_count] = kmalloc(len + 1);
            memcpy(logo_lines[logo_count], ptr, len);
            logo_lines[logo_count][len] = 0;

            if (len > logo_width) logo_width = len;
            logo_count++;

            ptr = (*line_end) ? line_end + 1 : line_end;
        }

        kfree(splash_buffer, splash_size);
        inode_drop(splash_inode);
    }

    size_t phys_mem = paging_get_usable_mem_size() / 1024 / 1024;
    uint64_t task_count = get_task_count();

    // Print logo and info side by side
    size_t max_lines = (logo_count > 5) ? logo_count : 5; // 5 info lines
    for (size_t i = 0; i < max_lines; i++) {
        if (i < logo_count) {
            kprintf("%s%s%s", RED_FG, logo_lines[i], RESET);
            for (size_t j = strlen(logo_lines[i]); j < logo_width; j++) kprintf(" ");
            kfree(logo_lines[i], strlen(logo_lines[i]));
        } else {
            for (size_t j = 0; j < logo_width; j++) kprintf(" ");
        }
        kprintf("  ");

        // Info line
        switch(i) {
            case 0:
            case 1:
            case 2:
            case 3: kprintf(" "); break;
            case 4: kprintf("%sKernel:%s Bleed Kernel 2025", GRAY_FG, RESET); break;
            case 5: kprintf("%sAuthor:%s Myles \"Mellurboo\" Wilson", GRAY_FG, RESET); break;
            case 6: kprintf("%sLicense:%s GPLv3", GRAY_FG, RESET); break;
            case 7: kprintf("%sPhysical Memory:%s %zuMiB", GRAY_FG, RESET, phys_mem); break;
            case 8: kprintf("%sScheduler Tasks:%s %llu", GRAY_FG, RESET, task_count); break;
        }

        kprintf("\n");
    }

    kprintf("\n");
}

static tty_cursor_t prev_cursor = {0, 0};
static void draw_cursor() {
    psf_font_t *font = psf_get_current_font();
    tty_cursor_t c = cursor_get_position();
    if (!font) return;

    uint32_t *fb_ptr = (uint32_t *)framebuffer_get_addr(0);
    size_t pitch = framebuffer_get_pitch(0);

    size_t px_base = prev_cursor.x * font->width;
    size_t py_base = prev_cursor.y * font->height + font->height - 1;
    for (size_t x = 0; x < font->width; x++)
        fb_ptr[py_base * pitch + (px_base + x)] = 0x00000000;

    px_base = c.x * font->width;
    py_base = c.y * font->height + font->height - 1;
    for (size_t x = 0; x < font->width; x++)
        fb_ptr[py_base * pitch + (px_base + x)] = CURSOR_COLOR;

    prev_cursor = c;
}

static void handle_key(char c) {
    if (c == '\n') {
        line[pos] = 0;
        kprintf("\n");
        run_command(line);
        pos = 0;
        kprintf("%skernel@%sbleed-kernel$ %s", RED_FG, GRAY_FG, RESET);
        draw_cursor();
        return;
    }
    if (c == '\b' && pos > 0) {
        pos--;
        kprintf("\b \b");
        draw_cursor();
        return;
    }
    if (pos < LINE_BUF - 1) {
        line[pos++] = c;
        kprintf("%c", c);
    }
    draw_cursor();
}

static void sched_print_task(task_t *task, void *userdata) {
    (void)userdata;
    if (task->state == TASK_READY ||
        task->state == TASK_RUNNING ||
        task->state == TASK_DEAD) {

        kprintf("%s%llu%s\t%s\t%u\t%p\n",
            CYAN_FG, task->id, RESET,
            task_state_str(task->state),
            task->quantum_remaining,
            (void*)task->page_map
        );
    }
}

void task_exit_test(void) {
    kprintf("[Test Task] Starting and will exit immediately\n");
    exit();
    kprintf("[Test Task] ERROR: Task did not exit!\n");
    for (;;) __asm__("hlt");
}

static void print_directory_list(const char* path_str) {
    path_t path = vfs_path_from_abs(path_str);

    INode_t* dir = NULL;
    if (vfs_lookup(&path, &dir) < 0) {
        kprintf(LOG_ERROR "ls: cannot access '%s'\n", path_str);
        return;
    }

    if (dir->type != INODE_DIRECTORY) {
        kprintf(LOG_ERROR "%s is not a directory\n", path_str);
        vfs_drop(dir);
        return;
    }

    kprintf("Directory Listing of %s%s%s\n\t", CYAN_FG, path.data, RESET);
    for (size_t i = 0;; i++) {
        INode_t* child = NULL;
        int r = vfs_readdir(dir, i, &child);
        if (r < 0) break;

        kprintf("%s%s  ", child->type == INODE_DIRECTORY ? CYAN_FG : "", (char *)child->internal_data);
        if (child->type == INODE_DIRECTORY) kprintf(CYAN_FG);
        kprintf("%s", RESET);
        vfs_drop(child);
    }

    kprintf("\n");
    vfs_drop(dir);
}

static void run_command(const char *cmd) {
    if (strcmp(cmd, "help") == 0) {
        kprintf("Shitty Shell (tm) Commands:\n");
        kprintf("help: display this help message\n");
        kprintf("echo: print the message following\n");
        kprintf("halt: disables interrupts and stops the cpu\n");
        kprintf("ls: lists the contents of a directory\n");
        kprintf("cat: displays the contents of a file\n");
        kprintf("fault: manually trigger a page fault exception\n");
        kprintf("neofetch: display splash and system info\n");
    }
    else if (strncmp(cmd, "echo ", 5) == 0) {
        kprintf("%s\n", cmd + 5);
    }
    else if (strcmp(cmd, "halt") == 0) {
        kprintf("Your Computer has stopped\nFrom here nothing will happen until you hard-reset\n");
        __asm__ volatile("cli");
        for(;;) __asm__ volatile("hlt");
    }
    else if (strncmp(cmd, "ls", 2) == 0) {
        const char *path = "/";
        if (strlen(cmd) > 3) path = cmd + 3;
        print_directory_list(path);
    }
    else if (strncmp(cmd, "cat ", 4) == 0) {
        const char *path_str = cmd + 4;
        path_t path = vfs_path_from_abs(path_str);
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
                buffer[r] = '\0';
                kprintf("%s\n", buffer);
            }

            kfree(buffer, size);
            inode_drop(inode);
        }
    }
    else if (strncmp(cmd, "fault", 5) == 0){
        *(volatile int*)0 = 1;  //#pf
    }
    else if (strncmp(cmd, "panic", 5) == 0){
        ke_panic("Manually Initiated Panic");
    }
    else if (strncmp(cmd, "sched", 5) == 0) {
        kprintf("%sID\tSTATE\tQUANTUM\tPRIV\tMAPADDR\n%s", GRAY_FG, RESET);
        itterate_each_task(sched_print_task, NULL);
    }
    else if (strncmp(cmd, "reboot", 6) == 0) {
        idt_ptr_t bad_idt = {0, 0};
        asm volatile("lidt %0" :: "m"(bad_idt));
        asm volatile("int3");
    }
    else if (strcmp(cmd, "neofetch") == 0) {
        neofetch();
    }
    else {
        kprintf("Unknown command: %s\n", cmd);
    }
}

void shell_start(void) {
    kprintf(LOG_WARN "This is a very primitive shell that will be removed very soon, thanks for trying out bleed!\n");
    PS2_Keyboard_set_callback(handle_key);  // Ensure the callback is set
    kprintf("%skernel@%sbleed-kernel$ %s", RED_FG, GRAY_FG, RESET);
    draw_cursor();
}
