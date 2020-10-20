#include "rectangle.h"
#include "video.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "status.h"
#include <stdbool.h>

static bool video_rectangle_pixel_in_bounds(struct video_rectangle *rect, int x, int y)
{
    return !(x >= rect->width || y >= rect->height || x < 0 || y < 0);
}

static inline void video_rectangle_vga_320_200_plot_pixel(char *back_buffer, int x, int y, char colour)
{
    int index = (y * VIDEO_MODE_VGA_320_200_WIDTH) + x;
    back_buffer[index] = colour;
}

char video_rectangle_get_pixel(struct video_rectangle *rect, int x, int y)
{
    if (!video_rectangle_pixel_in_bounds(rect, x, y))
    {
        return -EINVARG;
    }
    int index = (y * rect->height) + x;
    return rect->pixels[index];
}

int video_rectangle_set_pixel(struct video_rectangle *rect, int x, int y, char colour)
{
    if (!video_rectangle_pixel_in_bounds(rect, x, y))
    {
        return -EINVARG;
    }

    int index = (y * rect->height) + x;
    rect->pixels[index] = colour;
    rect->redraw = true;

    return 0;
}

int video_rectangle_fill(struct video_rectangle *rect, int colour)
{
    int res = 0;
    for (int x = 0; x < rect->width; x++)
    {
        for (int y = 0; y < rect->height; y++)
        {
            res = video_rectangle_set_pixel(rect, x, y, colour);
            if (res < 0)
            {
                break;
            }
        }
    }

    return res;
}

void video_rectangle_draw(struct video_rectangle *rect)
{
    char *back_buffer = video_back_buffer();

    for (int x = 0; x < rect->width; x++)
    {
        for (int y = 0; y < rect->height; y++)
        {
            char pixel = video_rectangle_get_pixel(rect, x, y);
            video_rectangle_vga_320_200_plot_pixel(back_buffer, x + rect->x, y + rect->y, pixel);
        }
    }
    rect->redraw = false;
}

void video_rectangle_register(struct video *video, struct video_rectangle *rect)
{
    rect->shared++;
    if (video->rectangles == 0)
    {
        video->rectangles = rect;
        video->rectangle_last = rect;
        return;
    }

    video->rectangle_last->next = rect;
    video->rectangle_last = rect;
}

struct video_rectangle *video_rectangle_new(struct video *video, int x, int y, int width, int height)
{
    struct video_rectangle *rectangle = kzalloc(sizeof(struct video_rectangle));
    rectangle->x = x;
    rectangle->y = y;
    rectangle->width = width;
    rectangle->height = height;
    rectangle->next = 0;
    rectangle->pixels = kzalloc(width * height);

    video_rectangle_register(video, rectangle);
    return rectangle;
}

void video_rectangle_free(struct video_rectangle *rectangle)
{
    // We don't free anything if theirs more entities using this
    rectangle->shared--;
    if (rectangle->shared > 0)
    {
        return;
    }

    kfree(rectangle->pixels);
    kfree(rectangle);
}

void video_rectangles_free(struct video *video)
{
    struct video_rectangle *rect = video->rectangles;
    while (rect)
    {
        struct video_rectangle *tmp = rect->next;
        video_rectangle_free(rect);
        rect = tmp;
    }
}