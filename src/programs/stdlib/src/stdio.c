#include <stdarg.h>
#include "stdlib.h"
#include "cos32.h"

void putchar(char c)
{
    cos32_putchar(c);
}

void printf(char *fmt,...)
{
    va_list ap;
    char *p, *sval;
    int ival;

    va_start(ap, fmt);
    for(p = fmt; *p; p++) {
        if(*p != '%') {
            putchar(*p);
            continue;
        }
        switch (*++p) {
            case 'i':
                ival = va_arg(ap, int);
                print(itoa(ival));
                break;
            case 's':
                sval = va_arg(ap, char *); 
                print(sval);
                break;
            default:
                putchar(*p);
                break;
        }
    }
    va_end(ap); 
}