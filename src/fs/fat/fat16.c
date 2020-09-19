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
    union fat_h_e
    {
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
    int sector_pos;
    int ending_sector_pos;
};

struct fat_item
{
    union
    {
        struct fat_directory_item *item;
        struct fat_directory *directory;
    };

    FAT_ITEM_TYPE type;
};

struct fat_file_descriptor
{
    struct fat_item *item;
    uint32_t pos;
};

struct fat_private
{
    struct fat_h header;
    struct fat_directory root_directory;
};

int fat16_get_root_directory(struct disk *disk, struct fat_private *fat_private, struct fat_directory *directory);
int fat16_resolve(struct disk *disk);
void *fat16_open(struct disk *disk, char *filename, FILE_MODE mode);
int fat16_close(void *private);
int fat16_read(struct disk *disk, void *private, uint32_t size, uint32_t nmemb, char *out_ptr);
int fat16_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode);
int fat16_stat(struct disk *disk, void *private, struct file_stat *stat);

struct filesystem fat16_fs = {
    .open = fat16_open,
    .resolve = fat16_resolve,
    .close = fat16_close,
    .read = fat16_read,
    .seek = fat16_seek,
    .stat = fat16_stat};

static uint32_t fat16_get_first_fat_sector(struct disk *disk)
{
    struct fat_private *fat_private = disk->fs_private;
    return fat_private->header.primary_header.reserved_sectors;
}

static uint32_t fat16_get_cluster_size_in_bytes(struct disk *disk)
{
    struct fat_private *fat_private = disk->fs_private;
    return fat_private->header.primary_header.sectors_per_cluster * disk->sector_size;
}

static uint32_t fat16_get_first_cluster(struct fat_directory_item *item)
{
    return (item->high_16_bits_first_cluster << 16) | item->low_16_bits_first_cluster;
}

static void fat16_free_private(struct fat_file_descriptor *private)
{
    kfree(private->item);
    kfree(private);
}

int fat16_stat(struct disk *disk, void *private, struct file_stat *stat)
{
    int res = 0;
    struct fat_file_descriptor *descriptor = (struct fat_file_descriptor *)private;
    struct fat_item *desc_item = descriptor->item;
    // We only allow statting of files at the moment
    if (desc_item->type != FAT_ITEM_TYPE_FILE)
    {
        res = -EINVARG;
        goto out;
    }

    struct fat_directory_item *ritem = desc_item->item;
    stat->filesize = ritem->filesize;
    // We aren't bothered about flags right now
    stat->flags = 0;

out:
    return res;
}

int fat16_close(void *private)
{
    fat16_free_private((struct fat_file_descriptor *)private);
    return 0;
}

static int fat16_cluster_to_sector(struct fat_private *private, int cluster)
{
    return private->root_directory.ending_sector_pos + ((cluster - 2) * private->header.primary_header.sectors_per_cluster);
}

int fat16_read(struct disk *disk, void *private, uint32_t size, uint32_t nmemb, char *out_ptr)
{
    // This is the start of the read operation, offset is zero#
    return -EUNIMP;
}

int fat16_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode)
{
    int res = 0;
    struct fat_file_descriptor *desc = private;
    struct fat_item *desc_item = desc->item;
    if (desc_item->type != FAT_ITEM_TYPE_FILE)
    {
        // We allow seeking only on files, not directories
        res = -EINVARG;
        goto out;
    }

    // We can't allow people to seek past the file size, that will be bad!
    struct fat_directory_item *ritem = desc_item->item;
    if (offset >= ritem->filesize)
    {
        res = -EIO;
        goto out;
    }

    switch (seek_mode)
    {
    case SEEK_SET:
        desc->pos = offset;
        break;

    case SEEK_END:
        res = -EUNIMP;
        break;

    case SEEK_CUR:
        desc->pos += offset;
        break;

    default:
        res = -EINVARG;
        break;
    }

out:
    return res;
}

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

int fat16_get_total_items_for_directory(struct disk *disk, uint32_t directory_start_pos)
{
    struct fat_directory_item item;
    struct fat_directory_item empty_item;
    memset(&empty_item, 0, sizeof(empty_item));
    int res = 0;
    int i = 0;
    while(1)
    {
        if (disk_read_block(disk, directory_start_pos+i, 1, &item) != COS32_ALL_OK)
        {
            res = -EIO;
            goto out;
        }

        // If we have a blank record then we are finished
        if (memcmp(&item, &empty_item, sizeof(item) == 0))
        {
            break;
        }
        i++;
    }
    res = i;
out:
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

    fat16_get_total_items_for_directory(disk, root_dir_sector_pos);
    // We should load the entire root directory into memory, this is only FAT it's not going to kill us
    struct fat_directory_item *dir = kzalloc(root_dir_size);
    if (!dir)
    {
        ret = -ENOMEM;
        goto out;
    }
    if (disk_read_block(disk, root_dir_sector_pos, 1, dir) != COS32_ALL_OK)
    {
        ret = -EIO;
        goto out;
    }

    directory->item = dir;
    directory->total = root_dir_entries;
    directory->sector_pos = root_dir_sector_pos;
    directory->ending_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);
out:
    return ret;
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
    strncpy(&out[filename_len + 1], (char *)item->ext, strnlen_terminator((char *)item->ext, sizeof(item->ext), 0x20));
    // Add the null terminator
    int full_filename_len = strnlen(out, max_len);
    out[full_filename_len - 1] = 0x00;
}

void *fat16_open(struct disk *disk, char *filename, FILE_MODE mode)
{
    // Read only filesystem
    if (mode != FILE_MODE_READ)
    {
        return ERROR(-ERDONLY);
    }

    struct fat_file_descriptor *descriptor = 0;

    descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    //  descriptor->item = item;
    //descriptor->pos = 0;
out:
    return descriptor;
}

struct filesystem *fat16_init()
{
    strncpy(fat16_fs.name, "FAT16", sizeof(fat16_fs.name));
    return &fat16_fs;
}
