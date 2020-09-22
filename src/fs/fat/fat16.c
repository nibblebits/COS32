#include "fat16.h"
#include "memory/memory.h"
#include "memory/kheap.h"
#include "disk/streamer.h"
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

static uint32_t fat16_get_first_fat_sector(struct fat_private *private)
{
    return private->header.primary_header.reserved_sectors;
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

static int fat16_root_dir_sectors(struct fat_private *private)
{
    return ((private->header.primary_header.root_dir_entries * sizeof(struct fat_directory_item)) + (private->header.primary_header.bytes_per_sector - 1)) / private->header.primary_header.bytes_per_sector;
}

static int fat16_get_fat_entry(struct fat_private *private, int cluster)
{
    int res = -1;
    struct disk_stream *stream = diskstreamer_new(0);
    if (!stream)
    {
        goto out;
    }
    uint32_t fat_table_position = fat16_get_first_fat_sector(private) * COS32_SECTOR_SIZE;
    res = diskstreamer_seek(stream, fat_table_position + (cluster * COS32_FAT16_FAT_ENTRY_SIZE));
    if (res < 0)
    {
        goto out;
    }

    uint16_t result = 0;
    res = diskstreamer_read(stream, &result, sizeof(result));
    if (res < 0)
    {
        goto out;
    }
    res = result;
out:
    diskstreamer_close(stream);
    return res;
}

/**
 * Gets the correct cluster to use based on the starting cluster and the offset
 */
static int fat16_get_cluster_for_offset(struct fat_private *private, int starting_cluster, int offset)
{
    int res = 0;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * COS32_SECTOR_SIZE;
    int cluster_to_use = starting_cluster;
    int clusters_ahead = offset / size_of_cluster_bytes;

    for (int i = 0; i < clusters_ahead; i++)
    {
        int entry = fat16_get_fat_entry(private, cluster_to_use);
        if (entry == 0xFF8 || entry == 0xFFF)
        {
            // Last entry in file so we are out of bounds
            res = -EIO;
            goto out;
        }

        // Sector is marked as bad?
        if (entry == COS32_FAT16_BAD_SECTOR)
        {
            res = -EIO;
            goto out;
        }

        // Reserved sector?
        if (entry == 0xFF0 || entry == 0xFF6)
        {
            res = -EIO;
            goto out;
        }

        // Entry is free? I think our FAT is corrupted as we was trying to read further past here
        if (entry == 0)
        {
            res = -EIO;
            goto out;
        }


        // Ok we made it this far, the number in the fat entry should be the next cluster number
        cluster_to_use = entry;
    }

    res = cluster_to_use;
out:
    return res;
}

static int fat16_read_internal_from_stream(struct fat_private *private, struct disk_stream *stream, int cluster, int offset, int total, void *out)
{
    int res = 0;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * COS32_SECTOR_SIZE;
    int cluster_to_use = fat16_get_cluster_for_offset(private, cluster, offset);
    if (cluster_to_use < 0)
    {   
        res = cluster_to_use;
        goto out;
    }

    
    int offset_from_cluster = offset % size_of_cluster_bytes;

    int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
    int starting_pos = (starting_sector * COS32_SECTOR_SIZE) + offset_from_cluster;
    int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;
    res = diskstreamer_seek(stream, starting_pos);
    if (res != COS32_ALL_OK)
    {
        goto out;
    }

    res = diskstreamer_read(stream, out, total_to_read);
    if (res != COS32_ALL_OK)
    {
        goto out;
    }

    total -= total_to_read;
    if (total > 0)
    {
        // We have more to go lets get the next cluster from fat
        res = fat16_read_internal_from_stream(private, stream, cluster, offset + total_to_read, total, out + total_to_read);
    }

out:

    return res;
}

/*
 * Reads from the cluster, if reading overlaps then the next cluster is used
 */
static int fat16_read_internal(struct fat_private *private, int starting_cluster, int offset, int total, void *out)
{
    struct disk_stream *stream = diskstreamer_new(0);
    int res = fat16_read_internal_from_stream(private, stream, starting_cluster, offset, total, out);
    diskstreamer_close(stream);
    return res;
}

int fat16_read(struct disk *disk, void *private, uint32_t size, uint32_t nmemb, char *out_ptr)
{
    // This is the start of the read operation, offset is zero
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

    res = fat16_cluster_to_sector(fat_private, 3);

    char buf[50];
    fat16_read_internal(fat_private, 3, 0x115d4, 3, buf);
out:

    if (res < 0)
    {
        kfree(fat_private);
        disk->fs_private = 0;
    }
    return res;
}

int fat16_get_total_items_for_directory(struct disk *disk, uint32_t directory_start_sector)
{
    struct fat_directory_item item;
    struct fat_directory_item empty_item;
    memset(&empty_item, 0, sizeof(empty_item));
    int res = 0;
    int i = 0;
    int directory_start_pos = directory_start_sector * COS32_SECTOR_SIZE;

    struct disk_stream *stream = diskstreamer_new(0);
    diskstreamer_seek(stream, directory_start_pos);
    while (1)
    {
        if (diskstreamer_read(stream, &item, sizeof(item)) != COS32_ALL_OK)
        {
            res = -EIO;
            goto out;
        }

        // If we have a blank record then we are finished
        if (item.filename[0] == 0x00)
        {
            // We are done
            break;
        }

        // Item is unused lets skip it
        if (item.filename[0] == 0xE5)
        {
            continue;
        }

        i++;
    }
    res = i;
out:
    diskstreamer_close(stream);
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

    int total_items = fat16_get_total_items_for_directory(disk, root_dir_sector_pos);

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
    directory->total = total_items;
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
