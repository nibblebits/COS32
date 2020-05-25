#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * bits 0-11
 * Avail, GS0ADWURP
 * 
 * Bits 11-31
 * Page table 4-Kb aligned address
 * 
 * More information: https://wiki.osdev.org/Paging#Page_Directory
 */
typedef uint32_t PAGE_DIRECTORY_ENTRY;

/**
 * bits 0-11
 * Avail, G0DACWURP
 * 
 * Bits 11-31
 * Physical Page Address
 * 
 * More information: https://wiki.osdev.org/Paging#Page_Directory
 */
typedef uint32_t PAGE_TABLE_ENTRY;

extern void paging_load_directory(PAGE_DIRECTORY_ENTRY*);
extern void enable_paging();

#endif