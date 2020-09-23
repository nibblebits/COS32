#ifndef DISK_H
#define DISK_H

#include "fs/file.h"
typedef unsigned int COS32_DISK_TYPE;

// States this is a physical piece of hardware
#define COS32_DISK_TYPE_REAL 0

struct disk
{
    COS32_DISK_TYPE type;
    struct filesystem *filesystem;
    int id;
    int sector_size;

    // Private data for filesystem to manage, the filesystem can do what he wants with this pointer everyone else leave it alone
    void* fs_private;
};

/**
 * Finds all disks and initializes them
 */
void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_block(struct disk *idisk, unsigned int lba, int total, void *buf);
int disk_read_sector(int lba, int total, void *buf);
#endif