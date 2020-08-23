#ifndef PROCESS_H
#define PROCESS_H

#include "config.h"
#include "task.h"
#include "keyboard/keyboard.h"
// This directory path is rubbish, change the name
#include "formats/elf/elfloader.h"
#include <stdbool.h>


typedef enum ProcessFileType
{
    FILE_TYPE_BINARY,
    FILE_TYPE_ELF
} processfiletype_t;

struct interrupt_frame;
struct process
{
    // The id of this process
    uint8_t id;

    char filename[COS32_MAX_PATH];
    // Each process has a task for its self
    struct task* task;

    processfiletype_t filetype;

    union
    {
        // The physical pointer to the process memory, if this is a raw binary file
        void *ptr;
        // A pointer to the elf file, if this is an elf file. You can find the loaded elf sections here and their addresses
        struct elf_file* elf_file;
    };
    
    // The physical pointer to the stack memory
    void *stack;

    // The physical size of the data pointed to by the pointer
    uint32_t size;

    // True if this process was ever started
    bool started;

    // True if this process is currently awake
    bool awake;

    // This is the keyboard buffer for this process, any keyboard interrupts that happen will write to the buffer of the current process
    struct keyboard_buffer
    {
        char buffer[COS32_KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    } keyboard;


    // The video memory for this process, when we switch to this process the video memory
    // Should be wrote back out overwriting the current screen memory
    struct video* video;
};

int process_load(const char *filename, struct process **process);
int process_switch(struct process *process);
int process_start(struct process *process);
int process_load_start(const char* path);
int process_load_for_slot(const char* filename, struct process** process, int process_slot);
struct process *process_get(int index);
bool process_running();
void process_mark_running(bool running);
struct process *process_current();

/**
 * Frees and unloads the given process
 */
void process_free(struct process* process);



#endif