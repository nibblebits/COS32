#ifndef COS32_H
#define COS32_H

#include <stdbool.h>

struct kernel_info
{
    unsigned int date;
    unsigned int build_no;
};

struct command_argument
{
    char argument[512];
    struct command_argument* next;
};

/*
 * Executes the given shell command, provided "max" should be equal to the total buffer size
 */
bool cos32_run_command(const char *command, int max);
int cos32_invoke_command(struct command_argument* root_command);
void cos32_putchar(char c);
char cos32_getkey();
char cos32_getkeyblock();
void print(const char* message);
void kernel_information(struct kernel_info* info);
void* cos32_malloc(int size);
/**
 * Reads a line from the terminal, if you pass output_while_typing as true then
 * each individual character will be outputted to the terminal as they type.
 * We will return from this function upon a carriage return in which case the "out" buffer
 * will be populated with the entered string
 */
void cos32_terminal_readline(char* out, int max, bool output_while_typing);
#endif