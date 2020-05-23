#ifndef KHEAP_H
#define KHEAP_H

void kheap_init();
void* kmalloc(int size);
void kfree(void* ptr);

#endif