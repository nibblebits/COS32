#include "kheap.h"
#include "heap.h"
#include "memory/memory.h"
#include "kernel.h"

struct heap kernel_heap;

// 5120 4096-blocks for 200MB of ram
// Table is used to determine how much available memory is left in this heap

struct heap_table kernel_heap_table;

void kheap_init()
{
    // We want a kernel heap that has 200MB of storage
    int total_table_entires = COS32_200MB / COS32_PAGE_SIZE;
    kernel_heap_table.total = total_table_entires;
    // At this given heap table address we have around 700 Kilobytes to play with for this table
    // If we overflow we corrupt reserved memory so becareful.
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*) COS32_KERNAL_HEAP_TABLE_ADDRESS;

    void *end = (void *)(COS32_KERNEL_HEAP_ADDRESS + COS32_200MB);
    int res = heap_create(&kernel_heap, (void *)COS32_KERNEL_HEAP_ADDRESS, end, &kernel_heap_table);
    if (ISERR(res))
    {
        panic("Problem creating the kernel heap of 200Mb\n");
    }
}

void *kmalloc(int size)
{
    void *ptr = heap_malloc(&kernel_heap, size);
    return ptr;
}

void *kzalloc(int size)
{
    void *ptr = kmalloc(size);
    memset(ptr, 0, size);
    return ptr;
}


void kfree(void *ptr)
{
    if (!ptr)
        return;

    return heap_free(&kernel_heap, ptr);
}