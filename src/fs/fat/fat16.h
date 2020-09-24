#include "fs/file.h"
#include "types.h"


#define COS32_FAT16_SIGNATURE 0x29
#define COS32_FAT16_FAT_ENTRY_SIZE 0x02

#define COS32_FAT16_BAD_SECTOR 0xFF7
#define COS32_FAT16_UNUSED 0x00


struct filesystem* fat16_init();