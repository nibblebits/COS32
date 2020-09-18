#ifndef PROCESS_H
#define PROCESS_H

#include "config.h"
#include "task.h"
#include "keyboard/keyboard.h"
// This directory path is rubbish, change the name
#include "formats/elf/elfloader.h"
#include <stdbool.h>


struct command_argument
{
    char argument[512];
    struct command_argument* next;
};


typedef enum ProcessFileType
{
    FILE_TYPE_BINARY,
    FILE_TYPE_ELF
} processfiletype_t;

typedef unsigned char PROCESS_FLAGS;

#define PROCESS_USE_PARENT_VIDEO_MEMORY 0b00000001
#define PROCESS_UNPAUSE_PARENT_ON_DEATH 0b00000010

struct interrupt_frame;
struct process
{
    // The id of this process
    uint8_t id;

    
    char filename[COS32_MAX_PATH];
    // Each process has a task for its self
    struct task* task;

    // These are all the heap allocations that this process has, if its not NULL then its allocated
    // Limiting the user process to maximum allocations is not the best idea
    // maybe switch to linked list..
    void *allocations[COS32_MAX_PROGRAM_ALLOCATIONS];

    processfiletype_t filetype;

    // FLags relating to this process
    PROCESS_FLAGS flags;

    // The parent process, otherwise NULL
    struct process* parent;
    
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

    // True if this is a subprocess
    bool subprocess;

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

int process_load(const char *filename, struct process **process, struct process* parent, PROCESS_FLAGS flags);
int process_switch(struct process *process);
int process_start(struct process *process);

void process_wake(struct process* process);
void process_pause(struct process* process);


/**
 * Loads and starts the process with the given arguments
 * argv[0] is the process to load
 */
int process_run_for_argument(struct command_argument *root_argument, struct process* parent, PROCESS_FLAGS flags);
int process_load_start(const char *path, struct process* parent, PROCESS_FLAGS flags);
int process_load_for_slot(const char *filename, struct process **process, int process_slot, struct process* parent, PROCESS_FLAGS flags);
struct process *process_get(int index);
bool process_running();
void process_mark_running(bool running);
struct process *process_current();

/**
 * Allocates memory for the given process
 */
void *process_malloc(struct process *process, int size);

/**
 * Frees and unloads the given process
 */
void process_free(struct process* process);

/**
 * Maps pages into memory starting at the physical address until the physical end address is reached.
 * Pages are mapped into the address starting at "virt"
 * 
 * All tasks for the process are mapped 
 * 
 * All provided addresses must divide into a page and if they do not the system will panic
 * 
 * /param process The process to map the memory for
 * /param virt The virtual address to start mapping these pages to
 * /param phys The start physical address so map (must divide into a page)
 * /param phys_end The end physical address to map (must divide into a page)
 */
int process_paging_map_to(struct process* process, void *virt, void *phys, void *phys_end, int flags);


#endif