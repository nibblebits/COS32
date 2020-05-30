#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "disk/disk.h"
#include "memory/kheap.h"
#include "string/string.h"
#include "disk/disk.h"
#include "fs/fat/fat16.h"
#include "io/io.h"
#include "memory/idt/idt.h"
#include "kernel.h"
#include "memory/paging/paging.h"
#include "memory/memory.h"
#include "task/task.h"
#include "task/tss.h"
#include "gdt/gdt.h"
#include "config.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

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

size_t strlen(const char *str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t *terminal_buffer;

void terminal_initialize(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t *)0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c)
{
	if (c == '\n')
	{
		terminal_column = 0;
		terminal_row += 1;
		return;
	}

	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH)
	{
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

void terminal_write(const char *data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char *data)
{
	terminal_write(data, strlen(data));
}

static void print_digit(int digit)
{
	// We expect digits to be below ten
	if (digit > 9)
	{
		digit = 0;
	}

	terminal_putchar((char)(digit + 48));
}

void print_number(int number)
{
	int ten_thousands = 0;
	int thousands = 0;
	int hundreds = 0;
	int tens = 0;
	int units = 0;
	if (number <= 99)
	{
		tens = number / 10;
		units = number % 10;
		print_digit(tens);
		print_digit(units);
	}
	else if (number <= 999)
	{
		hundreds = number / 100;
		tens = number / 10 % 10;
		units = number % 10 % 10;
		print_digit(hundreds);
		print_digit(tens);
		print_digit(units);
	}
	else if (number <= 9999)
	{
		thousands = number / 1000;
		hundreds = number / 100 % 10;
		tens = number / 10 % 10;
		units = number % 10 % 10;
		print_digit(thousands);
		print_digit(hundreds);
		print_digit(tens);
		print_digit(units);
	}
	else if (number <= 99999)
	{
		ten_thousands = number / 10000;
		thousands = (number % 10000) / 1000;
		hundreds = (number % 1000) / 10;
		tens = number / 10 % 10;
		units = number % 10 % 10;
		print_digit(ten_thousands);
		print_digit(thousands);
		print_digit(hundreds);
		print_digit(tens);
		print_digit(units);
	}
}

char *itoa(int i)
{
	static char text[12];
	int loc = 11;
	text[11] = 0;
	char neg = 1;
	if (i >= 0)
	{
		neg = 0;
		i = -i;
	}
	while (i)
	{
		text[--loc] = '0' - (i % 10);
		i /= 10;
	}
	if (loc == 11)
		text[--loc] = '0';
	if (neg)
		text[--loc] = '-';
	return &text[loc];
}

void print(const char *message)
{
	terminal_writestring(message);
}

void panic(char *message)
{
	print(message);
	while (1)
	{
	}
}

void int_zero();
void testing()
{
	
		int80h();

	//int x = 0 / 0;
	while (1)
	{
	}
}

struct tss tss;

struct gdt gdt_real[COS32_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[COS32_TOTAL_GDT_SEGMENTS] = {
	{.base = 0x00, .limit = 0x00, .type = 0x00},				 // null segment
	{.base = 0x00, .limit = 0xffffffff, .type = 0x9a},			 // kernel code segment
	{.base = 0x00, .limit = 0xffffffff, .type = 0x92},			 // kernel data segment
	{.base = 0x00, .limit = 0xffffffff, .type = 0xf8},			 // user code segment
	{.base = 0x00, .limit = 0xffffffff, .type = 0xf2},			 // user data segment
	{.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9} // TSS segment

};

void int80h();
void kernel_main(void)
{
	/* Initialize terminal interface */
	terminal_initialize();

	memset(&gdt_real, 0, sizeof(gdt_real));
	gdt_structured_to_gdt(&gdt_real, gdt_structured, COS32_TOTAL_GDT_SEGMENTS);

	print("hello?");
	// Load our new GDT
	gdt_load(&gdt_real, sizeof(struct gdt) * COS32_TOTAL_GDT_SEGMENTS);


	// Initialize interrupts
	idt_init();


	// Setup TSS
	memset(&tss, 0, sizeof(tss));
	tss.ss0 = COS32_DATA_SELECTOR;
	tss.esp0 = 0x600000;




	tss_load(0x28);

	// Let's re-enable interrupts
	//enable_interrupts();

//	testing();
	
	user_mode_enter(testing);

		print("Hello worldssss\n");


	print("returned 1234\n");
	while (1)
	{
	}

	// Initialize the heap
	kheap_init();

	// Initialize paging
	struct paging_4gb_chunk *kernel_chunk = paging_new_4gb();
	char *ptr = kmalloc(4096);
	//memcpy(ptr, 0xB8000, 4096);

	//paging_map(kernel_chunk->directory_entry, 0x00, 0x00);

	//paging_switch(kernel_chunk->directory_entry);
	//enable_paging();
	print("testing");
	//user_mode_enter(testing);

	while (1)
	{
	}
	paging_map(kernel_chunk->directory_entry, 0xB8000, ptr);
	ptr = 0xB8000;
	*ptr = 'N';

	// Initialize filesystems
	fs_load();

	// Find the disks
	disk_search_and_init();

	if (fopen("0:/typs.H", 'r') > 0)
	{
		print("File opened\n");
	}
	else
	{
		print("Failed to open file\n");
	}

	print("Kernel initialized");
}