/*
    THIS WILL SOON BE REPLACED WITH AN ACTUAL SHELL, THIS IS JUST A BULLSHIT THING TO HELP TEST
    BEFORE I GET THERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#include <panic.h>
#include <drivers/ps2/ps2_keyboard.h>
#include <drivers/framebuffer/framebuffer.h>
#include <sched/scheduler.h>
#include <stdio.h>
#include <string.h>
#include <fs/fsutils.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <ansii.h>
#include <threads/exit.h>

#define LINE_BUF    256

static char line[LINE_BUF];
static int pos = 0;

#define CURSOR_COLOR 0xFFFFFFFF

static cursor prev_cursor = {0, 0};
static void draw_cursor() {
    psf_font_t *font = get_tty_font();
    cursor c = get_cursor_pos();

    if (!font) return;

    uint32_t *fb_ptr = (uint32_t *)get_framebuffer_addr();
    size_t pitch = get_framebuffer_pitch();

    // Erase previous cursor underline
    size_t px_base = prev_cursor.x * font->width;
    size_t py_base = prev_cursor.y * font->height + font->height - 1;
    for (size_t x = 0; x < font->width; x++) {
        fb_ptr[py_base * pitch + (px_base + x)] = 0x00000000;
    }

    // Draw underline at current cursor position
    px_base = c.x * font->width;
    py_base = c.y * font->height + font->height - 1;
    for (size_t x = 0; x < font->width; x++) {
        fb_ptr[py_base * pitch + (px_base + x)] = CURSOR_COLOR;
    }

    prev_cursor = c;
}

/// @brief print task stuff
/// @param task task structure
/// @param userdata goes unused here, just null!
static void sched_print_task(task_t *task, void *userdata) {
    (void)userdata;

    if (task->state == TASK_READY ||
        task->state == TASK_RUNNING ||
        task->state == TASK_DEAD) {

        kprintf("%s%llu%s\t%s\t%u\n",
            CYAN_FG, task->id, RESET,
            task_state_str(task->state),
            task->quantum_remaining
        );
    }
}

void task_exit_test(void) {
    kprintf("[Test Task] Starting and will exit immediately\n");
    exit();
    kprintf("[Test Task] ERROR: Task did not exit!\n");
    for (;;) __asm__("hlt");
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
        list_directory(path);
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
    else if (strncmp(cmd, "panic", 5) == 0){
        ke_panic("Manually Initiated Panic");
    }
    else if (strncmp(cmd, "sched", 5) == 0) {
        kprintf("ID\tSTATE\tQUANTUM\n");
        itterate_each_task(sched_print_task, NULL);
    }
    else if (strncmp(cmd, "testexit", 8) == 0) {
    int tid = sched_create_task(task_exit_test);
    kprintf("[Shell] Created test exit task with ID %u\n", tid);

    kprintf("Current Scheduler List:\n");
    kprintf("ID\tSTATE\tQUANTUM\n");
    itterate_each_task(sched_print_task, NULL);
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
        kprintf("%skernel@%sbleed-kernel$ %s", RED_FG, GRAY_FG, RESET);
        draw_cursor();
        return;
    }

    if (c == '\b') {
        if (pos > 0) {
            pos--;
            kprintf("\b \b");
        }
        draw_cursor();
        return;
    }

    if (pos < LINE_BUF - 1) {
        line[pos++] = c;
        kprintf("%c", c);
    }

    draw_cursor();
}


void shell_start(void) {
    kprintf(LOG_WARN "This is a very primitive shell that will be removed very soon, thanks for trying out bleed!\n");
    keyboard_set_callback(handle_key);
    kprintf("%skernel@%sbleed-kernel$ %s", RED_FG, GRAY_FG, RESET);
    draw_cursor();
}