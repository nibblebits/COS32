#include "video.h"
#include "memory/memory.h"
#include "memory/kheap.h"
#include "io/io.h"
#include "config.h"
void* video_default = 0;


void video_init()
{
    video_default = kmalloc(COS32_VIDEO_MEMORY_SIZE);
    // Let's copy in the real video memory now so we have a default to work with
    memcpy(video_default, (void*) COS32_VIDEO_MEMORY_ADDRESS_START, COS32_VIDEO_MEMORY_SIZE);
}

void* video_new()
{
    // When creating new memory we will use the default video memory at the time of initializes
    // the video
    void* video_ptr = kmalloc(COS32_VIDEO_MEMORY_SIZE);
    memcpy(video_ptr, video_default, COS32_VIDEO_MEMORY_SIZE);
    return video_ptr;
}

void video_free(void* ptr)
{
    kfree(ptr);
}

void video_save(void* ptr)
{
    memcpy(ptr, (void*) COS32_VIDEO_MEMORY_ADDRESS_START, COS32_VIDEO_MEMORY_SIZE);
}

void video_restore(void* ptr)
{
    memcpy((void*) COS32_VIDEO_MEMORY_ADDRESS_START, ptr, COS32_VIDEO_MEMORY_SIZE);
    	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void video_reset_cursor()
{ 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (unsigned char) (0x00));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (unsigned char) (0x00));
}
