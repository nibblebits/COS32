#include "video.h"
#include "memory/memory.h"
#include "memory/kheap.h"
#include "task/process.h"
#include "video/font/font.h"
#include "io/io.h"
#include "string/string.h"
#include "config.h"
#include "kernel.h"

#include <stdint.h>
#include <stddef.h>

void *video_default = 0;
static struct video_font *console_font = 0;

/* Hardware text mode color constants. */
enum vga_color
{
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t)uc | (uint16_t)color << 8;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

struct terminal_properties kernel_terminal_properties;

void terminal_putchar(struct terminal_properties *properties, char c);

void kernel_terminal_initialize(void)
{
	kernel_terminal_properties.terminal_row = 0;
	kernel_terminal_properties.terminal_col = 0;
	kernel_terminal_properties.video = 0;
	// Heap probably doesn't exist at this point in time so we cant allocae the data for the kernel yet
	kernel_terminal_properties.data = 0;
	kernel_terminal_properties.terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void terminal_putentryat(char *terminal_buffer, char c, size_t x, size_t y)
{
	if (!terminal_buffer)
	{
		return;
	}

	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = c;
}

void terminal_backspace(struct terminal_properties *properties)
{

	// We can't go back anymore
	if (properties->terminal_row == 0x00 && properties->terminal_col == 0x00)
		return;

	if (properties->terminal_col == 0x00)
	{
		properties->terminal_row -= 1;
		properties->terminal_col = VGA_WIDTH;
	}

	properties->terminal_col -= 1;
	terminal_putchar(properties, ' ');
	properties->terminal_col -= 1;
}
void terminal_putchar(struct terminal_properties *properties, char c)
{
	if (c == '\n')
	{
		properties->terminal_col = 0;
		properties->terminal_row += 1;
		return;
	}
	// Backspace
	if (c == 0x08)
	{
		terminal_backspace(properties);
		return;
	}

	/**
     * Notice we actually write directly to video memory and don't expect a videos buffer,
     * this was no accident, each task has the real video memory paged to point to its own video memory.
     * This makes more sense than setting the video memory of the video structure as we need to remember
     * that pixels can also be set manually and theirs no gaurantee a user will use our functions, they may
     * write to video memory directly. This is why paging is important. We can safetly write to the real video memory
     * because if the task has paged it will write to its own internal buffers instead
     */
	terminal_putentryat(properties->data, c, properties->terminal_col, properties->terminal_row);
	if (++properties->terminal_col == VGA_WIDTH)
	{
		properties->terminal_col = 0;
		if (++properties->terminal_row == VGA_HEIGHT)
			properties->terminal_row = 0;
	}
}

void terminal_write(struct terminal_properties *properties, const char *data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(properties, data[i]);
}

void video_terminal_putchar(struct terminal_properties *properties, char c)
{
	terminal_putchar(properties, c);
}

void video_terminal_writestring(struct terminal_properties *properties, const char *data)
{
	terminal_write(properties, data, strlen(data));
}

void print(const char *message)
{
	video_terminal_writestring(&kernel_terminal_properties, message);
}



/**
 * Pastes the terminal buffer to the rectangle after first clearing it
 */
static void video_terminal_paste(struct terminal_properties *video, struct video_rectangle *rectangle)
{
	video_rectangle_fill(rectangle, 3);

	int terminal_buf_len = VGA_WIDTH*VGA_HEIGHT;

	void* str_pixel_data = video_font_make_empty_string(console_font, terminal_buf_len);
	video_font_draw(console_font, str_pixel_data, video->data);
	video_rectangle_draw_font_data(rectangle, console_font, str_pixel_data, 0, 0, terminal_buf_len);
	video_font_free_string(str_pixel_data);
}

void video_draw(struct video *video)
{
	// Clear the back buffer
	video_back_buffer_clear();

	// Let's draw the terminal data to the printing rectangle
	if (video->printing_rectangle)
	{
		video_terminal_paste(&video->properties, video->printing_rectangle);
	}

	struct video_rectangle_list_item *list_item = video->rectangles;
	while (list_item != 0)
	{
		video_rectangle_draw(list_item->rectangle);
		list_item = list_item->next;
	}

	// Here we flush the back buffer
	// We copy every bit of video memory from the back buffer, this makes for a slow operation
	// In the future it might make more sense to copy only what was changed.
	video_flush_back_buffer();
}

void video_process(struct video *video)
{
	video_draw(video);
}

void video_init()
{
	kernel_terminal_initialize();

	video_default = kzalloc(COS32_VIDEO_MEMORY_SIZE);
	// Let's copy in the real video memory now so we have a default to work with
	memcpy(video_default, (void *)COS32_VIDEO_MEMORY_ADDRESS_START, COS32_VIDEO_MEMORY_SIZE);

	// Load all the fonts
	video_font_load_defaults();

	// Let's set the console fault
	console_font = video_font_get("Default");
	if (!console_font)
	{
		panic("No default font was found, we cannot continue!\n");
	}
}

char *video_back_buffer_clear()
{
	char *ptr = video_back_buffer();
	memset(ptr, 0, VIDEO_MODE_VGA_320_200_MEMORY_SIZE);
	return ptr;
}

char *video_back_buffer()
{
	static char *ptr = 0;
	if (ptr == 0)
	{
		ptr = kzalloc(VIDEO_MODE_VGA_320_200_MEMORY_SIZE);
	}

	return ptr;
}

void video_flush_back_buffer()
{
	char *video_ptr = (char *)0xA0000;
	char *back_buffer = video_back_buffer();
	memcpy(video_ptr, back_buffer, VIDEO_MODE_VGA_320_200_MEMORY_SIZE);
}

struct video *video_new()
{
	void *video_ptr = kmalloc(COS32_VIDEO_MEMORY_SIZE);
	memcpy(video_ptr, video_default, COS32_VIDEO_MEMORY_SIZE);

	struct video *video = kzalloc(sizeof(struct video));
	video->properties.terminal_row = 0;
	video->properties.terminal_col = 0;
	video->properties.terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	video->properties.video = video;
	video->properties.data = kzalloc(VGA_WIDTH*VGA_HEIGHT);
	memset(video->properties.data, ' ', VGA_WIDTH*VGA_HEIGHT);
	video->ptr = video_ptr;

	return video;
}

void video_free(struct video *video)
{
	// Free all rectangles relating to this video
	video_rectangles_free(video);

	kfree(video->ptr);
	kfree(video);
}

/**
 * Saves the current video  memory state into the provided video pointer
 * so that it can be restored at a later time
 */
void video_save(struct video *video)
{
	memcpy(video->ptr, (void *)COS32_VIDEO_MEMORY_ADDRESS_START, COS32_VIDEO_MEMORY_SIZE);
}

void video_restore(struct video *video)
{
	memcpy((void *)COS32_VIDEO_MEMORY_ADDRESS_START, video->ptr, COS32_VIDEO_MEMORY_SIZE);
}