#ifndef FILE_H
#define FILE_H
#include <stdint.h>

typedef unsigned int FILE_SEEK_MODE;
enum
{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

typedef unsigned int FILE_MODE;
enum
{
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};

struct disk;
typedef void *(*FS_OPEN_FUNCTION)(struct disk *disk, char *filename, FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk *disk);
typedef int (*FS_CLOSE_FUNCTION)(void *private);
typedef int (*FS_SEEK_FUNCTION)(void* private, uint32_t offset, FILE_SEEK_MODE seek_mode);

/**
 * 
 * Implementor should cast "private" to its own private data. The private data should represent a local file descriptor
 * for its own file in its own filesystem, use size and nmemb to know how many bytes to read back to us in pointer "out"
 */
typedef int (*FS_READ_FUNCTION)(struct disk *disk, void *private, uint32_t size, uint32_t nmemb, char *out);

struct filesystem
{
    // Filesystem should return zero from resolve if the provided disk is using its filesystem
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    FS_CLOSE_FUNCTION close;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    char name[20];
};

struct file_descriptor
{
    // The descriptor index
    int index;
    struct filesystem *filesystem;
    // Private data for the actual filesystem who this descriptor belongs to
    void *private;

    // The disk that this file descriptor should be used on
    struct disk* disk;
};

void fs_init();
int fopen(char *filename, const char *mode_str);
int fread(void *ptr, uint32_t size, uint32_t nmemb, int fd);
int fseek(int fd, int offset, FILE_SEEK_MODE whence);

int fclose(int fd);
void fs_insert_filesystem(struct filesystem *filesystem);
struct filesystem *fs_resolve(struct disk *disk);
#endif