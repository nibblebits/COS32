/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)

OUTPUT_FORMAT(elf32-i386)
/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	. = 0x400000;

	.text : ALIGN(4096)
	{
		*(.text)
	}
	
	.asm : ALIGN(4096)
	{
		*(.asm)
	}

 

	/* Read-write data (initialized) */
	.data : ALIGN(4096)
	{
		*(.data)
		*(.rodata)
		*(COMMON)
		*(.bss)
	}
 
	
 
	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}
