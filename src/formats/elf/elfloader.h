#ifndef ELFLOADER_H
#define ELFLOADER_H

#include <stdint.h>
#include <stddef.h>

#include "elf.h"
#include "config.h"


struct elf_file
{
    char filename[COS32_MAX_PATH];

    int in_memory_size;

    // The physical memory address for the elf memory 
    // This is the entire ELF file loaded/mapped into memory
    void* elf_memory;


    /**
     * These memory pointers point to the code and data of this elf file
     */

    /**
     * The virtual base address of this binary
     */
    void* virtual_base_address;

    /**
     * The virtual end address of this binary
     */
    void* virtual_end_address;

    /**
     * The physical base address of this binary
     */
    void* physical_base_address;

    /**
     * The physical end address of this binary
     */
    void* physical_end_address;

};

/**
 * 
 * Returns zero on success, loads the elf file
 */
int elf_load(const char *filename, struct elf_file **file_out);


void* elf_memory(struct elf_file* file);

struct elf_header* elf_header(struct elf_file* file);
void *elf_virtual_base(struct elf_file *file);
void *elf_virtual_end(struct elf_file *file);
void* elf_phys_base(struct elf_file* file);
void* elf_phys_end(struct elf_file* file);

#endif