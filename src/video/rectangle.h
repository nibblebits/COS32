#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <stdbool.h>

// Possibly put it in config.h but then we might overcrowd it all...
#define COS32_VIDEO_RECTANGLE_NAME_SIZE 64

#define VIDEO_RECTANGLE_FLAG_PUBLISHED 0b00000001

typedef unsigned char VIDEO_RECTANGLE_FLAGS;

struct video;
struct video_font;
struct video_rectangle
{
    // This should be equal to NULL if the video rectangle is private and not published.
    // Otherwise its the published accessible name.
    const char name[COS32_VIDEO_RECTANGLE_NAME_SIZE];

    int width;
    int height;
    int x;
    int y;
    // This boolean is set if the rectangle needs to be redrawn
    bool redraw;
    char* pixels;

    // Increment this if you share the rectangle with someone else
    // When it hits zero we will erase the memory
    int shared;

    VIDEO_RECTANGLE_FLAGS flags;

    struct properties
    {
        // Used to represent scaleing aspect when drawing 
        float scale;
    } properties;
};

void video_rectangle_register(struct video *video, struct video_rectangle *rect);
struct video_rectangle *video_rectangle_new(struct video *video, int x, int y, int width, int height);
void video_rectangles_free(struct video* video);
void video_rectangle_draw(struct video* video, struct video_rectangle *rect);
int video_rectangle_set_pixel(struct video_rectangle *rect, int x, int y, char colour);
char video_rectangle_get_pixel(struct video_rectangle *rect, int x, int y);
int video_rectangle_fill(struct video_rectangle* rect, int colour);
void video_rectangle_draw_block(struct video_rectangle *rect, void *ptr, int absx, int absy, int total_rows, int pixels_per_row);
void video_rectangle_draw_blocks(struct video_rectangle *rect, void *ptr, int absx, int absy, int total_rows, int pixels_per_row, int slen);
void video_rectangle_draw_font_data(struct video_rectangle* rect, struct video_font* font, void* ptr, int absx, int absy, int slen);
void video_rectangle_set_scale(struct video_rectangle* rect, float scale);


/**
 * 
 * Returns true if the rectangle with the given name has been published
 */
bool video_rectangle_is_published(const char* name);

/**
 * Globally publishes the rectangle allowing other processes to include it in their video
 */
int video_rectangle_publish(const char* name, struct video_rectangle* rect);

/**
 * Gets a published rectangle by name
 */
struct video_rectangle* video_rectangle_get(const char* name);

/**
 * Registers the default rectangles to the given video
 */
void video_rectangle_register_default_rectangles(struct video* video);

#endif