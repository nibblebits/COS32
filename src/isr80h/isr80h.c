#include "isr80h.h"
#include "isr80h/io.h"
#include "isr80h/process.h"
#include "isr80h/isrkernel.h"
#include "isr80h/video.h"
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
}