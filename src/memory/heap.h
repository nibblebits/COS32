#ifndef HEAP_H
#define HEAP_H
#include "config.h"
#include <stdint.h>
#include <stddef.h>

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0x00

#define HEAP_BLOCK_HAS_NEXT 0b10000000
#define HEAP_BLOCK_IS_FIRST 0b01000000

// Entries are one byte in length and are bitmasks
// Lower 4 bits are the entry type
// Upper 4 bits are flags for this heap block entry
typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct heap_table
{
    HEAP_BLOCK_TABLE_ENTRY *entries;
    size_t total;
};

struct heap
{
    // This is the heap table where memories of the allocations are stored
    // e.g which blocks are taken
    struct heap_table* table;
    // The start address for this heap
    void *saddr;
};

/**
 * Creates a heap starting at the provided address and ending at the provided end address
 * Caller also has to pass the table which must be valid for the given start and end addresses.
 * 
 * The heap pointer provided is the one thats initialized
 * 
 * Return 0 on success
 */
int heap_create(struct heap *heap, void *ptr, void *end, struct heap_table *table);
void *heap_malloc(struct heap *heap, size_t size);
void heap_free(struct heap *heap, void *ptr);

#endif