#include "kheap.h"
#include "heap.h"
#include "memory/memory.h"
#include "kernel.h"

static struct heap* kheap;

void kheap_init()
{
    // Our kernel heap should be just after the stack
    kheap = heap_create((void*)COS32_KERNEL_HEAP_ADDRESS);   
}

void* kmalloc(int size)
{
    void* ptr = heap_malloc(kheap, size);
    // While assertions are enabled if we fail to allocate we should panic the kernel
    ASSERT(ptr >= kheap);
    return ptr;
}

void* kzalloc(int size)
{
    void* ptr = kmalloc(size);
    memset(ptr, 0, size);
    return ptr;
}

void kfree(void* ptr)
{
    return heap_free(kheap, ptr);
}