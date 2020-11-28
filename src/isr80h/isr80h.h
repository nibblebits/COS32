#ifndef ISR80H_H
#define ISR80H_H

enum SystemCommands
{
    SYSTEM_COMMAND_EXIT,
    SYSTEM_COMMAND_PRINT,
    SYSTEM_COMMAND_GET_KEY,
    SYSTEM_COMMAND_GET_KERNEL_INFO,
    SYSTEM_COMMAND_PUTCHAR,
    SYSTEM_COMMAND_MALLOC,
    SYSTEM_COMMAND_INVOKE,
    SYSTEM_COMMAND_SLEEP,
    SYSTEM_COMMAND_VIDEO_RECTANGLE_NEW,
    SYSTEM_COMMAND_VIDEO_RECTANGLE_SET_PIXEL,
    SYSTEM_COMMAND_VIDEO_RECTANGLE_FILL,
    SYSTEM_COMMAND_VIDEO_RECTANGLE_DRAW_BLOCK,
    SYSTEM_COMMAND_VIDEO_RECTANGLE_DRAW_BLOCKS,
    SYSTEM_COMMAND_VIDEO_FONT_GET,
    SYSTEM_COMMAND_VIDEO_FONT_DRAW,
    SYSTEM_COMMAND_VIDEO_FONT_MAKE_EMPTY_STRING,
    SYSTEM_COMMAND_VIDEO_RECTANGLE_DRAW_FONT_DATA,
    SYSTEM_COMMAND_VIDEO_RECTANGLE_PUBLISH,
    SYSTEM_COMMAND_VIDEO_RECTANGLE_GET,
    SYSTEM_COMMAND_PROCESS_GET_ARGUMENTS
};


/**
 * Registers all the possible default kernel commands that can be called from user space
 */
void isr80h_register_all();

#endif