#include "cos32.h"
#include <stddef.h>

int main(int argc, char **argv)
{
  print("hello\n");
  void* default_font = cos32_video_font_get("Default");
  if (!default_font)
  {
    return -1;
  }
  
  void* pixel_ptr = cos32_video_font_make_empty_string(default_font, 12);
  if (!pixel_ptr)
  {
    return -1;
  }

  cos32_video_font_draw(default_font, pixel_ptr, "ABC world!");
  
  void *taskbar_rect = cos32_video_rectangle_new(0, 0, 180, 100);
       cos32_video_rectangle_fill(taskbar_rect, 4);
  cos32_video_rectangle_draw_font_data(taskbar_rect, default_font, pixel_ptr, 0, 0, 12);

  while (1)
  {
  }
  return 0;
}