#include "file.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"
#include "fat/fat16.h"
#include "disk/disk.h"
#include "string/string.h"
#include "status.h"

struct filesystem *filesystems[COS32_MAX_FILESYSTEMS];

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

    filesystems[0] = filesystem;
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
    return len >= 3 && isdigit(filename[0]) && memcmp(&filename[1], ":/", 2) == 0;
}

static int fs_get_drive_by_path(char *filename)
{
    if (!fs_valid_path_format(filename))
    {
        return -EBADPATH;
    }

    return tonumericdigit(filename[0]);
}

int fopen(char *filename, char mode)
{
    int drive_no = fs_get_drive_by_path(filename);
    if (drive_no < 0)
    {
        return drive_no;
    }


    char *start_of_relative_path = &filename[2];

    struct disk *disk = disk_get(drive_no);
    if (!disk)
    {
        return -EINVARG;
    }

    if (!disk->filesystem)
    {
        return -EINVARG;
    }

    disk->filesystem->open(disk, start_of_relative_path, mode);

    return 0;
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