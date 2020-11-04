#include "cos32.h"
#include <stddef.h>

int main(int argc, char **argv)
{
  //print("hello");
  //while(1) {}
  void *taskbar_rect = cos32_video_rectangle_new(0, 0, 320, 10);
  while (1)
  {
    for (int i = 0; i < 15; i++)
    {
      cos32_video_rectangle_fill(taskbar_rect, i);
    }

  }

  while (1)
  {
  }
  return 0;
}