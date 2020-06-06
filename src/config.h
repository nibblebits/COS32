#ifndef CONFIG_H
#define CONFIG_H
#define COS32_MAX_FILESYSTEMS 10
#define COS32_MAX_FILE_DESCRIPTORS 128
// We allow for around 10 MB of memory, it will be slightly less due to the space some tables will take up
#define COS32_MAX_HEAP_ALLOCATIONS 2460
#define COS32_MAX_HEAP_SIZE 10485760
#define COS32_MEMORY_BLOCK_SIZE 4096
// 13 Megabyte maximum at this region for kernel heap
#define COS32_KERNEL_HEAP_ADDRESS  0x01000000
#define COS32_PROGRAM_VIRTUAL_ADDRESS 0x0400000
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


// The maximum amount of malloc allocations a user program can have
#define COS32_MAX_PROGRAM_ALLOCATIONS 1024
#endif