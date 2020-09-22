#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "disk/disk.h"
#include "memory/kheap.h"
#include "string/string.h"
#include "fs/pparser.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "fs/fat/fat16.h"
#include "io/io.h"
#include "idt/idt.h"
#include "kernel.h"
#include "memory/paging/paging.h"
#include "memory/memory.h"
#include "memory/registers.h"
#include "video/video.h"
#include "keyboard/keyboard.h"
#include "keyboard/listener.h"
#include "formats/elf/elfloader.h"
#include "task/task.h"
#include "task/tss.h"
#include "task/process.h"
#include "gdt/gdt.h"
#include "config.h"
void kernel_registers();

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif




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


void panic(char *message)
{
	print(message);
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

struct paging_4gb_chunk *kernel_paging_chunk = 0;

uint32_t* kernel_get_page_directory()
{
	return kernel_paging_chunk->directory_entry;
}

void kernel_page()
{
	kernel_registers();
	paging_switch(kernel_paging_chunk->directory_entry);
}

bool is_kernel_page()
{
	return paging_current_directory() == kernel_get_page_directory();
}

void kernel_main(void)
{
	/* Initialize terminal interface */
	kernel_terminal_initialize();

	memset(gdt_real, 0, sizeof(gdt_real));
	gdt_structured_to_gdt(gdt_real, gdt_structured, COS32_TOTAL_GDT_SEGMENTS);

	// Load our new GDT
	gdt_load(gdt_real, sizeof(struct gdt) * COS32_TOTAL_GDT_SEGMENTS);

	// Initialize interrupts
	idt_init();

	// Setup TSS
	memset(&tss, 0, sizeof(tss));
	tss.ss0 = COS32_DATA_SELECTOR;
	tss.esp0 = 0x600000;

	// Load the TSS
	tss_load(0x28);

	// Initialize the heap
	kheap_init();

	// Initialize the video memory
	video_init();

	// Initialize all the keyboards
	keyboard_init();

	// Initialize all the default keyboard listeners
	keyboard_listener_init();

	// Initialize filesystems
	fs_init();

	// Find the disks
	disk_search_and_init();

	// Enable IDT
	idt_load_now();

	// Initialize paging
	kernel_paging_chunk = paging_new_4gb(PAGING_ACCESS_FROM_ALL | PAGING_PAGE_PRESENT | PAGING_CACHE_DISABLED | PAGING_PAGE_WRITEABLE);
	kernel_page();
	enable_paging();


	print("Kernel initialized\n");

	// Load the start program
	int res = process_load_start("0:/start.e", 0, 0);
	if (res < 0)
	{
		panic("Failed to load the start program!\n");
	}

	// Usually the interrupt handler will call this, but as we have started the very first process
	// we are responsible
	
	task_run_first_ever_task();
	
}