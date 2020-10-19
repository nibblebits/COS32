#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stdbool.h>


struct kernel_info
{
    unsigned int date;
    unsigned int build_no;
};


void print(const char* message);
void panic(char* message);
void print_number(int number);
char* itoa(int i);
void kernel_page();
bool is_kernel_page();

/**
 * Returns the page directory that the kernel uses
 */
uint32_t* kernel_get_page_directory();


/**
 * Returns the kernel page chunk
 */
struct paging_4gb_chunk* kernel_page_get_chunk();



#define ASSERT(value) \
    if (!(value)) \
    { \
    print(__FUNCTION__); \
    panic(": Assertion failed");\
    }
#endif

#define ERROR(value) (void*)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)value < 0)

