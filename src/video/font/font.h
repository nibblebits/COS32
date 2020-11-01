#ifndef FONT_H
#define FONT_H

#define VIDEO_FONT_MAX_CHARACTERS 256
#define VIDEO_FONT_NAME_SIZE 16

typedef unsigned short ASCII_CHARACTER;

struct video_font_list
{
    struct video_font* head;
    struct video_font* last;
};


struct video_font
{
    const char font_name[VIDEO_FONT_NAME_SIZE];
    
    // Font data is datasize in length!
    const char* data;
    int datasize;

    // The width and height for each character in this font.
    // One way to fix the problem when I don't know the default ;)
    int c_width;
    int c_height;
    int c_max;
    // Bytes per character
    int c_bytes;
    
    // The next video font in this list
    struct video_font* next;
};

void video_font_register(struct video_font* font);
struct video_font* video_font_load(const char* filename);
struct video_font *video_font_new(const char *name, const char *data, int c_bytes, int max_characters, int width, int height);
void video_font_draw_character(struct video_font* font, void* out, char c);
void video_font_draw(struct video_font *font, void *out, const char *text);
void video_font_free(struct video_font* font);

/**
 * Creates a pointer to some room to store a pixel string of "len" amount of bytes
 */
void* video_font_make_empty_string(struct video_font* font, int len);
/**
 * Releases the pixel data created by video_font_make_empty_string
 */
void video_font_free_string(void* ptr);

struct video_font* video_font_get(const char* font_name);

#endif