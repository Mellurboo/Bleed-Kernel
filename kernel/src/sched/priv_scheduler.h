#ifndef PRIV_SCHEDULER_H
#define PRIV_SCHEDULER_H

#include <sched/scheduler.h>

extern task_t *current_task;
extern task_t *task_queue;
extern task_t *task_list_head;

extern task_t *dead_task_head;
extern task_t *dead_task_tail;

#endif