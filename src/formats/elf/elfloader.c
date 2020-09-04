#include "elfloader.h"
#include "fs/file.h"
#include "status.h"
#include <stdbool.h>
#include "memory/memory.h"
#include "memory/kheap.h"
#include "memory/paging/paging.h"
#include "kernel.h"
#include "config.h"

// First byte apparently is for 0-3 EI_MAG's will assume I have understood correctly
const char elf_signature[] = {0x7f, 'E', 'L', 'F'};

static bool elf_valid_signature(void *buffer)
{
    return memcmp(buffer, (void *)elf_signature, sizeof(elf_signature)) == 0;
}

static bool elf_valid_class(struct elf_header *header)
{
    // We don't support 64 bit elfs yet....
    return header->e_ident[EI_CLASS] == ELFCLASSNONE || header->e_ident[EI_CLASS] == ELFCLASS32;
}

static bool elf_valid_encoding(struct elf_header *header)
{
    // MSB Is not supported yet
    return header->e_ident[EI_DATA] == ELFDATANONE || header->e_ident[EI_DATA] == ELFDATA2LSB;
}

static bool elf_is_executable(struct elf_header *header)
{
    return header->e_type == ET_EXEC && header->e_entry >= COS32_PROGRAM_VIRTUAL_ADDRESS;
}

static bool elf_has_program_header(struct elf_header *header)
{
    return header->e_phoff != 0;
}

static int elf_read_program_header(int fp, struct elf_header *elf_header, struct elf32_phdr *p_header, int index)
{
    int res = 0;
    res = fseek(fp, elf_header->e_phoff + (elf_header->e_phentsize * index), SEEK_SET);
    if (res < 0)
    {
        goto out;
    }

    if (fread(p_header, sizeof(struct elf32_phdr), 1, fp) != 1)
    {
        res = -EIO;
        goto out;
    }

    // We currently only support segments ready to be loaded into memory, no dynamic linking is yet available
    if (p_header->p_type != PT_LOAD && p_header->p_type != PT_NULL)
    {
        res = -EUNIMP;
        goto out;
    }
out:
    return res;
}

static int elf_load_section(int fp, struct elf32_phdr *phdr, struct elf_header *header, struct elf_loaded_section *section)
{
    int res = 0;
    // If the section is not aligned to a page then we can't load this elf, we expect alignment
    if (!paging_is_address_aligned((void *)phdr->p_paddr) || !paging_is_address_aligned((void *)phdr->p_vaddr))
    {
        res = -EINFORMAT;
        goto out;
    }

    void *data_ptr = 0;
    if (phdr->p_filesz == 0)
    {
        // We don't load sections with no data... needs a little cleaning... get a new function here
        goto set_section;
    }

    data_ptr = kzalloc(phdr->p_filesz);
    if (!data_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    if (fseek(fp, phdr->p_offset, SEEK_SET) < 0)
    {
        res = -EIO;
        goto out;
    }

    // I Know we read the entire thing at once, not the best idea but its easier to work with
    // may change this in the future although no performance losses will happen
    // in this kernel from us doing it this way
    if (fread(data_ptr, phdr->p_filesz, 1, fp) != 1)
    {
        res = -EIO;
        goto out;
    }


set_section:
    section->phys_addr = (void *)data_ptr;
    section->virt_addr = (void *)phdr->p_vaddr;
    // We end at the end of the page regardless of section size.
    section->phys_end = paging_align_address((void *)(data_ptr + phdr->p_filesz));
    section->flags = phdr->p_flags;

out:
    // If their was a problem clean the memory
    if (res < 0 && data_ptr != 0)
    {
        kfree(data_ptr);
    }

    return res;
}

static int elf_load_header(int fp, struct elf_header *header)
{
    int res = 0;
    memset(header, 0, sizeof(struct elf_header));
    if (fread((void *)header, sizeof(struct elf_header), 1, fp) != 1)
    {
        res = -EIO;
        goto out;
    }

    if (!elf_valid_signature(header))
    {
        res = -EINFORMAT;
        goto out;
    }

    if (!elf_valid_class(header))
    {
        res = -EINFORMAT;
        if (header->e_ident[EI_CLASS] == ELFCLASS64)
        {
            res = -EUNIMP;
        }
        goto out;
    }

    if (!elf_valid_encoding(header))
    {
        res = -EINFORMAT;
        if (header->e_ident[EI_DATA] == ELFDATA2MSB)
        {
            res = -EUNIMP;
        }

        goto out;
    }

    if (!elf_is_executable(header))
    {
        res = -EINFORMAT;
        goto out;
    }

    if (!elf_has_program_header(header))
    {
        res = -EINFORMAT;
        goto out;
    }

out:
    return res;
}

int elf_free_sections(struct elf_loaded_section *section)
{
    struct elf_loaded_section *current = section;
    while (current != 0)
    {
        struct elf_loaded_section *next = current->next;
        kfree(current);
        current = next;
    }

    return 0;
}

int elf_load_sections(int fp, struct elf_header *header, struct elf_file *elf_file)
{
    struct elf_loaded_section *section_head = kzalloc(sizeof(struct elf_loaded_section));
    struct elf_loaded_section *current_section = section_head;
    int res = 0;
    for (int i = 0; i < header->e_phnum; i++)
    {
        struct elf32_phdr elf_program_header;
        res = elf_read_program_header(fp, header, &elf_program_header, i);

        if (res < 0)
        {
            goto out;
        }
        
        res = elf_load_section(fp, &elf_program_header, header, current_section);
        if (res < 0)
        {
            goto out;
        }

        // I don't like this needs cleaning up, consider linked list functionality abstracted out
        if (i != header->e_phnum - 1)
        {
            struct elf_loaded_section *next_s = kzalloc(sizeof(struct elf_loaded_section));
            current_section->next = next_s;
            current_section = next_s;
        }
    }

    elf_file->section_h = section_head;
out:
    // We had an error so delete all the sections we loaded into memory...
    if (res < 0)
    {
        elf_free_sections(section_head);
    }

    return res;
}

void *elf_virtual_address(struct elf_loaded_section *section)
{
    return section->virt_addr;
}

void *elf_phys_address(struct elf_loaded_section *section)
{
    return section->phys_addr;
}

void *elf_phys_end_address(struct elf_loaded_section *section)
{
    return section->phys_end;
}

struct elf_loaded_section *elf_first_section(struct elf_file *file)
{
    return file->section_h;
}

struct elf_loaded_section *elf_next_section(struct elf_loaded_section *section)
{
    return section->next;
}

int elf_close(struct elf_file *file)
{
    // Ugly
    if (!file)
    {
        return 0;
    }

    int res = 0;
    struct elf_loaded_section *section = elf_first_section(file);
    while (section)
    {
        struct elf_loaded_section *nxt_section = elf_next_section(section);
        kfree(section->phys_addr);
        kfree(section);
        section = nxt_section;
    }
    return res;
}

int elf_load(const char *filename, struct elf_file **file_out)
{
    struct elf_file *elf_file = kzalloc(sizeof(struct elf_file));
    int res = fopen(filename, "r");
    if (res <= 0)
    {
        res = -EIO;
        goto out;
    }

    int fp = res;
    res = elf_load_header(fp, &elf_file->header);
    if (res < 0)
    {
        goto out;
    }

    res = elf_load_sections(fp, &elf_file->header, elf_file);
    if (res < 0)
    {
        goto out;
    }

    *file_out = elf_file;
out:
    fclose(fp);
    return res;
}