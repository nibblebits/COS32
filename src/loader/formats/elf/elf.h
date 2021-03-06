#ifndef ELFFORMAT_H
#define ELFFORMAT_H

#include <stdint.h>

#define PF_X 0x01
#define PF_W 0x02
#define PF_R 0x04

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6

#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2
#define STB_LOPROC 13
#define STB_HIPROC 15

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_LOPROC 12
#define SHT_HIPROC 13
#define SHT_LOUSER 14
#define SHT_HIUSER 15


#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4

#define EI_NIDENT 16
#define EI_CLASS 4
#define EI_DATA 5

#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2

#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

#define SHN_UNDEF 0

#define DT_NULL 0
#define DT_NEEDED 1
#define DT_RELA 7
#define DT_REL 17
#define DT_HASH 4

#define SHF_WRITE 0x01
#define SHF_ALLOC 0x02
#define SHF_EXECINSTR 0x04
#define SHF_MASKPROC 0xf0000000

#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4
#define STT_LOPROC 5
#define STT_HIPROC 6

#define R_386_32 1
#define R_386_GOT32 3
#define R_386_PLT32 4
#define R_386_COPY 5
#define R_386_GLOB_DAT 6
#define R_386_JUMP_SLOT 7
#define R_386_RELATIVE 8
#define R_386_GOTOFF 9
#define R_386_GOTPC 10

#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

typedef uint16_t elf32_half;
typedef uint32_t elf32_word;
typedef int32_t elf32_sword;
typedef uint32_t elf32_addr;
typedef uint32_t elf32_off;

struct elf32_phdr
{
    elf32_word p_type;
    elf32_off p_offset;
    elf32_addr p_vaddr;
    elf32_addr p_paddr;
    elf32_word p_filesz;
    elf32_word p_memsz;
    elf32_word p_flags;
    elf32_word p_align;
} __attribute__((packed));


struct elf32_shdr
{
    elf32_word	sh_name;
    elf32_word	sh_type;
	elf32_word	sh_flags;
	elf32_addr	sh_addr;
	elf32_off	sh_offset;
	elf32_word	sh_size;
	elf32_word	sh_link;
	elf32_word	sh_info;
	elf32_word	sh_addralign;
	elf32_word	sh_entsize;
} __attribute__((packed));

struct elf_header
{
    unsigned char e_ident[EI_NIDENT];
    elf32_half e_type;
    elf32_half e_machine;
    elf32_word e_version;
    elf32_addr e_entry;
    elf32_off e_phoff;
    elf32_off e_shoff;
    elf32_word e_flags;
    elf32_half e_ehsize;
    elf32_half e_phentsize;
    elf32_half e_phnum;
    elf32_half e_shentsize;
    elf32_half e_shnum;
    elf32_half e_shstrndx;
} __attribute__((packed));

struct elf32_dyn
{
    elf32_sword d_tag;
    union
    {
        elf32_word d_val;
        elf32_addr d_ptr;

    } d_un;
} __attribute__((packed));

struct elf32_sym
{
    elf32_word st_name;
    elf32_addr st_value;
    elf32_word st_size;
    unsigned char st_info;
    unsigned char st_other;
    elf32_half st_shndx;
} __attribute__((packed));

struct elf32_rel
{
    elf32_addr r_offset;
    elf32_addr r_info;
} __attribute__((packed));

struct elf32_rela
{
    elf32_addr r_offset;
    elf32_addr r_info;
    elf32_sword r_append;
} __attribute__((packed));




void * elf_get_entry_ptr(struct elf_header *elf_header);
uint32_t elf_get_entry(struct elf_header *elf_header);

#endif