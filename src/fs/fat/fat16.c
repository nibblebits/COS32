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
    int sector_pos;
    int ending_sector_pos;
};

struct fat_item
{
    union {
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
int fat16_stat(struct disk *disk, void *private, struct file_stat* stat);


struct filesystem fat16_fs = {
    .open = fat16_open,
    .resolve = fat16_resolve,
    .close = fat16_close,
    .read = fat16_read,
    .seek = fat16_seek,
    .stat = fat16_stat
    };

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

static int fat16_get_next_cluster_for_item(uint32_t item_last_cluster_abs, struct disk *disk)
{
    int res = 0;
    uint32_t first_fat_sector = fat16_get_first_fat_sector(disk);
    // FAT Likes us to use an offset of "2" when working with clusters, very strange...
    uint32_t relative_next_cluster_in_fat = item_last_cluster_abs * 2;
    uint32_t relative_fat_sector = relative_next_cluster_in_fat / disk->sector_size;
    uint32_t relative_fat_offset = relative_next_cluster_in_fat % disk->sector_size;

    uint32_t abs_fat_sector = first_fat_sector + relative_fat_sector;

    // Ok let's load in the sector and find the next cluster
    char *buf = (char *)kzalloc(disk->sector_size);
    if (disk_read_block(disk, abs_fat_sector, 1, buf) != COS32_ALL_OK)
    {
        goto out;
    }

    res = *(uint16_t *)(&buf[relative_fat_offset]);

out:
    return res;
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


int fat16_stat(struct disk *disk, void *private, struct file_stat* stat)
{
    int res = 0;
    struct fat_file_descriptor *descriptor = (struct fat_file_descriptor *)private;
    struct fat_item* desc_item = descriptor->item;
    // We only allow statting of files at the moment
    if (desc_item->type != FAT_ITEM_TYPE_FILE)
    {
        res = -EINVARG;
        goto out;
    }

    struct fat_directory_item* ritem = desc_item->item;
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

int fat16_read_next(struct disk *disk, void *private, uint32_t size, uint32_t nmemb, char *out_ptr)
{
    int res = 0;
    struct fat_file_descriptor *descriptor = (struct fat_file_descriptor *)private;
    struct fat_item *desc_item = descriptor->item;
    struct fat_private *fat_private = disk->fs_private;
    // +1 is required as size does not have to be sector_size aligned
    uint32_t total_sectors_to_read = (size / disk->sector_size) + 1;

    // We don't allow you to cross the size boundary into multiple clusters, right now
    if (size >= fat16_get_cluster_size_in_bytes(disk))
    {
        res = -EINVARG;
        goto out;
    }

    // Possibly move kzalloc in the function that calls this function, the size is the same across the board, this is a waste of cycles
    char *buf = kzalloc(total_sectors_to_read * disk->sector_size);

    if (desc_item->type != FAT_ITEM_TYPE_FILE)
    {
        // We don't allow fread on directories
        res = -EIO;
        goto out;
    }

    struct fat_directory_item *item = desc_item->item;
    #warning currently its not possible to read past the 1st cluster, this needs to be implemented at some point
    if (descriptor->pos >= fat16_get_cluster_size_in_bytes(disk))
    {
        res = -EUNIMP;
        goto out;
    }

    if (descriptor->pos + size > item->filesize)
    {
        // We can't read another block we have gone too far! Zero blocks read for this iteration
        res = 0;
        goto out;
    }
    
    uint32_t first_cluster_abs = fat16_get_first_cluster(item);
    uint32_t file_first_sector = fat16_cluster_to_sector(fat_private, first_cluster_abs);

    // pos contains current stream position
    uint32_t offset_bytes = descriptor->pos;
    uint32_t offset_sector = offset_bytes / disk->sector_size;
    uint32_t offset_relative_remainder = offset_bytes % disk->sector_size;

    uint32_t abs_file_sector = file_first_sector + offset_sector;
    if (disk_read_block(disk, abs_file_sector, total_sectors_to_read, buf) != COS32_ALL_OK)
    {
        goto out;
    }

    // Let's now push the remainder to memory
    for (int i = 0; i < size; i++)
    {
        out_ptr[i] = buf[i + offset_relative_remainder];
    }
    // One block read
    res = 1;

    // Let's adjust the file position
    descriptor->pos += size;

    // Do we have any more to read lets use some recursion?
    if (nmemb > 1)
    {
        out_ptr += size;
        // Adjust current total blocks read (hopefully will work?)
        res += fat16_read_next(disk, private, size, nmemb - 1, out_ptr);
    }

out:
    kfree(buf);
    return res;
}
int fat16_read(struct disk *disk, void *private, uint32_t size, uint32_t nmemb, char *out_ptr)
{
    // This is the start of the read operation, offset is zero
    return fat16_read_next(disk, private, size, nmemb, out_ptr);
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
    directory->sector_pos = root_dir_sector_pos;
    directory->ending_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);
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
    strncpy(&out[filename_len + 1], (char *)item->ext, strnlen_terminator((char *)item->ext, sizeof(item->ext), 0x20));
    // Add the null terminator
    int full_filename_len = strnlen(out, max_len);
    out[full_filename_len - 1] = 0x00;
}

struct fat_item *fat16_search_for_file_in_directory(struct disk *disk, struct fat_directory *directory, const char *filename)
{
    ASSERT(disk);
    ASSERT(filename);
    ASSERT(directory);
    ASSERT(*filename != 0);

    struct fat_item *res = ERROR(-EIO);

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

struct fat_item *fat16_search_for_file(struct disk *disk, const char *filename)
{
    struct fat_item *res = 0;
    struct fat_private *fat_private = disk->fs_private;
    char filename_copy[COS32_MAX_PATH];
    memset(filename_copy, 0, sizeof(filename_copy));
    strncpy(filename_copy, filename, sizeof(filename_copy));

#warning "Move this parsing stuff to a different part of the system, abstract it out"
    int filename_len = strnlen(filename_copy, sizeof(filename_copy));
    if (filename_len == 1 && filename[0] == '/')
    {
        // User is only requesting root so just return the root directory
        struct fat_item *item = kzalloc(sizeof(struct fat_item));
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

void *fat16_open(struct disk *disk, char *filename, FILE_MODE mode)
{
    // Read only filesystem
    if (mode != FILE_MODE_READ)
    {
        return ERROR(-ERDONLY);
    }

    struct fat_file_descriptor *descriptor = 0;
    struct fat_item *item = fat16_search_for_file(disk, filename);
    if (item == 0)
    {
        goto out;
    }

    descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    descriptor->item = item;
    descriptor->pos = 0;
out:
    return descriptor;
}

struct filesystem *fat16_init()
{
    strncpy(fat16_fs.name, "FAT16", sizeof(fat16_fs.name));
    return &fat16_fs;
}
