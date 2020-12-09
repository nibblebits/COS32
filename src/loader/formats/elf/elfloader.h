#ifndef ELFLOADER_H
#define ELFLOADER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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

    /**
     * This contains a pointer to the library  instance that represents this elf file.
     * If this elf file is a library. If its an executable then this variable is NULL
     */
    struct library* library;

};

/**
 * 
 * Returns zero on success, loads the elf file
 */
int elf_load(const char *filename, struct elf_file **file_out);

/**
 * Closes this ELF file
 */
int elf_close(struct elf_file* file);


bool elf_is_executable(struct elf_header *header);

void* elf_memory(struct elf_file* file);
struct elf_header* elf_header(struct elf_file* file);
void *elf_virtual_base(struct elf_file *file);
void *elf_virtual_end(struct elf_file *file);
void* elf_phys_base(struct elf_file* file);
void* elf_phys_end(struct elf_file* file);
struct elf32_shdr* elf_sheader_by_name(struct elf_header* header, const char* name);
struct elf32_sym* elf_dynsym(struct elf_header* header, int* total_symbols);
struct elf32_shdr* elf_dynstr_section(struct elf_header* header);
char* elf_dynstr_table(struct elf_header* header);
char* elf_dynamic_string(struct elf_header* header, int index);


#endif