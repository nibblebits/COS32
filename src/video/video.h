#ifndef VIDEO_H
#define VIDEO_H

#include <stddef.h>
#include <stdint.h>
#include "rectangle.h"

#define VIDEO_MODE_VGA_320x200_WIDTH 320
#define VIDEO_MODE_VGA_320x200_HEIGHT 200
#define VIDEO_MODE_VGA_320x200_MEMORY_SIZE VIDEO_MODE_VGA_320x200_WIDTH * VIDEO_MODE_VGA_320x200_HEIGHT


#define VIDEO_FLAG_AUTO_FLUSH 0b00000001
#define VIDEO_FLAG_FLUSH 0b00000010

typedef unsigned char VIDEO_FLAGS;

/**
 * Consider to typedef the video pointers, void* is too generic,
 * either that or create a structure to represent it
 */


struct terminal_properties
{
    // The index in the data array for the current character
    int index;
    
    // The colour of the terminal
    uint8_t terminal_color;

    // ASCII Data for the terminal
    char* data;

    // The Y scroll position for this terminal
    int y_scroll;

    // The video associated with this terminal
    struct video* video;

};

struct video_rectangle_list_item
{
    struct video_rectangle* rectangle;
    struct video_rectangle_list_item* next;
};

struct video
{
    struct terminal_properties properties;
    // Pointer to video memory
    void* ptr;
    void* backbuffer;

    struct video_rectangle_list_item* rectangles;
    struct video_rectangle_list_item* rectangle_last;

    // The rectangle that all print operations should be drawn too
    struct video_rectangle* printing_rectangle;
    VIDEO_FLAGS flags;
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
 * Draws a block of pixels to the screen based on the total rows and pixels per row arguments provided
 */
void video_draw_block(struct video *video, void *ptr, int x, int y, int total_rows, int pixels_per_row);

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


char* video_back_buffer(struct video* video);
void video_back_buffer_clear(struct video* video);
void video_flush_back_buffer(struct video* video);
void video_draw(struct video *video);


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
void video_terminal_set_scroll(struct terminal_properties* properties, int scroll);

void kernel_terminal_initialize(void);
#endif