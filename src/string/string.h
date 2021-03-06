#ifndef STRING_H
#define STRING_H

#include <stddef.h>

char *strncpy(char *dest, const char *src, int n);
size_t strlen(const char *str);
int strnlen(const char* str, int max);
int strnlen_terminator(const char *str, int max, char terminator);
int isdigit(char c);
int tonumericdigit(char c);
char *strtok(char *str, const char *delimiters);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, int n);
int istrncmp(const char *s1, const char *s2, int n);
char tolower(char s1);
/**
 * Accepts a full path and returns a relative path.
 * E.g "0:/abc/test.c" will return test.c
 */
void basename(const char* filename, char* out_filename, int max_len);

#endif