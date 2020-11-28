#include "cos32.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <stddef.h>

int main(int argc, char **argvs)
{

  struct process_arguments arguments;
  cos32_get_arguments(&arguments);

  char** argv = arguments.argv;
  printf("The program \"%s\" has crashed. err=%s\n", argv[1], argv[2]);
  printf("Stack trace:\n");
  printf("Death IP: %i\n", argv[3]);

  while(1) {}
}