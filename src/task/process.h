#ifndef PROCESS_H
#define PROCESS_H

#include "config.h"
#include "task.h"
#include <stdbool.h>
struct process
{
    char filename[COS32_MAX_PATH];
    // Each process has a task for its self
    struct task task;

    // The physical pointer to the process memory
    void *ptr;

    // The physical pointer to the stack memory
    void *stack;

    // The physical size of the data pointed to by the pointer
    uint32_t size;

    // True if this process was ever started
    bool started;

    // This is the keyboard buffer for this process, any keyboard interrupts that happen will write to the buffer of the current process
    struct keyboard_buffer
    {
        char buffer[COS32_KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    } keyboard;
};


int process_page();
int process_load(const char *filename, struct process **process);
int process_switch(struct process *process);
int process_start(struct process *process);
bool process_running();
void process_mark_running(bool running);
struct process *process_current();

#endif