#ifndef PSFFONT_H
#define PSFFONT_H

#include "video/font/font.h"
#include <stdint.h>


#define PSF_FILE_MAGIC_SIGNATURE 0x864AB572
struct psf_font_header
{
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
};

struct video_font *psffont_load(const char *filename, const char* font_name);
#endif