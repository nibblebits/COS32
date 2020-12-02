#include "isr80h.h"
#include "isr80h/io.h"
#include "isr80h/process.h"
#include "isr80h/isrkernel.h"
#include "isr80h/video.h"
#include "isr80h/font.h"
#include "idt/idt.h"

void isr80h_register_all()
{
    //   isr80h_register_command(SYSTEM_COMMAND_EXIT, isr80h_command0_exit);
    isr80h_register_command(SYSTEM_COMMAND_PRINT, isr80h_command1_print);
    isr80h_register_command(SYSTEM_COMMAND_GET_KEY, isr80h_command2_get_key);
    isr80h_register_command(SYSTEM_COMMAND_GET_KERNEL_INFO, isr80h_command3_get_kernel_info);
    isr80h_register_command(SYSTEM_COMMAND_PUTCHAR, isr80h_command4_putchar);
    isr80h_register_command(SYSTEM_COMMAND_MALLOC, isr80h_command5_malloc);
    isr80h_register_command(SYSTEM_COMMAND_INVOKE, isr80h_command6_invoke);
    isr80h_register_command(SYSTEM_COMMAND_SLEEP, isr80h_command7_sleep);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_RECTANGLE_NEW, isr80h_command8_video_rectangle_new);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_RECTANGLE_SET_PIXEL, isr80h_command9_video_rectangle_set_pixel);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_RECTANGLE_FILL, isr80h_command10_video_rectangle_fill);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_RECTANGLE_DRAW_BLOCK, isr80h_command11_video_rectangle_draw_block);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_RECTANGLE_DRAW_BLOCKS, isr80h_command12_video_rectangle_draw_blocks);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_FONT_GET, isr80h_command13_font_get);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_FONT_DRAW, isr80h_command14_font_draw);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_FONT_MAKE_EMPTY_STRING, isr80h_command_15_font_make_empty_string);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_RECTANGLE_DRAW_FONT_DATA, isr80h_command16_rectangle_draw_font_data);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_RECTANGLE_PUBLISH, isr80h_command17_rectangle_publish);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_RECTANGLE_GET, isr80h_command18_rectangle_get);
    isr80h_register_command(SYSTEM_COMMAND_PROCESS_GET_ARGUMENTS, isr80h_command19_process_get_arguments);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_BUFFER_FLUSH, isr80h_command20_video_buffer_flush);
    isr80h_register_command(SYSTEM_COMMAND_VIDEO_CLEAR_FLAG, isr80h_command21_video_clear_flag);
}