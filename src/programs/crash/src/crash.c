#include "cos32.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <stddef.h>

int main(int argc, char **argv)
{
  printf("The program \"%s\" has crashed. err=%s\n", argv[1], argv[2]);
  printf("Stack trace:\n");
  printf("Death IP: %i\n", argv[3]);

  while(1) {}
}