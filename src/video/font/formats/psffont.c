#include "psffont.h"
#include "fs/file.h"
#include "memory/kheap.h"
#include "status.h"
#include "config.h"
static int psffont_load_header(int fd, struct psf_font_header *header_out)
{
    int res = 0;
    res = fseek(fd, 0, SEEK_SET);
    if (res != 0)
    {
        return -EIO;
    }

    res = fread(header_out, sizeof(struct psf_font_header), 1, fd);
    if (res != 1)
    {
        return -EIO;
    }

    if (header_out->magic != PSF_FILE_MAGIC_SIGNATURE)
    {
        return -EINVARG;
    }

    return 0;
}
struct video_font *psffont_load(const char *filename, const char* font_name)
{
    int res = 0;
    struct video_font *font = 0;
    int fd = fopen(filename, "r");
    if (!fd)
    {
        return 0;
    }

    // Let's read the PSF header.
    struct psf_font_header header;
    if (psffont_load_header(fd, &header) != COS32_ALL_OK)
    {
        goto out;
    }

    // Ok we have read the header. Let's create some data for this font
    int total_char_data_bytes = header.numglyph * header.bytesperglyph;
    char *data = kzalloc(total_char_data_bytes);
    // Great lets seek to the data
    res = fseek(fd, header.headersize, SEEK_SET);
    if (res != COS32_ALL_OK)
    {
        goto out;
    }

    res = fread(data, header.bytesperglyph, header.numglyph, fd);
    if (res != (int)header.numglyph)
    {
        res = -EIO;
        goto out;
    }

    // Great we have all the bytes loaded now lets create the font!
    font = video_font_new(font_name, data, header.bytesperglyph, header.numglyph, header.width, header.height);
    if (!font)
    {
        goto out;
    }

    // Font has now been created it can safetly be returned
out:
    fclose(fd);
    return font;
}