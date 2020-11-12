#ifndef COS32_H
#define COS32_H

#include <stdbool.h>

struct kernel_info
{
    unsigned int date;
    unsigned int build_no;
};

struct command_argument
{
    char argument[512];
    struct command_argument* next;
};

/*
 * Executes the given shell command, provided "max" should be equal to the total buffer size
 */
int cos32_run_command(const char *command, int max);
int cos32_invoke_command(struct command_argument* root_command);
void cos32_putchar(char c);
char cos32_getkey();
char cos32_getkeyblock();
void print(const char* message);
void kernel_information(struct kernel_info* info);
void* cos32_malloc(int size);

/**
 * Puts the task to sleep for the given number of miliseconds
 */
void cos32_sleep(int millis);

/**
 * Creates a new video rectangle for the process
 */
void* cos32_video_rectangle_new(int x, int y, int width, int height);


/**
 * Sets a pixel on the given rectangle
 */
void* cos32_video_rectangle_set_pixel(void* rect_ptr, int x, int y, char colour);

/**
 * Fills the given rectangle with a colour
 */

void* cos32_video_rectangle_fill(void* rect_ptr, char colour);

/**
 * Reads a line from the terminal, if you pass output_while_typing as true then
 * each individual character will be outputted to the terminal as they type.
 * We will return from this function upon a carriage return in which case the "out" buffer
 * will be populated with the entered string
 */
void cos32_terminal_readline(char* out, int max, bool output_while_typing);

/**
 * Draws a block of pixels onto the given rectangle
 */
void cos32_video_rectangle_draw_block(void *rect, void *ptr, int absx, int absy, int total_rows, int pixels_per_row);
/**
 * Draws a series of blocks of pixels onto the given rectangle
 */
void cos32_video_rectangle_draw_blocks(void *rect, void *ptr, int absx, int absy, int total_rows, int pixels_per_row, int total);

/**
 * Gets the font with the given name
 */
void* cos32_video_font_get(const char* font_name);

/**
 * Draws the pixel data into the provided "ptr" for the given font and message
 */
void cos32_video_font_draw(void* font, void* ptr, const char* message);

/**
 * Creates an empty string for the given font. The string will allow for "len" amount of characters
 * Pixel data is created to be able to store pixel data for a string of x "len" for the given "font"
 */
void* cos32_video_font_make_empty_string(void* font, int len);

/**
 * Draws the font data stored in "ptr" for the given font.
 * The font data "ptr" must be the pixel rendered buffer and not a pointer to a string
 * 
 * Call "cos32_video_font_make_empty_string" and "cos32_video_font_draw" before calling this function
 * 
 * Pass the pointer created with "cos32_video_font_make_empty_string" as the "ptr" argument.
 */
void* cos32_video_rectangle_draw_font_data(void *rect, void* font, void *ptr, int absx, int absy, int total);

#endif