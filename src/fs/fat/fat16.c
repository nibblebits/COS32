#include "fat16.h"
#include "memory/memory.h"
#include "memory/kheap.h"
#include "string/string.h"
#include "kernel.h"
#include "status.h"
#include "config.h"
#include "disk/disk.h"
#include "kernel.h"
#include "config.h"

typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

struct fat_header_extended
{
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_header
{
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed));

struct fat_h
{
    struct fat_header primary_header;
    union fat_h_e {
        struct fat_header_extended extended_header;
    } shared;
};

struct fat_directory_item
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct fat_directory
{
    struct fat_directory_item *item;
    int total;
};

struct fat_item
{
    union {
        struct fat_directory_item *item;
        struct fat_directory *directory;
    };

    FAT_ITEM_TYPE type;
};

struct fat_private
{
    struct fat_h header;
    struct fat_directory root_directory;
};

struct filesystem fat16_fs;

int fat16_get_root_directory(struct disk *disk, struct fat_private *fat_private, struct fat_directory *directory);

int fat16_resolve(struct disk *disk)
{

    int res = 0;
    struct fat_private *fat_private = kmalloc(sizeof(struct fat_private));
    memset(fat_private, 0, sizeof(struct fat_private));
    if (disk_read_block(disk, 0, 1, &fat_private->header) != COS32_ALL_OK)
    {
        res = -EFSNOTUS;
        goto out;
    }

    if (fat_private->header.shared.extended_header.signature != 0x29)
    {
        res = -EFSNOTUS;
        goto out;
    }

    if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != COS32_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    disk->fs_private = fat_private;
    disk->filesystem = &fat16_fs;
out:

    if (res < 0)
    {
        kfree(fat_private);
        disk->fs_private = 0;
    }
    return res;
}

int fat16_get_root_directory(struct disk *disk, struct fat_private *fat_private, struct fat_directory *directory)
{
    int ret = 0;
    struct fat_header *primary_header = &fat_private->header.primary_header;
    int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_dir_entries = fat_private->header.primary_header.root_dir_entries;
    int root_dir_size = (root_dir_entries * sizeof(struct fat_directory_item));
    int total_sectors = root_dir_size / disk->sector_size;
    if (root_dir_size % disk->sector_size)
    {
        // Round up as it does not divide without a remainder
        total_sectors += 1;
    }

    // We should load the entire root directory into memory, this is only FAT it's not going to kill us
    struct fat_directory_item *dir = kmalloc(root_dir_size);
    if (!dir)
    {
        ret = -EMEM;
        goto out;
    }
    if (disk_read_block(disk, root_dir_sector_pos, 1, dir) != COS32_ALL_OK)
    {
        ret = -EIO;
        goto out;
    }

    directory->item = dir;
    directory->total = root_dir_entries;
out:
    return ret;
}

static int fat16_check_relative_path(const char *relative_path)
{
    int size = strnlen(relative_path, COS32_MAX_PATH);
    for (int i = 0; i < size; i++)
    {
        if (relative_path[i] == '/' || relative_path[i] == '\\')
        {
            return -EINVARG;
        }
    }

    return 0;
}

int fat16_search_for_file_in_directory_next(struct disk *disk, struct fat_directory *directory, const char *filename)
{

}

int fat16_search_for_file_in_directory(struct disk *disk, struct fat_directory *directory, const char *filename)
{
    // We can't handle path roots
    if (filename[0] == '/')
    {
        return -EINVARG;
    }

    // We need to keep looking its more than just root
    char *ptr = strtok(filename, '/');
    
}

int fat16_search_for_file(struct disk *disk, const char *filename, struct fat_item *item)
{
    int res = 0;
    struct fat_private *fat_private = disk->fs_private;
    char filename_copy[COS32_MAX_PATH];
    memset(filename_copy, 0, COS32_MAX_PATH);
    strncpy(filename_copy, filename, COS32_MAX_PATH);

    int filename_len = strnlen(filename_copy, COS32_MAX_PATH);
    if (filename_len == 1 && filename[0] == '/')
    {
        // User is only requesting root so just return the root directory
        item->directory = &fat_private->root_directory;
        item->type = FAT_ITEM_TYPE_DIRECTORY;
        goto out;
    }

    // We need to keep looking its more than just root. Index 1 to ignore "/" that was for root directory
    res = fat16_search_for_file_in_directory(disk, &filename[1], &fat_private->root_directory);
out:
    return res;
}

void *fat16_open(struct disk *disk, char *filename, char mode)
{
    // Read only filesystem
    if (mode != 'r')
    {
        return (void *)-ERDONLY;
    }

    struct fat_item item;
    fat16_search_for_file(disk, filename, &item);
    print("can do");
    return 0;
}

struct filesystem *fat16_init()
{
    fat16_fs.open = fat16_open;
    fat16_fs.resolve = fat16_resolve;
    strncpy(fat16_fs.name, "FAT16", sizeof(fat16_fs.name));
    return &fat16_fs;
}
