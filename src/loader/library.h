#ifndef LIBRARY_H
#define LIBRARY_H

#include "config.h"
#include "memory/array.h"
#include <stddef.h>

#define LIBRARY_NAME_MAX 256
#define SECTION_NAME_MAX 256
#define SYMBOL_NAME_MAX 256

struct library;

struct addr
{
    void *phys;
    void *virt;
};

struct symbol
{
    char name[SYMBOL_NAME_MAX];
    struct addr addr;
    struct library *library;
};

struct section
{
    char name[SECTION_NAME_MAX];
    struct addr addr;
    size_t size;
    struct library *library;
};


struct library
{
    char name[LIBRARY_NAME_MAX];
    struct array* sections;
    struct array* symbols;
    struct library *next;
};

void library_insert(struct library* library);
struct library* library_new(const char* name);
int library_new_symbol(struct library* library, const char* name, struct addr* addr);

#endif