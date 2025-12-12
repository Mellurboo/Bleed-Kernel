#ifndef PSF1_H
#define PSF1_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <fs/vfs.h>

typedef struct psf1_font {
    uint8_t *glyphs;
    size_t glyph_count;
    uint8_t charsize;
    bool has_unicode_table;
    uint8_t *unicode_table;
    size_t unicode_table_size;
} psf1_font_t;

psf1_font_t* psf_load_font(const void *data, size_t size);
psf1_font_t* psf_load_font_file(INode_t *inode);

const uint8_t* psf_get_glyph(const psf1_font_t *font, uint16_t code);
void psf_free(psf1_font_t *font);

void psf_init();

#endif