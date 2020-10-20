#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <stdbool.h>

struct video;
struct video_rectangle
{
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

    struct video_rectangle* next;
};

void video_rectangle_register(struct video *video, struct video_rectangle *rect);
struct video_rectangle *video_rectangle_new(struct video *video, int x, int y, int width, int height);
void video_rectangles_free(struct video* video);
void video_rectangle_draw(struct video_rectangle *rect);
int video_rectangle_set_pixel(struct video_rectangle *rect, int x, int y, char colour);
char video_rectangle_get_pixel(struct video_rectangle *rect, int x, int y);
int video_rectangle_fill(struct video_rectangle* rect, int colour);
#endif