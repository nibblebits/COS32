#include "process.h"
#include "config.h"
#include "status.h"
#include "fs/file.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "string/string.h"
#include "video/video.h"
#include "video/rectangle.h"
#include "idt/idt.h"
#include "loader/formats/elf/elfloader.h"
#include "loader/formats/elf/elf.h"
#include "loader/library.h"

#include "task.h"
#include "kernel.h"

// The current process that was just running or is running
struct process *current_process = 0;
// This is true if the process is running right now
bool process_is_running = 0;

struct process *processes[COS32_MAX_PROCESSES] = {};

static void process_init(struct process *process)
{
    memset(process, 0, sizeof(struct process));
}

struct process *process_current()
{
    return current_process;
}

void process_mark_running(bool running)
{
    process_is_running = running;
}

bool process_running()
{
    return process_is_running;
}

int process_count_command_arguments(struct command_argument *argument)
{
    int i = 0;
    struct command_argument *current = argument;
    while (current)
    {
        i++;
        current = current->next;
    }

    return i;
}

void process_save(struct process *process)
{
    // Save the video memory back into our processes video memory, so when we switch back
    // we can carry on where we left off
    video_save(process->video);
}

void process_restore(struct process *process)
{
    if (current_process != 0)
    {
        // Since we are switching process the task associated with the current process
        // must have its video memory mapped back to its internal private video memory
        // so that any prints made by this task do not write to the terminal
        task_map_video_memory(current_process->task);
    }
    current_process = process;
    process_is_running = true;

    // We now need to unmap the restored processes video memory so that it outputs data directly to the terminal
    // rather than its internal private video memory
    task_unmap_video_memory(process->task);

    // Restore the processes video memory back into the main video memory
    // So what was once shown shows again on the screen
    video_restore(process->video);
}

int process_switch(struct process *process)
{
    if (current_process != 0)
    {
        // We must save the old process state
        process_save(current_process);
    }

    // Restore the given process
    process_restore(process);
    return 0;
}

void process_get_arguments(struct process *process, int *argc, char ***argv)
{
    *argc = process->arguments.argc;
    *argv = process->arguments.argv;
}

/**
 * Injects the arguments into the process stack, used to inject argv, argc.
 * 
 * All the argument strings are injected along with the total amount of elements.
 */
int process_inject_arguments(struct process *process, struct command_argument *root_argument)
{
    int res = 0;
    struct command_argument *current = root_argument;
    int i = 0;

    // We must create enough room to store all the pointers that we have
    int argc = process_count_command_arguments(root_argument);
    if (argc == 0)
    {
        res = -EIO;
        goto out;
    }

    char **argv = process_malloc(process, sizeof(const char *) * argc);
    if (!argv)
    {
        res = -ENOMEM;
        goto out;
    }

    uint32_t directory_out, table_out = 0;
    paging_get_indexes(argv, &directory_out, &table_out);

    // Now we have the memory we need let's start setting it
    while (current)
    {
        // We have to copy the argument we can't just pass it in like this corruption would be likely
        char *argument_str = process_malloc(process, sizeof(current->argument));
        if (!argument_str)
        {
            res = -ENOMEM;
            goto out;
        }

        // We must now copy the string to this process
        strncpy(argument_str, current->argument, sizeof(current->argument));

        argv[i] = argument_str;
        current = current->next;
        i++;
    }

    // In previous implementations we would push to the tasks stack, in this implementation
    // We will just remember the pointers for the process to poll us for later on
    process->arguments.argc = argc;
    process->arguments.argv = argv;

out:
    return res;
}

int process_load_start(const char *path, struct process *parent, PROCESS_FLAGS flags, struct command_argument *root_argument)
{
    int res = 0;
    struct process *process = 0x00;
    res = process_load(path, &process, parent, flags);
    if (ISERR(res))
    {
        goto out;
    }

    // Do we have any arguments to inject into the process?
    if (root_argument)
    {
        res = process_inject_arguments(process, root_argument);
        if (ISERR(res))
        {
            goto out;
        }
    }


    // Start the process :)
    res = process_start(process);
    if (ISERR(res))
    {
        goto out;
    }

out:
    return res;
}

int process_start(struct process *process)
{
    if (process->started)
        return -EIO;

    process->started = true;

    // As we have started a process we should switch to it
    process_switch(process);

    return 0;
}

int process_get_free_slot()
{
    for (int i = 0; i < COS32_MAX_PROCESSES; i++)
    {
        if (processes[i] == 0)
            return i;
    }

    return -1;
}

struct process *process_get(int index)
{
    // Out of bounds
    if (index < 0 || index >= COS32_MAX_PROCESSES)
    {
        return NULL;
    }
    return processes[index];
}

static int process_load_binary_data(const char *filename, struct process *process)
{
    int res = 0;
    int fd = fopen(filename, "r");
    if (!fd)
    {
        res = -EIO;
        goto out;
    }

    struct file_stat stat;
    res = fstat(fd, &stat);
    if (res != COS32_ALL_OK)
    {
        goto out;
    }

    // Let's create some memory for this program
    void *program_data_ptr = kzalloc(stat.filesize);
    if (!program_data_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    // Now load it in ;)  (Note we are loading it in as one block here, this may be problematic)
    if (fread(program_data_ptr, stat.filesize, 1, fd) != 1)
    {
        res = -EIO;
        goto out;
    }

    process->filetype = FILE_TYPE_BINARY;
    process->ptr = program_data_ptr;
    process->size = stat.filesize;

    fclose(fd);

out:
    return res;
}

static int process_load_elf(const char *filename, struct process *process)
{
    int res = 0;
    struct elf_file *elf_file = 0;
    res = elf_load(filename, &elf_file);
    if (ISERR(res))
    {
        goto out;
    }

    // We can't execute what is not an executable!
    if (!elf_is_executable(elf_header(elf_file)))
    {
        res = -EINVARG;
        goto out;
    }
    
    process->filetype = FILE_TYPE_ELF;
    process->elf_file = elf_file;

out:
    if (ISERR(res))
    {
        elf_close(elf_file);
    }
    return res;
}

static int process_load_data(const char *filename, struct process *process)
{
    int res = 0;
    res = process_load_elf(filename, process);
    if (res == -EINFORMAT)
    {
        res = process_load_binary_data(filename, process);
    }
    return res;
}

int process_map_binary(struct process *process)
{
    int res = 0;
    if (process->filetype != FILE_TYPE_BINARY)
    {
        res = -EINVARG;
        goto out;
    }
    ASSERT(paging_map_to(process->task->page_directory->directory_entry, (void *)COS32_PROGRAM_VIRTUAL_ADDRESS, process->ptr, paging_align_address(process->ptr + process->size), PAGING_PAGE_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_PAGE_WRITEABLE) == 0);
out:
    return res;
}

int process_map_elf(struct process *process)
{
    int res = 0;

    if (process->filetype != FILE_TYPE_ELF)
    {
        res = -EINVARG;
        goto out;
    }

    struct elf_file *elf_file = process->elf_file;
    ASSERT(paging_map_to(process->task->page_directory->directory_entry, paging_align_to_lower_page(elf_virtual_base(elf_file)), elf_phys_base(elf_file), paging_align_address(elf_phys_end(elf_file)), PAGING_PAGE_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_PAGE_WRITEABLE) == 0);

out:
    return res;
}

int process_run_for_argument(struct command_argument *root_argument, struct process *parent, PROCESS_FLAGS flags)
{
    if (root_argument == 0)
    {
        return -EINVARG;
    }

    const char *program_name = root_argument->argument;
    // This could be better in future we should take a name and try to get a realpath()
    // Change the below code to use the path parser this is legacy now...
    char path[COS32_MAX_PATH];
    strncpy(path, "0:/", sizeof(path));
    strncpy(path + 3, program_name, sizeof(path));

    return process_load_start(path, parent, flags, root_argument);
}

int process_map_memory(struct process *process)
{
    int res = 0;

    if (process->filetype == FILE_TYPE_BINARY)
    {
        res = process_map_binary(process);
    }
    else if (process->filetype == FILE_TYPE_ELF)
    {
        res = process_map_elf(process);
    }

    if (!ISERR(res))
    {
        // Map the stack if we have no problems so far, we map from the end of the stack because stack grows downwards
        ASSERT(paging_map_to(process->task->page_directory->directory_entry, (void *)COS32_PROGRAM_VIRTUAL_STACK_ADDRESS_END, process->stack, paging_align_address(process->stack + COS32_USER_PROGRAM_STACK_SIZE), PAGING_ACCESS_FROM_ALL | PAGING_PAGE_PRESENT | PAGING_PAGE_WRITEABLE) == 0);
    
        // Maps all the loaded libraries in memory for this task.
        // This way the user process will be able to see their memory.
        library_map_all(process->task);        
    }


    return res;
}

/**
 * Returns the video memory this process should be using based on the flags provided.
 * This function may create new video memory
 */
static struct video *process_get_video(PROCESS_FLAGS flags, struct process *parent)
{
    return flags & PROCESS_USE_PARENT_VIDEO_MEMORY ? parent->video : video_new();
}

struct command_argument *process_arguments_create(const char *starting_argument)
{
    struct command_argument *argument = process_argument_create_one();
    if (!argument)
    {
        return 0;
    }

    strncpy(argument->argument, starting_argument, sizeof(argument->argument));
    return argument;
}

struct command_argument *process_argument_create_one()
{
    return kzalloc(sizeof(struct command_argument));
}

struct command_argument *process_arguments_get_last(struct command_argument *root_argument)
{
    struct command_argument *current = root_argument;
    while (current)
    {
        if (current->next == 0)
            return current;

        current = current->next;
    }

    return 0;
}

struct command_argument *process_arguments_add(struct command_argument *arguments, const char *argument)
{
    struct command_argument *last_arg = process_arguments_get_last(arguments);
    struct command_argument *new_argument = process_argument_create_one();
    last_arg->next = new_argument;

    strncpy(new_argument->argument, argument, sizeof(new_argument->argument));
    return new_argument;
}

void process_argument_destory(struct command_argument *argument)
{
    kfree(argument);
}

void process_arguments_destory(struct command_argument *root_argument)
{
    struct command_argument *current = root_argument;
    while (current)
    {
        struct command_argument *next_arg = current->next;
        process_argument_destory(current);
        current = next_arg;
    }
}

int process_crash(struct process *process, int error_code)
{
    struct command_argument *root_argument = 0;
    int res = 0;

    // Store the filename of the crashed process
    char filename[COS32_MAX_PATH];
    strncpy(filename, process->filename, sizeof(filename));

    // itoa recyles the same memory, this can be a problem so we must copy the data
    char error_code_str[10];
    memset(error_code_str, 0, sizeof(error_code_str));
    strncpy(error_code_str, itoa(error_code), sizeof(error_code_str));

    char death_ip_str[20];
    strncpy(death_ip_str, itoa((int)process->task->registers.ip), sizeof(death_ip_str));

    // Terminate the program that crashed
    res = process_terminate(process, error_code);
    if (res < 0)
    {
        goto out;
    }

    // Prepare our crash program so we can alert we had to terminate a process
    root_argument = process_arguments_create("0:/bin/crash.e");
    if (!root_argument)
    {
        goto out;
    }

    process_arguments_add(root_argument, filename);
    process_arguments_add(root_argument, error_code_str);
    process_arguments_add(root_argument, death_ip_str);

    // Let's now load in the crash program to explain what happend
    process_load_start("0:/bin/crash.e", NULL, 0, root_argument);
out:
    if (root_argument)
    {
        process_arguments_destory(root_argument);
    }
    return res;
}

int process_terminate(struct process *process, int error_code)
{
    // Free the current process
    process_free(process);
    return 0;
}

int process_load_for_slot(const char *filename, struct process **process, int process_slot, struct process *parent, PROCESS_FLAGS flags)
{
    int res = 0;
    struct task *task = 0;
    struct process *_process = 0;
    void *program_stack_ptr = 0;

    // Flag to use parent video memory provided but NULL parent?
    if ((flags & PROCESS_USE_PARENT_VIDEO_MEMORY) && !parent)
    {
        res = -EINVARG;
        goto out;
    }

    // A process with the given id is already taken
    if (process_get(process_slot) != 0)
    {
        res = -EISTKN;
        goto out;
    }

    _process = kzalloc(sizeof(struct process));
    if (!_process)
    {
        res = -ENOMEM;
        goto out;
    }

    process_init(_process);
    res = process_load_data(filename, _process);
    if (res < 0)
    {
        goto out;
    }

    // Let's now create a 16K stack
    program_stack_ptr = kzalloc(COS32_USER_PROGRAM_STACK_SIZE);
    if (!program_stack_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    strncpy(_process->filename, filename, sizeof(_process->filename));
    _process->parent = parent;
    _process->flags = flags;
    _process->stack = program_stack_ptr;

    _process->video = process_get_video(flags, parent);
    if (!_process->video)
    {
        res = -ENOMEM;
        goto out;
    }

    // The default rectangles that should be shown for each process must be registered to us
    video_rectangle_register_default_rectangles(_process->video);

    task = task_new(_process);
    if (ERROR_I(task) <= 0)
    {
        res = ERROR_I(task);
        goto out;
    }

    _process->task = task;
    // We now need to map the process memory into real memory
    res = process_map_memory(_process);
    if (res < 0)
    {
        goto out;
    }

    // Set the process ID so we can reference it later.
    _process->id = process_slot;

    // We have the program loaded :o
    *process = _process;

    // Add the process to the array
    processes[process_slot] = _process;

out:
    if (ISERR(res))
    {
        // Free the task here
        if (_process && _process->task)
        {
            task_free(_process->task);
        }
        // Free the process data here
    }
    return res;
}

static struct process *process_get_first_ignore(struct process *process)
{
    for (int i = 0; i < COS32_MAX_PROCESSES; i++)
    {
        if (processes[i] != 0 && processes[i] != process)
        {
            return processes[i];
        }
    }

    return NULL;
}

void **process_get_free_heap_allocation_slot(struct process *process)
{
    for (int i = 0; i < COS32_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        if (process->allocations[i] == 0)
            return &process->allocations[i];
    }

    return 0;
}

int process_paging_map_to(struct process *process, void *virt, void *phys, void *phys_end, int flags)
{
    // Currently only one task per process exists
    return paging_map_to(process->task->page_directory->directory_entry, virt, phys, phys_end, flags);
}

void *process_malloc(struct process *process, int size)
{

    ASSERT(is_kernel_page());

    int res = 0;
    void *ptr = 0;
    void **allocation_slot = process_get_free_heap_allocation_slot(process);
    if (!allocation_slot)
    {
        goto out;
    }

    ptr = kmalloc(size);
    if (!ptr)
    {
        goto out;
    }

    *allocation_slot = ptr;
    // Now we must map the page for the data, mapping is essential because the current page will be supervisor only
    res = process_paging_map_to(process, ptr, ptr, paging_align_address(ptr + size), PAGING_ACCESS_FROM_ALL | PAGING_CACHE_DISABLED | PAGING_PAGE_WRITEABLE | PAGING_PAGE_PRESENT);
    if (res < 0)
    {
        panic("Mapping of process memory failed\n");
        goto out;
    }

out:
    if (ISERR(res))
    {
        kfree(ptr);
    }
    return ptr;
}

static struct process *process_get_first()
{
    return process_get_first_ignore(NULL);
}

void process_free_binary_data(struct process *process)
{
    // Free the binary data
    kfree(process->ptr);
}

void process_free_elf_data(struct process *process)
{
    // Close the elf file
   // elf_close(process->elf_file);
}

void process_free_data(struct process *process)
{
    switch (process->filetype)
    {
    case FILE_TYPE_BINARY:
        process_free_binary_data(process);
        break;

    case FILE_TYPE_ELF:
        process_free_elf_data(process);
        break;

    default:
        panic("Unknown process file type, cannot free!\n");
    }
}

static void process_free_allocations(struct process *process)
{
    for (int i = 0; i < COS32_MAX_PROGRAM_ALLOCATIONS; i++)
    {
        if (process->allocations[i] != 0)
        {
            kfree(process->allocations[i]);
        }
    }
}

void process_terminate_subprocesses(struct process *process)
{
    // We must loop through all the processes and if their parent is the one provided we must terminate them
    for (int i = 0; i < COS32_MAX_PROCESSES; i++)
    {
        if (processes[i] != 0 && processes[i]->parent == process)
        {
            process_free(processes[i]);
        }
    }
}

void process_wake(struct process *process)
{
    process->awake = true;
    // Awake the main process task
    task_wake(process->task);
}

void process_pause(struct process *process)
{
    process->awake = false;
    // Pause the main process task
    task_pause(process->task);
}

void process_free(struct process *process)
{
    /**
    * In the event the system runs out of running processes, unsure how to respond
    * maybe we start a new process or just panic the system like this....
    */
    struct process *next_process = process_get_first_ignore(process);
    if (!next_process)
    {
        panic("No more processes available to switch to\n");
    }

    // If we are the current process we need to swap it as we are about to delete ourselves
    if (process_current() == process)
    {
        process_switch(next_process);
    }

    // We must first terminate all subprocesses
    process_terminate_subprocesses(process);

    // Delete the process allocations
    process_free_allocations(process);

    // Delete the task in question
    task_free(process->task);

    // Let's delete the process data
    process_free_data(process);

    // Free the video memory, only if we are the ones responsible for it
    if (!(process->flags & PROCESS_USE_PARENT_VIDEO_MEMORY))
    {
        video_free(process->video);
    }

    if (process->flags & PROCESS_UNPAUSE_PARENT_ON_DEATH)
    {
        process_wake(process->parent);
    }
    // Delete the process memory
    kfree(process);

    // Let's free up the process slot
    processes[process->id] = 0;
}

int process_load(const char *filename, struct process **process, struct process *parent, PROCESS_FLAGS flags)
{
    int res = 0;
    int process_slot = process_get_free_slot();
    if (process_slot < 0)
    {
        res = -ENOMEM;
        goto out;
    }

    res = process_load_for_slot(filename, process, process_slot, parent, flags);
out:
    return res;
}
