#ifndef STRING_H
#define STRING_H

char *strncpy(char *dest, const char *src, int n);
int strnlen(const char* str, int max);
int strnlen_terminator(const char *str, int max, char terminator);
int isdigit(char c);
int tonumericdigit(char c);
char* strtok(char* input, char delm);
int strncmp(const char *s1, const char *s2, int n);
int istrncmp(const char *s1, const char *s2, int n);
char tolower(char s1);

#endif