/**
 * This file is written very badly consider improving the design
 */

#include "cos32.h"
#include "string.h"
#include <stddef.h>

void *default_font = 0;
void *taskbar_rect = 0;
void *info_rect = 0;
void *info_text_use_function_keys_ptr = 0;
int info_text_use_function_keys_ptr_len = 0;

int c = 0;
int initialize()
{

  // We don't want the video buffer getting flushed we want to do it ourselves.
  cos32_video_clear_flag(VIDEO_FLAG_AUTO_FLUSH);

  default_font = cos32_video_font_get("Default");
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

  taskbar_rect = cos32_video_rectangle_new(0, 0, 320, 20);
  cos32_video_rectangle_fill(taskbar_rect, 29);
  cos32_video_rectangle_draw_font_data(taskbar_rect, default_font, pixel_ptr, 120, 5, strlen("COS32 Kernel"));

  // Publish the rectangle so its accessible from other processes
  cos32_video_rectangle_publish(taskbar_rect, "taskbar");

  const char *info_text = "Press F1-F12 keys to start a shell";
  info_text_use_function_keys_ptr_len = strlen(info_text);
  info_text_use_function_keys_ptr = cos32_video_font_make_empty_string(default_font, info_text_use_function_keys_ptr_len);
  cos32_video_font_draw(default_font, info_text_use_function_keys_ptr, info_text);
  info_rect = cos32_video_rectangle_new(20, 100, 220, 20);
  return 0;
}

void info_rect_update()
{
  cos32_video_rectangle_fill(info_rect, 28);
  cos32_video_rectangle_draw_font_data(info_rect, default_font, info_text_use_function_keys_ptr, 10, 1, info_text_use_function_keys_ptr_len);
  cos32_sleep(3000);
}

void update()
{
  info_rect_update();
}


void redraw()
{
    cos32_flush_video_buffer();
}

int main(int argc, char **argv)
{
  initialize();
  while (1)
  {
  update();
    redraw();
    
  }
  return 0;
}