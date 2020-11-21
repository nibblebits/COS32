#include "cos32.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <stddef.h>

int main(int argc, char **argv)
{
  struct kernel_info info;
  kernel_information(&info);

  printf("COS32 Shell");
  printf("abcdef NEW LINE CAN WORK NOW WE NEED TO IMPLEMENTOVERLAPPING!!!geowgkowpgokewpokgokewhokpwokphkoperkopherpokhokperpokherkopherpokerhkpherokp");
  //printf("Kernel version: D%i-B%i fewioewokfodggew,ogkewgok43op34pko4g3pogk34opko\n", info.date, info.build_no);

  char *shell_buf = malloc(512);

  void* ptr = cos32_video_rectangle_new(100, 80, 120, 120);
  cos32_video_rectangle_fill(ptr, 5);

  cos32_video_rectangle_get("taskbar");
  
  while(1) {}
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