#ifndef ARRAY_H
#define ARRAY_H
#include <stddef.h>
struct array
{
    size_t total;
    // Element size
    size_t e_sz;
    void* ptr;
};

struct array* array_create(size_t element_size);
void array_insert(struct array* array, void* ptr);
void* array_get_index(struct array* array, int index);
size_t array_total(struct array* array);
void array_destroy(struct array *array);

#endif