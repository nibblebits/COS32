#include "cos32.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <stddef.h>

int main(int argc, char **argv)
{
  struct kernel_info info;
  kernel_information(&info);


  printf("COS32 Shell\n");
  printf("Kernel version: D%i-B%i\n", info.date, info.build_no);

  char *shell_buf = malloc(512);



  char* ptr = (char*)0xA0000;
  for (int i = 0; i < 4096; i++)
  {
    ptr[i] = 1;
  }
  while (1)
  {
    printf(">");
    cos32_terminal_readline(shell_buf, 512, true);
    if (cos32_run_command(shell_buf, 512) != 0)
    {
      printf("\nThe command \"%s\" is invalid.", shell_buf);
    }
    printf("\n");
  }
}