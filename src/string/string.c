#include "string.h"
#include "kernel.h"
char * strncpy(char *dest, const char *src, int n)
{
    int i;

    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';

    return dest;
}

int strnlen(const char* str, int max)
{
    int i = 0;
    for (i = 0; i < max; i++)
    {
        if (str[i] == '\0')
            break;
    }

    return i;
}

int isdigit(char c)
{
    return c >= 48 && c <= 57;
}

/**
 * Probably best to move this to another place
 */
int tonumericdigit(char c)
{
    return c - 48;
}

char* sp = 0;
char* strtok(char* input, char delm)
{
    int i = 0;
    if (input == 0)
        input = sp;

    if (input == 0)
        return 0;
        
    while(input[i] != 0 && input[i] != delm) 
    {
        i++;
    }

    sp = 0;
    if (input[i] != 0)
    {
        sp = &input[i+1];
    }

    input[i] = 0x00;
    return input;
}