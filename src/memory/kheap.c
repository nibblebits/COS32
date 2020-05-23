#include "kheap.h"
#include "heap.h"
static struct heap* kheap;

void kheap_init()
{
    // Our kernel heap should be just after the stack
    kheap = heap_create((void*)COS32_KERNEL_HEAP_ADDRESS);   
}

void* kmalloc(int size)
{
    return heap_malloc(kheap, size);
}

void kfree(void* ptr)
{
    return heap_free(kheap, ptr);
}