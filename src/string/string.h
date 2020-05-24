#ifndef STRING_H
#define STRING_H

char *strncpy(char *dest, const char *src, int n);
int strnlen(const char* str, int max);
int isdigit(char c);
int tonumericdigit(char c);
char* strtok(char* input, char delm);

#endif