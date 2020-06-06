#ifndef TASK_H
#define TASK_H
#include "config.h"
#include "memory/paging/paging.h"
struct task
{
    // These are all the heap allocations that this process has, if its not NULL then its allocated
    void *allocations[COS32_MAX_PROGRAM_ALLOCATIONS];

    // The page directory for this task. Upon running this task at any moment this page directory should be enabled for paging
    // When we switch to another task this should not be enabled
    struct paging_4gb_chunk* page_directory;
};

typedef void (*USER_MODE_FUNCTION)();
void user_mode_enter(USER_MODE_FUNCTION func, uint32_t stack_addr);
struct task* task_current();
int task_init(struct task* task);
int task_switch(struct task* task);
#endif