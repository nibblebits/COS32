#include "disk.h"
#include "memory/memory.h"
#include "kernel.h"
#include "status.h"
#include "config.h"
struct disk disk;
void ata_lba_read(int lba, int total_sectors, void *addr);
int disk_read_sector(int lba, int total, void *buf)
{
    ata_lba_read(lba, total, buf);
    return 0;
}

/**
 * Finds all disks and initializes them
 */
void disk_search_and_init()
{
    // We only have one disk right now, lets just make it the primary disk
    // we wont bother checking if we even have a hard disk connected lets just hope we do
    // This abstraction exists so later we can make a clean implementation when we come to implement multiple disks and possibly virtual disks
    memset(&disk, 0, sizeof(disk));
    disk.type = COS32_DISK_TYPE_REAL;
    disk.sector_size = COS32_SECTOR_SIZE;
    disk.filesystem = fs_resolve(&disk);
}

struct disk *disk_get(int index)
{
    // We only have one disk implemented right now
    if (index != 0)
        return 0;

    return &disk;
}

int disk_read_block(struct disk *idisk, unsigned int lba, int total, void *buf)
{

    // We have only the possibility one disk at the moment so the pointer shouldnt differ from it
    if (idisk != &disk)
        return -EIO;


    ata_lba_read(lba, total, buf);
    return COS32_ALL_OK;
}