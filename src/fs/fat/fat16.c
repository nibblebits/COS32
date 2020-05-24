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
    struct fat_private *fat_private = kzalloc(sizeof(struct fat_private));
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
    struct fat_directory_item *dir = kzalloc(root_dir_size);
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


/**
 * Converts the filename and extension of the provided item into a full string. Outputted in the "out" variable
 * \warning This function expects a maximum length of 13. We make you provide it to us to keep things clean and understandable
 */
void fat16_get_full_relative_filename(struct fat_directory_item *item, char *out, int max_len)
{
    memset(out, 0, max_len);
    strncpy(out, (char *)item->filename, sizeof(item->filename));
    int filename_len = strnlen_terminator(out, max_len, 0x20);
    // Add a decimal point to represent that file extension is coming next
    out[filename_len] = '.';
    strncpy(&out[filename_len + 1], (char *)item->ext, strnlen_terminator((char*)item->ext, sizeof(item->ext), 0x20));
    // Add the null terminator
    int full_filename_len = strnlen(out, max_len);
    out[full_filename_len - 1] = 0x00;
}

struct fat_item* fat16_search_for_file_in_directory(struct disk *disk, struct fat_directory *directory, const char *filename)
{
    ASSERT(disk);
    ASSERT(filename);
    ASSERT(directory);
    ASSERT(*filename != 0);

    struct fat_item* res = ERROR(-EIO);

    // We can't handle path roots
    if (filename[0] == '/')
    {
        res = ERROR(-EINVARG);
        goto out;
    }


    // We need to keep looking its more than just root
    for (int i = 0; i < directory->total; i++)
    {
        // +2 for decimal point and null terminator
        int max_len = sizeof(directory->item->filename) + sizeof(directory->item->ext) + 2;

        char combined_filename[max_len];
        fat16_get_full_relative_filename(&directory->item[i], combined_filename, max_len);

        if (istrncmp(combined_filename, filename, max_len) == 0)
        {   
            // Check for attribute for directory in the future rather than just return a file
            res = kzalloc(sizeof(struct fat_item));
            res->type = FAT_ITEM_TYPE_FILE;
            res->item = &directory->item[i];
            goto out;
            
        }

    }
out:
    return res;
}

struct fat_item* fat16_search_for_file(struct disk *disk, const char *filename)
{
    struct fat_item* res = 0;
    struct fat_private *fat_private = disk->fs_private;
    char filename_copy[COS32_MAX_PATH];
    memset(filename_copy, 0, sizeof(filename_copy));
    strncpy(filename_copy, filename, sizeof(filename_copy));

    int filename_len = strnlen(filename_copy, sizeof(filename_copy));
    if (filename_len == 1 && filename[0] == '/')
    {
        // User is only requesting root so just return the root directory
        struct fat_item* item = kzalloc(sizeof(struct fat_item));
        item->directory = &fat_private->root_directory;
        item->type = FAT_ITEM_TYPE_DIRECTORY;
        res = item;
        goto out;
    }

    // We need to keep looking its more than just root. Index 1 to ignore "/" that was for root directory
    res = fat16_search_for_file_in_directory(disk, &fat_private->root_directory, &filename[1]);
out:
    return res;
}

void *fat16_open(struct disk *disk, char *filename, char mode)
{
    // Read only filesystem
    if (mode != 'r')
    {
        return ERROR(-ERDONLY);
    }

    return fat16_search_for_file(disk, filename);
}

struct filesystem *fat16_init()
{
    fat16_fs.open = fat16_open;
    fat16_fs.resolve = fat16_resolve;
    strncpy(fat16_fs.name, "FAT16", sizeof(fat16_fs.name));
    return &fat16_fs;
}
