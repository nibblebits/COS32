#include "rectangle.h"
#include "video.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "video/font/font.h"
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
    int index = (y * rect->width) + x;
    return rect->pixels[index];
}

int video_rectangle_set_pixel(struct video_rectangle *rect, int x, int y, char colour)
{
    if (!video_rectangle_pixel_in_bounds(rect, x, y))
    {
        return -EINVARG;
    }

    int index = (y * rect->width) + x;
    rect->pixels[index] = colour;
    rect->redraw = true;

    return 0;
}

void video_rectangle_draw_block(struct video_rectangle *rect, void *ptr, int absx, int absy, int total_rows, int pixels_per_row)
{
    char *c_ptr = ptr;
    int rx = 0;
    int ry = 0;
    for (int y = 0; y < total_rows; y++)
    {
        ry = absy + y;
        for (int i = 0; i < pixels_per_row; i++)
        {
            if ((*c_ptr << i) & 0b10000000)
            {
                // The pixel is set.
                rx = absx + i;
                video_rectangle_set_pixel(rect, rx, ry, 1);
            }
        }
        c_ptr++;
    }
}

void video_rectangle_draw_font_data(struct video_rectangle* rect, struct video_font* font, void* ptr, int absx, int absy, int slen)
{
    video_rectangle_draw_blocks(rect, ptr, absx, absy, font->c_height, font->c_width, slen);
}

void video_rectangle_draw_blocks(struct video_rectangle *rect, void *ptr, int absx, int absy, int total_rows, int pixels_per_row, int slen)
{
    int size_per_block = total_rows;
    for (int i = 0; i < slen; i++)
    {
        video_rectangle_draw_block(rect, ptr, absx, absy, total_rows, pixels_per_row);
        absx += pixels_per_row;
        ptr += size_per_block;
    }
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

void video_rectangle_set_scale(struct video_rectangle *rect, float scale)
{
    rect->properties.scale = scale;
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