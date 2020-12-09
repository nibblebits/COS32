#include "elfloader.h"
#include "fs/file.h"
#include "status.h"
#include <stdbool.h>
#include "string/string.h"
#include "memory/memory.h"
#include "memory/kheap.h"
#include "loader/library.h"
#include "string/string.h"
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

bool elf_is_executable(struct elf_header *header)
{
    return header->e_type == ET_EXEC && header->e_entry >= COS32_PROGRAM_VIRTUAL_ADDRESS;
}

static bool elf_is_shared_library(struct elf_header *header)
{
    return header->e_type == ET_DYN;
}

static bool elf_has_program_header(struct elf_header *header)
{
    return header->e_phoff != 0;
}

void *elf_memory(struct elf_file *file)
{
    return file->elf_memory;
}

struct elf_header *elf_header(struct elf_file *file)
{
    // First byte in the elf memory is the start of the elf header
    return file->elf_memory;
}

struct elf32_shdr *elf_sheader(struct elf_header *header)
{
    return (struct elf32_shdr *)((int)header + header->e_shoff);
}

struct elf32_phdr *elf_pheader(struct elf_header *header)
{
    if (header->e_phoff == 0)
    {
        return 0;
    }

    return (struct elf32_phdr *)((int)header + header->e_phoff);
}

struct elf32_phdr *elf_program_header(struct elf_header *header, int index)
{
    return &elf_pheader(header)[index];
}
struct elf32_shdr *elf_section(struct elf_header *header, int index)
{
    return &elf_sheader(header)[index];
}

char *elf_str_table(struct elf_header *header)
{
    return (char *)header + elf_section(header, header->e_shstrndx)->sh_offset;
}

char *elf_string(struct elf_header *header, int index)
{
    char *string_table = elf_str_table(header);
    return &string_table[index];
}

struct elf32_shdr *elf_sheader_by_name(struct elf_header *header, const char *name)
{
    struct elf32_shdr *shdr = elf_sheader(header);
    for (int i = 0; i < header->e_shnum; i++)
    {
        if (strcmp(elf_string(header, shdr[i].sh_name), name) == 0)
        {
            return &shdr[i];
        }
    }

    return 0;
}

struct elf32_dyn *elf_dynamic(struct elf_header *header)
{
    struct elf32_shdr *dynamic_section = elf_sheader_by_name(header, ".dynamic");
    if (!dynamic_section)
    {
        return 0;
    }

    return (void *)header + dynamic_section->sh_offset;
}

int elf32_shdr_total_items(struct elf32_shdr *shdr)
{
    return shdr->sh_size / shdr->sh_entsize;
}

struct elf32_sym *elf_dynsym(struct elf_header *header, int *total_symbols)
{
    struct elf32_shdr *dynsym_shdr = elf_sheader_by_name(header, ".dynsym");
    if (!dynsym_shdr || dynsym_shdr->sh_type != SHT_DYNSYM)
    {
        return 0;
    }

    if (total_symbols)
    {
        *total_symbols = elf32_shdr_total_items(dynsym_shdr);
    }
    return (void *)header + dynsym_shdr->sh_offset;
}

struct elf32_shdr *elf_dynstr_section(struct elf_header *header)
{
    return elf_sheader_by_name(header, ".dynstr");
}

char *elf_dynstr_table(struct elf_header *header)
{
    struct elf32_shdr *dynstr = elf_dynstr_section(header);
    if (dynstr->sh_type != SHT_STRTAB)
    {
        return 0;
    }

    return (void *)header + dynstr->sh_offset;
}

char *elf_dynamic_string(struct elf_header *header, int index)
{
    char *table = elf_dynstr_table(header);
    if (!table)
    {
        return 0;
    }

    return table + index;
}

void *elf_virtual_base(struct elf_file *file)
{
    return file->virtual_base_address;
}

void *elf_virtual_end(struct elf_file *file)
{
    return file->virtual_end_address;
}

void *elf_phys_base(struct elf_file *file)
{
    return file->physical_base_address;
}

void *elf_phys_end(struct elf_file *file)
{
    return file->physical_end_address;
}

void *elf_virtual_to_phys(struct elf_file *file, void *virt_addr)
{
    return (virt_addr - file->virtual_base_address) + file->physical_base_address;
}

int elf_validate_loaded(struct elf_header *header)
{
    return (elf_valid_signature(header) && elf_valid_class(header) && elf_valid_encoding(header) && elf_has_program_header(header)) ? COS32_ALL_OK : -EINVARG;
}

int elf_process_phdr_pt_load(struct elf_file *elf_file, struct elf32_phdr *phdr)
{
    // If we are using PIC(Program independant code) which we will be if we are processing
    // a dyanmic link library. Then we need to recalculated the virtual address as its currently
    // just an offset

    if (elf_is_shared_library(elf_header(elf_file)))
    {
        // This is a shared library, lets add on the base address to the virtual address
        // resolving the virtual address of this PHDR.
        phdr->p_vaddr += (elf32_addr)elf_memory(elf_file);
    }

    // We want the lowest address we can find for the program header.
    // This will give us the virtual base address of this elf file.
    if (elf_file->virtual_base_address >= (void *)phdr->p_vaddr || elf_file->virtual_base_address == 0x00)
    {
        elf_file->virtual_base_address = (void *)phdr->p_vaddr;

        // We also want to calculate the physical base address here
        elf_file->physical_base_address = elf_memory(elf_file) + phdr->p_offset;
    }

    // We want to get the highest address we can find for the program header
    // This will allow us to calculate the end address of this elf file
    unsigned int end_virtual_address = phdr->p_vaddr + phdr->p_filesz;
    if (elf_file->virtual_end_address <= (void *)(end_virtual_address) || elf_file->virtual_end_address == 0x00)
    {
        elf_file->virtual_end_address = (void *)end_virtual_address;

        // We also want to set the physical end address
        elf_file->physical_end_address = elf_memory(elf_file) + phdr->p_offset + phdr->p_filesz;
    }

    return 0;
}

int elf_process_dynamic_tag_needed(struct elf_file *elf_file, struct elf32_dyn *dynamic_tag)
{
    const char *required_library_name = elf_dynamic_string(elf_header(elf_file), dynamic_tag->d_un.d_val);
    // Obviously later on we will change this to some abstracted library_load function...
    // For now this is the best we got.

    struct elf_file *library_file = 0x00;
    char path_buf[COS32_MAX_PATH];
    strncpy(path_buf, "0:/", sizeof(path_buf));
    strncpy(&path_buf[3], required_library_name, sizeof(path_buf));
    return elf_load(path_buf, &library_file);
}

int elf_process_phdr_pt_dynamic(struct elf_file *elf_file, struct elf32_phdr *phdr)
{
    int res = 0;
    struct elf32_dyn *dynamic_tag = elf_memory(elf_file) + phdr->p_offset;
    for (int i = 0;; i++)
    {
        // Have we reached the end of the dynamic tags?
        if (dynamic_tag[i].d_tag == DT_NULL)
        {
            break;
        }

        switch (dynamic_tag[i].d_tag)
        {
        case DT_NEEDED:
            res = elf_process_dynamic_tag_needed(elf_file, &dynamic_tag[i]);
            if (res < 0)
            {
                goto out;
            }
            break;
        }
    }

out:
    return res;
}

int elf_process_pheader(struct elf_file *elf_file, struct elf32_phdr *phdr)
{
    int res = 0;
    switch (phdr->p_type)
    {
    case PT_LOAD:
        res = elf_process_phdr_pt_load(elf_file, phdr);
        break;

    case PT_DYNAMIC:
        res = elf_process_phdr_pt_dynamic(elf_file, phdr);
        break;
    };

    return res;
}

int elf_process_pheaders(struct elf_file *elf_file)
{
    int res = 0;
    struct elf_header *header = elf_header(elf_file);
    for (int i = 0; i < header->e_phnum; i++)
    {
        struct elf32_phdr *phdr = elf_program_header(header, i);
        res = elf_process_pheader(elf_file, phdr);
        if (res < 0)
        {
            break;
        }
    }
    return res;
}

int elf_resolve_non_local_symbol(struct elf_file *elf_file, struct elf32_sym *symbol)
{
    //const char *symbol_name = elf_dynamic_string(elf_header(elf_file), symbol->st_name);
    // We don't really care about resolving anything here right now, we just care about
    // relocations at the moment.

    return 0;
}

int elf_export_symbol(struct elf_file *elf_file, struct elf32_sym *symbol)
{
    if (!elf_is_shared_library(elf_header(elf_file)))
    {
        // Looks like a non shared library has tried to export a symbol.
        // We should just ignore this... we are not a library.. but this is also not an error
        return 0;
    }

    void *abs_phys_addr = elf_file->physical_base_address + symbol->st_value;
    void *abs_virt_addr = elf_file->virtual_base_address + symbol->st_value;
    struct addr addr;
    addr.phys = abs_phys_addr;
    addr.virt = abs_virt_addr;
    return library_new_symbol(elf_file->library, elf_dynamic_string(elf_header(elf_file), symbol->st_name), &addr);
}

int elf_process_symbols(struct elf_file *elf_file)
{
    int res = 0;
    int total_symbols = 0;
    struct elf32_sym *symbols = elf_dynsym(elf_header(elf_file), &total_symbols);
    for (int i = 0; i < total_symbols; i++)
    {
        if (symbols[i].st_shndx == SHN_UNDEF)
        {
            res = elf_resolve_non_local_symbol(elf_file, &symbols[i]);
            if (res < 0)
            {
                break;
            }
        }
        else if (ELF32_R_TYPE(symbols[i].st_info) != STB_LOCAL)
        {
            // We are exporting a symbol here.
            res = elf_export_symbol(elf_file, &symbols[i]);
            if (res < 0)
            {
                break;
            }
        }
    }

    return res;
}

void *elf_get_loaded_symbol_address(struct elf_file *elf_file, const char *symbol_name)
{
    void *symbol_address = 0;
    // We must go through all the needed libraries and ask them for their symbols
    struct elf32_dyn *dynamic_tag = elf_dynamic(elf_header(elf_file));
    for (int i = 0;; i++)
    {
        // Have we reached the end of the dynamic tags?
        if (dynamic_tag[i].d_tag == DT_NULL)
        {
            break;
        }

        if (dynamic_tag[i].d_tag == DT_NEEDED)
        {
            // We have a needed tag, its value points to the library name thats required in the dynamic string
            // table
            const char *required_library_name = elf_dynamic_string(elf_header(elf_file), dynamic_tag[i].d_un.d_val);
            struct library *library = library_get(required_library_name);
            if (!library)
            {
                // We can't resolve a dependency as we are missing a loaded library that
                // this binary depends on.
                return 0;
            }

            struct symbol *symbol = library_get_symbol_by_name(library, symbol_name);
            if (symbol)
            {
                symbol_address = symbol->addr.virt;
                break;
            }
        }
    }
    return symbol_address;
}

int elf_symbol_poke_to_virtual_address(struct elf_file *elf_file, void *virtual_address, const char *symbol_name)
{
    uint32_t *phys_addr = elf_virtual_to_phys(elf_file, virtual_address);
    if (!phys_addr)
    {
        return -EIO;
    }

    *phys_addr = (uint32_t)elf_get_loaded_symbol_address(elf_file, symbol_name);

    return 0;
}
int elf_resolve_relocation(struct elf_file *elf_file, struct elf32_rel *relocation)
{
    int res = 0;
    struct elf_header *header = elf_header(elf_file);
    int relocation_type = ELF32_R_TYPE(relocation->r_info);

    // We support only R_386_JUMP_SLOT relocation types at this time.
    if (relocation_type != R_386_JUMP_SLOT)
    {
        return -EUNIMP;
    }

    int symbol_index = ELF32_R_SYM(relocation->r_info);
    struct elf32_sym *symbol = &elf_dynsym(header, 0)[symbol_index];
    const char *symbol_name = elf_dynamic_string(header, symbol->st_name);

    res = elf_symbol_poke_to_virtual_address(elf_file, (void *)relocation->r_offset, symbol_name);
    return res;
}
int elf_resolve_relocation_table(struct elf_file *elf_file, struct elf32_shdr *section)
{
    int res = 0;
    int total_relocations = elf32_shdr_total_items(section);
    struct elf32_rel *relocations = elf_memory(elf_file) + section->sh_offset;
    for (int i = 0; i < total_relocations; i++)
    {
        struct elf32_rel *relocation = &relocations[i];
        res = elf_resolve_relocation(elf_file, relocation);
        if (res < 0)
        {
            break;
        }
    }
    return res;
}
int elf_resolve_relocations(struct elf_file *elf_file)
{
    int res = 0;
    struct elf_header *header = elf_header(elf_file);
    struct elf32_shdr *sections = elf_sheader(header);
    for (int i = 0; i < header->e_shnum; i++)
    {
        struct elf32_shdr *section = &sections[i];
        if (section->sh_type == SHT_REL)
        {
            res = elf_resolve_relocation_table(elf_file, section);
            if (res < 0)
            {
                break;
            }
        }
    }

    return res;
}

int elf_process_header(struct elf_file *elf_file)
{
    struct elf_header *header = elf_header(elf_file);
    if (elf_is_shared_library(header))
    {
        // This is a shared library, we must register a library in our name
        elf_file->library = library_new(basename(elf_file->filename));
        if (!elf_file->library)
        {
            return -EIO;
        }
    }

    return 0;
}

int elf_library_process_section(struct elf_file *elf_file, struct elf32_shdr *shdr)
{
    /**
     * We want to create a new library section in the system for each section in the elf file.
     */
    struct elf_header *header = elf_header(elf_file);
    void *abs_phys_addr = elf_file->physical_base_address + shdr->sh_offset;
    void *abs_virt_addr = elf_file->virtual_base_address + shdr->sh_offset;
    struct addr addr;
    addr.phys = abs_phys_addr;
    addr.virt = abs_virt_addr;
    return library_new_section(elf_file->library, elf_string(header, shdr->sh_name), &addr, shdr->sh_size);
}

int elf_process_section(struct elf_file *elf_file, struct elf32_shdr *shdr)
{
    struct elf_header *header = elf_header(elf_file);
    if (!elf_is_shared_library(header))
    {
        // We don't care about the sections for non shared libraries at this time
        return 0;
    }

    return elf_library_process_section(elf_file, shdr);
}

int elf_process_sections(struct elf_file *elf_file)
{
    int res = 0;
    struct elf_header *header = elf_header(elf_file);
    struct elf32_shdr *shdr = elf_sheader(header);
    for (int i = 0; i < header->e_shnum; i++)
    {
        res = elf_process_section(elf_file, &shdr[i]);
        if (res < 0)
            break;
    }

    return res;
}
int elf_process_loaded(struct elf_file *elf_file)
{
    struct elf_header *header = elf_header(elf_file);
    int res = elf_validate_loaded(header);
    if (res < 0)
    {
        goto out;
    }

    res = elf_process_header(elf_file);
    if (res < 0)
    {
        goto out;
    }

    res = elf_process_pheaders(elf_file);
    if (res < 0)
    {
        goto out;
    }

    res = elf_process_sections(elf_file);
    if (res < 0)
    {
        goto out;
    }

    res = elf_process_symbols(elf_file);
    if (res < 0)
    {
        goto out;
    }

    res = elf_resolve_relocations(elf_file);
    if (res < 0)
    {
        goto out;
    }

out:
    return res;
}

int elf_load(const char *filename, struct elf_file **file_out)
{
    struct elf_file *elf_file = kzalloc(sizeof(struct elf_file));
    int fd = 0;
    int res = fopen(filename, "r");
    if (res <= 0)
    {
        goto out;
    }
    fd = res;

    struct file_stat stat;
    res = fstat(fd, &stat);
    if (res < 0)
    {
        goto out;
    }

    // First thing to do is copy the filename to the elf file
    strncpy(elf_file->filename, filename, sizeof(elf_file->filename));

    elf_file->elf_memory = kzalloc(stat.filesize);
    res = fread(elf_file->elf_memory, stat.filesize, 1, fd);
    if (res < 0)
    {
        goto out;
    }

    res = elf_process_loaded(elf_file);
    if (res < 0)
    {
        goto out;
    }

    *file_out = elf_file;
out:
    fclose(fd);
    return res;
}

int elf_close(struct elf_file *file)
{
    if (!file)
        return 0;

    kfree(file->elf_memory);

    return 0;
}