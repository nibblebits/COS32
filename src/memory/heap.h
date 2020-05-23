#ifndef HEAP_H
#define HEAP_H
#include "config.h"

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01

// Entries are one byte in length and are bitmasks
typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;
struct heap_block_table
{
    HEAP_BLOCK_TABLE_ENTRY entry[COS32_MAX_HEAP_ALLOCATIONS];
};

struct heap_blocks
{
    // Pointer to the first block entry in the heap_block_table
    HEAP_BLOCK_TABLE_ENTRY *ptr;
    int total;

    // The index of the first block in the heap blocks table
    int sindex;
};

struct heap_entry
{
    struct heap_blocks blocks;
    void* data_ptr;
};

struct heap
{
    struct heap_block_table table;
    struct heap_entry entries[COS32_MAX_HEAP_ALLOCATIONS];

    // Actual data for our heap
    char data[COS32_MAX_HEAP_ALLOCATIONS * COS32_MEMORY_BLOCK_SIZE];
};

/**
 * Creates a heap at the given "ptr". We require at least 28952 bytes of memory available for the "ptr" provided
 */
struct heap *heap_create(void *ptr);
void *heap_malloc(struct heap *heap, int total);
void heap_free(struct heap *heap, void *ptr);

#endif