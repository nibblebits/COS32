#ifndef ELFLOADER_H
#define ELFLOADER_H

#include <stdint.h>
#include <stddef.h>

#include "elf.h"
#include "config.h"

struct elf_loaded_section
{
    void* phys_addr;
    void* virt_addr;
    void* phys_end;

    // The size of this loaded section
    size_t size;

    // Flags, e.g read, write, execute... memory protection
    elf32_word flags;

    // Consider storing section name at some point
    struct elf_loaded_section* next;
};

struct elf_file
{
    char filename[COS32_MAX_PATH];
    struct elf_header header;
    struct elf_loaded_section* section_h;
};

/**
 * 
 * Returns zero on success, we will change this to popualte some structure resource later on...
 */
int elf_load(const char* filename, struct elf_file** file_out);


/**
 * Closes the elf file
 */
int elf_close(struct elf_file* file);

/**
 * Returns the virtual address that the given elf section must be loaded at
 */
void* elf_virtual_address(struct elf_loaded_section* section);
/**
 * Returns the physical address in memory that the provided section is actually loaded at
 * \note This function does not return the physical address specified in the elf section on disk, it returns the allocated memory that we loaded the section into
 */
void* elf_phys_address(struct elf_loaded_section* section);


/**
 * Returns the physical address in memory for the end of this loaded section
 */
void* elf_phys_end_address(struct elf_loaded_section* section);

/**
 * Returns the first loaded section of the elf file
 */
struct elf_loaded_section* elf_first_section(struct elf_file* file);
/**
 * Returns the next loaded section thats next to the section provided
 */
struct elf_loaded_section* elf_next_section(struct elf_loaded_section* section);


#endif