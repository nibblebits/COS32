#ifndef VIDEO_H
#define VIDEO_H

#include <stddef.h>
#include <stdint.h>
#include "rectangle.h"

#define VIDEO_MODE_VGA_320_200_WIDTH 320
#define VIDEO_MODE_VGA_320_200_HEIGHT 200
#define VIDEO_MODE_VGA_320_200_MEMORY_SIZE VIDEO_MODE_VGA_320_200_WIDTH * VIDEO_MODE_VGA_320_200_HEIGHT

/**
 * Consider to typedef the video pointers, void* is too generic,
 * either that or create a structure to represent it
 */


struct terminal_properties
{
    // The row that the terminal will write to next
    size_t terminal_row;
    // the column that the terminal will write to next
    size_t terminal_col;

    // The colour of the terminal
    uint8_t terminal_color;

};


struct video
{
    struct terminal_properties properties;
    // Pointer to video memory
    void* ptr;

    struct video_rectangle* rectangles;
    struct video_rectangle* rectangle_last;

};


/**
 * Initializes video memory
 */
void video_init();

/**
 * Called by PIT timer frequently. Should work with the video drawing to screen if neccessary.
 */
void video_process(struct video* video);

/**
 * Creates new video memory and returns it, we are responsible for reeing with video_free
 */
struct video* video_new();
void video_free(struct video* video);

/**
 * Saves the current video memory into the provided pointer
 */
void video_save(struct video* video);


/**
 * Restores the provided video memory pointer back into screen memory
 */
void video_restore(struct video* video);

/**
 * Resets the cursor back to the top left of the terminal
 */
void video_reset_cursor();


char* video_back_buffer();
char* video_back_buffer_clear();
void video_flush_back_buffer();


/**
 * This is a print that prints directly to the terminal, its designed for use for outputs
 * relating to the kernel only
 */
void print(const char *message);

/**
 * Writes a string to the terminal using the terminal properties provided
 */
void video_terminal_writestring(struct terminal_properties* properties, const char *data);
void video_terminal_putchar(struct terminal_properties* properties, char c);

void kernel_terminal_initialize(void);
#endif