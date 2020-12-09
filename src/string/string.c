#include "string.h"
#include "kernel.h"
char *strncpy(char *dest, const char *src, int n)
{
    int i = 0;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';

    return dest;
}

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

char tolower(char s1)
{
    if (s1 >= 65 && s1 <= 90)
    {
        s1 += 32;
    }
    return s1;
}

int strcmp(const char *s1, const char *s2)
{
    unsigned char u1, u2;

    while (1)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            break;
    }
    return 0;
}
int strncmp(const char *s1, const char *s2, int n)
{
    unsigned char u1, u2;

    while (n-- > 0)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }
    return 0;
}

int istrncmp(const char *s1, const char *s2, int n)
{
    unsigned char u1, u2;

    while (n-- > 0)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2 && tolower(u1) != tolower(u2))
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }
    return 0;
}

/**
 * Gets the length of the string, stops when the terminator provided is reached or a null terminator
 */
int strnlen_terminator(const char *str, int max, char terminator)
{
    int i = 0;
    for (i = 0; i < max; i++)
    {
        if (str[i] == '\0' || str[i] == terminator)
            break;
    }

    return i;
}

int strnlen(const char *str, int max)
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

char *sp = NULL; /* the start position of the string */
char *strtok(char *str, const char *delimiters)
{

    int i = 0;
    int len = strlen(delimiters);

    /* if the original string has nothing left */
    if (!str && !sp)
        return NULL;

    /* initialize the sp during the first call */
    if (str && !sp)
        sp = str;

    /* find the start of the substring, skip delimiters */
    char *p_start = sp;
    while (true)
    {
        for (i = 0; i < len; i++)
        {
            if (*p_start == delimiters[i])
            {
                p_start++;
                break;
            }
        }

        if (i == len)
        {
            sp = p_start;
            break;
        }
    }

    /* return NULL if nothing left */
    if (*sp == '\0')
    {
        sp = NULL;
        return sp;
    }

    /* find the end of the substring, and
        replace the delimiter with null */
    while (*sp != '\0')
    {
        for (i = 0; i < len; i++)
        {
            if (*sp == delimiters[i])
            {
                *sp = '\0';
                break;
            }
        }

        sp++;
        if (i < len)
            break;
    }

    return p_start;
}

char* basename(char* filename)
{
    char* current = strtok(filename, "/");
    char* last = current;
    while(current)
    {
        last = current;
        current = strtok(current, "/");
    }

    return last;
}