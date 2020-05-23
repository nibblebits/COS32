#ifndef MEMORY_H
#define MEMORY_H

void memset(void* ptr, char v, int len);
void* memcpy (void *dest,  void *src, int len);
int memcmp (void* s1, void* s2, int count);

#endif