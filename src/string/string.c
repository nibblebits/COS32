#include "string.h"
char * strncpy(char *dest, char *src, int n)
{
    int i;

    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';

    return dest;
}

int strnlen(char* str, int max)
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