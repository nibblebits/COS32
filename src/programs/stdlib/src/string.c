#include "string.h"
#include <stddef.h>
#include <stdbool.h>

int strlen(const char *str)
{
    int i = 0;
    while (1)
    {
        if (str[i] == 0)
        {
            break;
        }
        i++;
    }

    return i;
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