#ifndef PATHPARSER_H
#define PATHPARSER_H


struct path_root
{
    int drive_no;
    struct path_part* first;
};

struct path_part
{
    const char* part;
    struct path_part* next;
};

/**
 * Parses the given path into a series of path tokens
 */
struct path_root* pathparser_parse(const char* path, const char* current_directory_path);
/**
 * Frees the given path
 */
void pathparser_free(struct path_root* root);

#endif