#ifndef LIBRARY_H
#define LIBRARY_H

#include "config.h"
#include "memory/array.h"
#include <stddef.h>

#define LIBRARY_NAME_MAX 256
#define SECTION_NAME_MAX 256
#define SYMBOL_NAME_MAX 256

struct task;
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
struct library* library_get(const char* name);
void library_build_address(void* virt, void* phys, struct addr* addr_out);

int library_new_symbol(struct library* library, const char* name, struct addr* addr);
struct symbol* library_get_symbol_by_name(struct library* library, const char* name);

int library_new_section(struct library *library, const char *name, struct addr *addr, size_t size);
struct section* library_get_section(struct library* library, int index);
int library_sections_count(struct library* library);

/**
 * Maps the given library into memory for the task.
 * The library must be loaded
 */
int library_map(struct task* task, struct library* library);

/**
 * Maps all loaded libraries into memory for the given task.
 */
int library_map_all(struct task *task);

#endif