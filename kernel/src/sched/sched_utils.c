#include <sched/scheduler.h>
#include <ansii.h>

const char *task_state_str(task_state_t state) {
    switch (state) {
        case TASK_READY:
            return CYAN_FG "READY" RESET;

        case TASK_RUNNING:
            return GREEN_FG "RUNNING" RESET;

        case TASK_BLOCKED:
            return GRAY_FG "BLOCKED" RESET;

        case TASK_DEAD:
            return RED_FG "DEAD" RESET;

        default:
            return MAGENTA_FG "UNKNOWN" RESET;
    }
}

