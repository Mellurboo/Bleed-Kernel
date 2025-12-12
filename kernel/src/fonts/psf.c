#include <fonts/psf.h>
#include <string.h>
#include <stdint.h>
#include <fs/vfs.h>
#include <status.h>
#include <mm/heap.h>
#include <stddef.h>
#include <stdio.h>
#include <ansii.h>
#include <drivers/framebuffer/framebuffer.h>

#define PSF_MAGIC_0     0x36
#define PSF_MAGIC_1     0x04
#define PSF_HEADER_SIZE 4

psf1_font_t* psf_load_font(const void* data, size_t size){
    if (!data || size < PSF_HEADER_SIZE) return NULL;
    const uint8_t* b = (const uint8_t *)data;

    if (b[0] != PSF_MAGIC_0 || b[1] != PSF_MAGIC_1){
        kprintf(LOG_ERROR "Not a Valid PSF1 file");
        return NULL;
    }

    uint8_t mode        = b[2];
    uint8_t charsize    = b[3];
    
    size_t glyphcount   = (mode & 0x01) ? 512 : 256;
    size_t glyphbytes   = glyphcount * (size_t)charsize;

    if (size < PSF_HEADER_SIZE + glyphbytes){
        return NULL;
    }

    psf1_font_t* font = (psf1_font_t *)kmalloc(sizeof(psf1_font_t));
    if (!font){
        kprintf(LOG_ERROR "Failure to allocate font space");
        return NULL;
    }

    memset(font, 0, sizeof(*font));
    font->charsize = charsize;
    font->glyph_count = glyphcount;
    font->has_unicode_table = false;
    font->unicode_table = NULL;
    font->unicode_table_size = 0;

    // allocate and copy glyph data
    font->glyphs = (uint8_t *)kmalloc(glyphbytes);
    if (!font->glyphs){
        kfree(font, sizeof(psf1_font_t));
        kprintf(LOG_ERROR "Failure to allocate glyph space");
        return NULL;
    }
    memcpy(font->glyphs, b + PSF_HEADER_SIZE, glyphbytes);

    // data proceeding the glyphs we will treat it as a unicode table
    size_t remaining_data = size - (PSF_HEADER_SIZE + glyphbytes);
    if (remaining_data > 0){
        font->has_unicode_table     = true;
        font->unicode_table_size    = remaining_data; 
        font->unicode_table         = (uint8_t *)kmalloc(remaining_data);
        if (font->unicode_table){
            memcpy(font->unicode_table, b+ PSF_HEADER_SIZE + glyphbytes, remaining_data);
        }else{
            font->has_unicode_table = false;
            font->unicode_table_size = 0;
        }
    }

    return font;
}

psf1_font_t* psf_load_font_file(INode_t* inode){
    if (!inode) return NULL;

    size_t filesize = vfs_filesize(inode);
    if (filesize == 0) return NULL;

    void *buffer = kmalloc(filesize);
    if (!buffer){
        kprintf(LOG_ERROR "Out of Memory");
        return NULL;
    }

    long r = inode_read(inode, buffer, filesize, 0);
    if (r < 0 || (size_t)r != filesize) {
        kfree(buffer, filesize);
        kprintf(LOG_ERROR "Bad Read");
        return NULL;
    }

    psf1_font_t* font = psf_load_font(buffer, filesize);

    kprintf(LOG_OK "Font Allocated at 0x%p (size: %lu)\n", buffer, filesize);
    kprintf(LOG_OK "Font Description:\n\tcharsize: %d\n\tglyph count: %ld\n", font->charsize, font->glyph_count);

    kfree(buffer, filesize);
    return font;
}

const uint8_t* psf_get_glyph(const psf1_font_t *font, uint16_t code){
    if (!font) return NULL;
    if (code >= font->glyph_count){
        kprintf(LOG_ERROR "Bad Glyph Count");
        return NULL;
    }
    return font->glyphs + ((size_t)code * font->charsize);
}

void psf_init(){
    INode_t *font_file;
    path_t font_path;

    font_path = path_from_abs("/initrd/resources/ttyfont.psf");
    int lookup = vfs_lookup(&font_path, &font_file);
    if (lookup < 0) return;
    psf1_font_t *font = psf_load_font_file(font_file);
    
    fontwrite(font, "hello world", 0xFFFFFFFF, 0x00000000);
    inode_drop(font_file);
}

void psf_free(psf1_font_t *font){
    if (!font) return;
    if (font->glyphs) kfree(font->glyphs, (font->glyph_count)*font->charsize);
    if (font->unicode_table) kfree(font->unicode_table, font->unicode_table_size);
    kfree(font, sizeof(psf1_font_t));
}