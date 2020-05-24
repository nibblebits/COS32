#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "disk/disk.h"
#include "memory/kheap.h"
#include "string/string.h"
#include "disk/disk.h"
#include "fs/fat/fat16.h"
#include "io/io.h"
#include "kernel.h"

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
void terminal_initialize();

void kernel_main(void)
{
	/* Initialize terminal interface */
	terminal_initialize();

	print_number(istrncmp("Hello worldS", "Hello worlds", sizeof("Hello worlds")));

	// Initialize the heap
	kheap_init();

	// Initialize filesystems
	fs_load();

	// Find the disks
	disk_search_and_init();

	fopen("0:/test.txt",'r');

	print("Kernel initialized");


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
	else
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

