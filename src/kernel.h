#ifndef KERNEL_H
#define KERNEL_H

void print(const char* message);
void panic(char* message);
void print_number(int number);

#define ASSERT(value) \
    if (!value) \
    { \
    print(__FUNCTION__); \
    panic(": Assertion failed");\
    }
#endif