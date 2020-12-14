#include "shell.h"
#include "cos32.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <stddef.h>

int main(int argc, char **argv)
{
  itoa(50);
  while(1) {}
  struct kernel_info info;
  kernel_information(&info);

  printf("COS32 Shell\n");
  printf("Kernel version: D%i-B%i\n", info.date, info.build_no);
  //cos32_flush_video_buffer();
  char *shell_buf = malloc(512);
  while (1)
  {
    printf(">");
    cos32_terminal_readline(shell_buf, 512, true);
    if (cos32_run_command(shell_buf, 512) != 0)
    {
      printf("\nThe command \"%s\" is invalid.", shell_buf);
    }
    printf("\n");
     // cos32_flush_video_buffer();

  }
}