#include "cos32.h"
#include "string.h"
#include <stddef.h>

int main(int argc, char **argv)
{
  void *default_font = cos32_video_font_get("Default");
  if (!default_font)
  {
    return -1;
  }

  void *pixel_ptr = cos32_video_font_make_empty_string(default_font, strlen("COS32 Kernel"));
  if (!pixel_ptr)
  {
    return -1;
  }

  cos32_video_font_draw(default_font, pixel_ptr, "COS32 Kernel");

  void *taskbar_rect = cos32_video_rectangle_new(0, 0, 320, 20);
  cos32_video_rectangle_fill(taskbar_rect, 2);
  cos32_video_rectangle_draw_font_data(taskbar_rect, default_font, pixel_ptr, 120, 5, strlen("COS32 Kernel"));

  while (1)
  {
  }
  return 0;
}