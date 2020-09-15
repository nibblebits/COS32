#ifndef STRING_H
#define STRING_H
#include <stddef.h>

char *strtok(char *str, const char *delimiters);
size_t strlen(const char *str);
int strnlen(const char *str, int max);
char *strncpy(char *dest, const char *src, int n);
char *strcpy(char *dest, const char *src);

#endif