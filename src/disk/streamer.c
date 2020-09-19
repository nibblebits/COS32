#include "streamer.h"
#include "memory/kheap.h"
#include "config.h"

struct disk_stream* diskstreamer_new(int disk_id)
{
    struct disk* disk = disk_get(disk_id);
    if (!disk)
    {
        return 0;
    }    

    struct disk_stream* streamer = kzalloc(sizeof(struct disk_stream));
    streamer->pos = 0;
    streamer->disk = disk;
    return streamer;
}

int diskstreamer_seek(struct disk_stream* stream, int position)
{
    stream->pos = position;
    return 0;
}

int diskstreamer_read(struct disk_stream* stream, void* out, int total)
{
    int sector = stream->pos / COS32_SECTOR_SIZE;
    int offset = stream->pos % COS32_SECTOR_SIZE;
    char buf[COS32_SECTOR_SIZE];

    int res = disk_read_block(stream->disk, sector, 1, buf);
    if (res < 0)
    {
        goto out;
    }

    int total_to_read = total > COS32_SECTOR_SIZE ? COS32_SECTOR_SIZE : total;
    for (int i = 0; i < total_to_read; i++)
    {
        *(char*)out++ = buf[offset+i];
    }

    // Adjust stream
    stream->pos += total_to_read;
    // Do we have more to read? If the total is above the sector size we do
    if (total > COS32_SECTOR_SIZE)
    {
        // However we just read a sector! So we should minus it from the total to read
        res = diskstreamer_read(stream, out, total-COS32_SECTOR_SIZE);
    }
out:
    return res;
}

void diskstreamer_close(struct disk_stream* stream)
{
    kfree(stream);
}