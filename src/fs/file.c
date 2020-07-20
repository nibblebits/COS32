#include "file.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"
#include "fat/fat16.h"
#include "disk/disk.h"
#include "string/string.h"
#include "status.h"
#include "memory/kheap.h"

struct filesystem *filesystems[COS32_MAX_FILESYSTEMS];
struct file_descriptor *file_descriptors[COS32_MAX_FILE_DESCRIPTORS];

static struct filesystem **fs_get_free_filesystem()
{
    int i = 0;
    for (i = 0; i < COS32_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] == 0)
        {
            return &filesystems[i];
        }
    }

    return 0;
}

void fs_insert_filesystem(struct filesystem *filesystem)
{
    struct filesystem **fs;
    if (filesystem == 0)
    {
        panic("NULL filesystem provided");
    }

    fs = fs_get_free_filesystem();

    if (!fs)
    {
        panic("No more filesystem slots available, failed to register filesystem");
    }

    *fs = filesystem;
}

/**
 * Loads statically compiled filesystems
 */
static void fs_static_load()
{
    fs_insert_filesystem(fat16_init());
}

void fs_load()
{
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

void fs_init()
{
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

/**
 * 
 * Tests the given filename to see if the path is a valid format
 * \warning This function does not test if the path exists or not
 * Valid paths
 * 0:/
 * 0:/testing/abc
 * 1:/abc/testing
 * 
 * Invalid paths
 * A:/abc
 * B:/
 */
static int fs_valid_path_format(char *filename)
{
    int len = strnlen(filename, COS32_MAX_PATH);
    return (len >= 3 && isdigit(filename[0])) && memcmp(&filename[1], ":/", 2) == 0;
}

static int fs_get_drive_by_path(char *filename)
{
    if (!fs_valid_path_format(filename))
    {
        return -EBADPATH;
    }

    return tonumericdigit(filename[0]);
}

static int file_new_descriptor(struct file_descriptor **desc_out)
{
    int res = -ENOMEM;
    for (int i = 0; i < COS32_MAX_FILE_DESCRIPTORS; i++)
    {
        if (file_descriptors[i] == 0)
        {
            struct file_descriptor *desc = kzalloc(sizeof(struct file_descriptor));
            // Descriptors start at index 1
            desc->index = i + 1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }

    return res;
}

static struct file_descriptor *file_get_descriptor(int fd)
{
    if (fd <= 0 || fd > COS32_MAX_FILE_DESCRIPTORS)
    {
        return 0;
    }

    // Descriptors start at index 1, so we must subtract one to get real array index
    int index = fd - 1;
    return file_descriptors[index];
}

int fread(void *ptr, uint32_t size, uint32_t nmemb, int fd)
{
    int res = 0;

    if (size == 0 || nmemb == 0 || fd < 1)
    {
        res = -EINVARG;
        goto out;
    }

    struct file_descriptor *desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EINVARG;
        goto out;
    }

    res = desc->filesystem->read(desc->disk, desc->private, size, nmemb, (char *)ptr);

out:
    return res;
}

FILE_MODE file_get_mode_by_string(const char *str)
{
    FILE_MODE mode = FILE_MODE_INVALID;
    if (strncmp(str, "r", 1) == 0)
    {
        mode = FILE_MODE_READ;
    }
    else if (strncmp(str, "w", 1) == 0)
    {
        mode = FILE_MODE_WRITE;
    }
    else if (strncmp(str, "a", 1) == 0)
    {
        mode = FILE_MODE_APPEND;
    }

    return mode;
}


int fopen(char *filename, const char *mode_str)
{
    int res = 0;
    int drive_no = fs_get_drive_by_path(filename);

    char path[COS32_MAX_PATH];
    memset(path, 0, sizeof(path));
    strncpy(path, filename, sizeof(path));
    char *start_of_relative_path = &path[2];

    struct disk *disk = disk_get(drive_no);
    if (!disk)
    {
        res = -EINVARG;
        goto out;
    }

    if (!disk->filesystem)
    {
        res = -EINVARG;
        goto out;
    }

    FILE_MODE mode = file_get_mode_by_string(mode_str);
    if (mode == FILE_MODE_INVALID)
    {
        res = -EINVARG;
        goto out;
    }

    void *private_data = disk->filesystem->open(disk, start_of_relative_path, mode);

    // Null returned? Seriously.
    if (private_data == 0)
    {
        res = -EIO;
        goto out;
    }

    if (ISERR(private_data))
    {
        res = ERROR_I(private_data);
        goto out;
    }

    struct file_descriptor *desc = 0;
    res = file_new_descriptor(&desc);
    if (res < 0)
    {
        goto out;
    }

    desc->filesystem = disk->filesystem;
    desc->private = private_data;
    desc->disk = disk;
    res = desc->index;
out:
    // fopen shouldnt return negative values.
    if (res < 0)
        res = 0;

    return res;
}

int fseek(int fd, int offset, FILE_SEEK_MODE whence)
{
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->seek(desc->private, offset, whence);
out:
    return res;
}

int fstat(int fd, struct file_stat* stat)
{
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }
    
    res = desc->filesystem->stat(desc->disk, desc->private, stat);
out:
    return res;
}
int fclose(int fd)
{
    int res = 0;
    struct file_descriptor *desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->close(desc->private);
out:
    return res;
}


struct filesystem *fs_resolve(struct disk *disk)
{
    struct filesystem *fs = 0;
    for (int i = 0; i < COS32_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0)
        {
            fs = filesystems[i];
            break;
        }
    }

    return fs;
}