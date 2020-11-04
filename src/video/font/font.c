#include "font.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "string/string.h"

static struct video_font_list video_font_list;

/**
 * For now we will have a static default font.
 * We will not care to load this from a file at the moment.
 */
void video_font_init_default()
{
}

void video_font_init()
{
    memset(&video_font_list, 0, sizeof(video_font_list));
}

void video_font_register(struct video_font *font)
{
    if (video_font_list.head == 0)
    {
        video_font_list.head = font;
        video_font_list.last = font;
        return;
    }

    font->next = video_font_list.last;
    video_font_list.last = font;
}

struct video_font *video_font_load(const char *filename)
{
    // Not implemented!
    return 0;
}


void* video_font_make_empty_string(struct video_font* font, int len)
{
    return kzalloc(font->c_bytes * len);
}

void video_font_free_string(void* ptr)
{
    kfree(ptr);
}

// We don't support unicode yet...
void video_font_draw_character(struct video_font *font, void *out, char c)
{
    char* out_c = out;
    // OK we need to draw the character into the buffer. Let's see if it works
    int start_index = c * font->c_bytes;
    if (start_index >= font->datasize)
    {
        // We can't draw an invalid character thats not in this font!
        return;
    }
    
    const char *char_pixel_data = &font->data[start_index];
    for (int y = 0; y < font->c_height; y++)
    {
        *out_c = char_pixel_data[y];
        out_c++;
    }
}

void video_font_draw(struct video_font *font, void *out, const char *text)
{
    //const char* font_data = font->data;
    char* out_c = out;
    int len = strlen(text);
    for (int i = 0; i < len; i++)
    {
        video_font_draw_character(font, out_c, text[i]);
        out_c += font->c_bytes;
    }
}

struct video_font *video_font_new(const char *name, const char *data, int c_bytes, int max_characters, int width, int height)
{
    // Invalid argument!
    if (max_characters <= 0)
    {
        return 0;
    }

    // Currently we only support fonts with widths no greater than 8 bits
    // This is due to the way we output the pixels
    // A better implementation should be revised.
    if (width > 8)
    {
        return 0;
    }

    int data_buffer_size = c_bytes * max_characters;
    struct video_font *font = kzalloc(sizeof(struct video_font));
    strncpy((char *)font->font_name, name, sizeof(font->font_name));
    font->data = kzalloc(data_buffer_size);
    font->datasize = data_buffer_size;
    memcpy((void *)font->data, (void *)data, data_buffer_size);

    font->c_width = width;
    font->c_height = height;
    font->c_max = max_characters;
    font->c_bytes = c_bytes;

    font->next = 0x00;
    return font;
}

void video_font_free(struct video_font *font)
{
    kfree(font);
}

struct video_font *video_font_get(const char *font_name)
{
    struct video_font *current = video_font_list.head;
    while (current)
    {
        if (strncmp(current->font_name, font_name, sizeof(current->font_name)) == 0)
        {
            // We found the font we care about.
            return current;
        }
        current = current->next;
    }

    return 0;
}