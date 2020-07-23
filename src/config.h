#ifndef CONFIG_H
#define CONFIG_H
#define COS32_KEYBOARD_BUFFER_SIZE 1024
#define COS32_MAX_FILESYSTEMS 10
#define COS32_MAX_FILE_DESCRIPTORS 128
// We allow for around 30 MB of memory, it will be slightly less due to the space some tables will take up
#define COS32_MAX_HEAP_ALLOCATIONS 8000
#define COS32_MAX_HEAP_SIZE 314572800
#define COS32_MEMORY_BLOCK_SIZE 4096
// 13 Megabyte maximum at this region for kernel heap
#define COS32_KERNEL_HEAP_ADDRESS  0x01000000
#define COS32_PROGRAM_VIRTUAL_ADDRESS 0x0400000
#define COS32_USER_PROGRAM_STACK_SIZE 1024*16
#define COS32_PROGRAM_VIRTUAL_STACK_ADDRESS_START 0x0400000

// Stack grows downwards remember
#define COS32_PROGRAM_VIRTUAL_STACK_ADDRESS_END COS32_PROGRAM_VIRTUAL_STACK_ADDRESS_START - COS32_USER_PROGRAM_STACK_SIZE

#define COS32_MAX_PROCESSES 12
#define COS32_MAX_DISKS 4
#define COS32_FORCE_MEMORY_ALIGNMENT 1
#define COS32_SECTOR_SIZE 512
#define COS32_MAX_PATH 108

#define COS32_MAX_INTERRUPTS 512

#define COS32_CODE_SELECTOR 0x08
#define COS32_DATA_SELECTOR 0x10

#define COS32_PAGE_SIZE 4096

#define COS32_TOTAL_GDT_SEGMENTS 6

#define COS32_TOTAL_PAGE_ENTRIES_PER_DIRECTORY 1024


// The maximum amount of malloc allocations a user program can have
#define COS32_MAX_PROGRAM_ALLOCATIONS 1024




#define KERNEL_CODE_SEGMENT 0x8
#define KERNEL_DATA_SEGMENT 0x10

// 0x20 comes to mind but its wrong I'll need to remember why
#define USER_DATA_SEGMENT 0x23

// DOuble check this thing
#define USER_CODE_SEGMENT 0x1b


#endif