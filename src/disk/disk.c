#include "disk.h"
#include "memory/memory.h"
#include "kernel.h"
#include "status.h"
#include "config.h"
#include "io/io.h"
struct disk disk;

int disk_read_sector(int lba, int total, void *buf)
{
    outb(0x1F6, (lba >> 24) | 0xE0);
    outb(0x1F2, total);
    outb(0x1F3, (unsigned char)(lba & 0xff));
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20);


    unsigned short* ptr = (unsigned short*) buf;
    for (int b = 0; b < total; b++)
    {

        // Wait until buffer is ready
        char c = insb(0x1F7);
        while (!(c & 0x08))
        {
            c = insb(0x1F7);
        }

        // Copy from hard disk to memory two bytes at a time
        for (int i = 0; i < 256; i++)
        {
            *ptr = insw(0x1F0);
            ptr++;
        }
    }

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
    disk.id = 0;
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

    return disk_read_sector(lba, total, buf);
}