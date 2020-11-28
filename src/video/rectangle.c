#include "rectangle.h"
#include "video.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "string/string.h"
#include "video/font/font.h"
#include "status.h"
#include "config.h"
#include <stdbool.h>

static struct video_rectangle *published_video_rectangles[COS32_VIDEO_RECTANGLES_MAX_PUBLISHABLE];

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

void video_rectangle_draw_font_data(struct video_rectangle *rect, struct video_font *font, void *ptr, int absx, int absy, int slen)
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

bool video_rectangle_is_published(const char *name)
{
    for (int i = 0; i < COS32_VIDEO_RECTANGLES_MAX_PUBLISHABLE; i++)
    {
        if (istrncmp(published_video_rectangles[i]->name, name, sizeof(published_video_rectangles[i]->name)) == 0)
        {
            return true;
        }
    }

    return false;
}

static int video_rectangle_find_publish_slot()
{
    for (int i = 0; i < COS32_VIDEO_RECTANGLES_MAX_PUBLISHABLE; i++)
    {
        if (published_video_rectangles[i] == 0)
            return i;
    }

    return -ENOMEM;
}

static int video_rectangle_publish_insert(struct video_rectangle *rect)
{
    int index = video_rectangle_find_publish_slot();
    if (index < 0)
        return index;

    published_video_rectangles[index] = rect;
    return 0;
}

int video_rectangle_publish(const char *name, struct video_rectangle *rect)
{
    if (!name)
    {
        return -EINVARG;
    }

    if (video_rectangle_is_published(name))
    {
        return -EISTKN;
    }

    // Copy the name to the rectangle.
    strncpy((char *)rect->name, name, sizeof(rect->name));

    // Set the published flag on this rectangle.
    rect->flags |= VIDEO_RECTANGLE_FLAG_PUBLISHED;

    return video_rectangle_publish_insert(rect);
}

struct video_rectangle *video_rectangle_get(const char *name)
{
    for (int i = 0; i < COS32_VIDEO_RECTANGLES_MAX_PUBLISHABLE; i++)
    {
        if (istrncmp(published_video_rectangles[i]->name, name, sizeof(published_video_rectangles[i]->name)) == 0)
        {
            return published_video_rectangles[i];
        }
    }

    return 0;
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

    struct video_rectangle_list_item *list_item = kzalloc(sizeof(struct video_rectangle_list_item));
    list_item->rectangle = rect;
    if (video->rectangles == 0)
    {
        video->rectangles = list_item;
        video->rectangle_last = list_item;
        return;
    }

    video->rectangle_last->next = list_item;
    video->rectangle_last = list_item;
}

struct video_rectangle *video_rectangle_new(struct video *video, int x, int y, int width, int height)
{
    struct video_rectangle *rectangle = kzalloc(sizeof(struct video_rectangle));
    rectangle->x = x;
    rectangle->y = y;
    rectangle->width = width;
    rectangle->height = height;
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
    struct video_rectangle_list_item *rect_list_item = video->rectangles;
    while (rect_list_item)
    {
        // It does not matter who created the rectangle as theirs a shared variable
        // It will not delete the memory until the shared variable hits zero
        // The memory can be shared safetly for the video_rectangle.
        struct video_rectangle_list_item *tmp = rect_list_item->next;
        video_rectangle_free(rect_list_item->rectangle);
        kfree(rect_list_item);
        rect_list_item = tmp;
    }
}

void video_rectangle_register_default_rectangles(struct video *video)
{
    // The taskbar is a default rectangle lets add it to the process video
    // This can be changed to some sort of list in the future allowing people
    // to make graphics that stick around ;)
    struct video_rectangle *taskbar_rect = video_rectangle_get("taskbar");
    if (!taskbar_rect)
    {
        return;
    }

    // Register the rectangle on this processes video!
    video_rectangle_register(video, taskbar_rect);

    // We should have a rectangle for the terminal for console programs
    // This will have to be enabled in the future, for now its default
    struct video_rectangle* printable_rectangle = video_rectangle_new(video, 0, taskbar_rect->height, 320, 200-taskbar_rect->height);
    video_rectangle_fill(printable_rectangle, 3);
    if (!printable_rectangle)
    {
        return;
    }

    // Any terminal operations to 0xB8000 will result in this rectangle being printed too.
    video->printing_rectangle = printable_rectangle;
}
