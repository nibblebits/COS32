#include "pparser.h"
#include "string/string.h"
#include "memory/memory.h"
#include "memory/kheap.h"
#include "config.h"
#include "status.h"

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
static int path_valid_format(const char *filename)
{
    int len = strnlen(filename, COS32_MAX_PATH);
    return (len >= 3 && isdigit(filename[0])) && memcmp((void *)&filename[1], ":/", 2) == 0;
}

/**
 * Gets the drive from the given path. If found repositons the pointer to point to after the drive number part of the path
 */
static int path_get_drive_by_path(const char **path)
{
    if (!path_valid_format(*path))
    {
        return -EBADPATH;
    }

    int drive_no = tonumericdigit(*path[0]);

    // Add 3 to skip "0:/" or "1:/". You know the drive number...
    *path += 3;
    return drive_no;
}

static struct path_root *pathparser_create_root(int drive_number)
{
    struct path_root *path_r = kzalloc(sizeof(struct path_root));
    path_r->drive_no = drive_number;
    path_r->first = 0;
    return path_r;
}

static const char *pathparser_get_path_part(const char **path)
{
    char *result_path_part = kzalloc(COS32_MAX_PATH);
    int i = 0;
    while (**path != '/' && **path != 0x00)
    {
        result_path_part[i] = **path;
        *path += 1;
        i++;
    }

    if (**path == '/')
    {
        // Skip the forward slash we found or we will have problems later on...
        *path +=1 ;
    }

    if (i == 0)
    {
        // We couldn't parse anything
        kfree(result_path_part);
        result_path_part = 0;
    }

    return result_path_part;
}

struct path_part *pathparser_parse_path_part(struct path_part *last_part, const char **path)
{
    // We are responsible for deleting this string returned to us. from pathparser_get_path_part
    const char *path_part_str = pathparser_get_path_part(path);
    if (!path_part_str)
    {
        return 0;
    }

    struct path_part *part = kzalloc(sizeof(struct path_part));
    part->part = path_part_str;
    part->next = 0x00;

    if (last_part)
    {
        last_part->next = part;
    }
    return part;
}

void pathparser_free(struct path_root* root)
{
    struct path_part* part = root->first;
    while(part)
    {
        struct path_part* next_part = part->next;
        kfree(part->part);
        kfree(part);
        part = next_part;
    }

    kfree(root);
}

struct path_root *pathparser_parse(const char *path, const char *current_directory_path)
{
    int res = 0;
    const char *tmp_path = path;
    struct path_root *path_root = 0;

    // We don't allow path's too large..
    if (strlen(path) >= COS32_MAX_PATH)
    {
        goto out;
    }

    res = path_get_drive_by_path(&tmp_path);
    if (res < 0)
    {
        goto out;
    }

    path_root = pathparser_create_root(res);
    if (!path_root)
    {
        goto out;
    }

    struct path_part *first_part = pathparser_parse_path_part(NULL, &tmp_path);
    if (first_part)
    {
        path_root->first = first_part;
    }

    struct path_part *part = pathparser_parse_path_part(first_part, &tmp_path);
    while (part)
    {
        part = pathparser_parse_path_part(part, &tmp_path);
    }
out:
    return path_root;
}