/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/

#ifndef _REDEMPTION_UTILS_PNG_HPP_
#define _REDEMPTION_UTILS_PNG_HPP_

#include <stdint.h>
#include <png.h>

#include "transport.hpp"

static inline void png_write_data(png_structp png_ptr, png_bytep data, png_size_t length){
    ((Transport *)(png_ptr->io_ptr))->send(data, length);
}

static inline void png_flush_data(png_structp png_ptr){
    ((Transport *)(png_ptr->io_ptr))->flush();
}

static inline void transport_dump_png24(Transport * trans, const uint8_t * data,
                            const size_t width,
                            const size_t height,
                            const size_t rowsize,
                            const bool bgr)
{
    png_struct * ppng = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_set_write_fn(ppng, trans, &png_write_data, &png_flush_data);

    png_info * pinfo = png_create_info_struct(ppng);
    png_set_IHDR(ppng, pinfo, width, height, 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    png_write_info(ppng, pinfo);

    // send image buffer to file, one pixel row at once
    const uint8_t * row = data;
    for (size_t k = 0 ; k < height ; ++k) {
        if (bgr){
            uint32_t bgrtmp[8192];
            const uint32_t * s = reinterpret_cast<const uint32_t*>(row);
            uint32_t * t = bgrtmp;
            for (size_t n = 0; n < (width / 4) ; n++){
                unsigned bRGB = *s++;
                unsigned GBrg = *s++;
                unsigned rgbR = *s++;
                *t++ = ((GBrg << 16) & 0xFF000000)
                   | ((bRGB << 16) & 0x00FF0000)
                   | (bRGB         & 0x0000FF00)
                   | ((bRGB >> 16) & 0x000000FF) ;
                *t++ = (GBrg         & 0xFF000000)
                   | ((rgbR << 16) & 0x00FF0000)
                   | ((bRGB >> 16) & 0x0000FF00)
                   | ( GBrg        & 0x000000FF) ;
                *t++ = ((rgbR << 16) & 0xFF000000)
                   | (rgbR         & 0x00FF0000)
                   | ((rgbR >> 16) & 0x0000FF00)
                   | ((GBrg >> 16) & 0x000000FF) ;
            }
            png_write_row(ppng, (unsigned char*)bgrtmp);
        }
        else {
            png_write_row(ppng, (unsigned char*)row);
        }
        row += rowsize;
    }
    png_write_end(ppng, pinfo);
    
    trans->flush();
    png_destroy_write_struct(&ppng, &pinfo);
    // commented line below it to create row capture
    // fwrite(this->data, 3, this->width * this->height, fd);
}


static inline void dump_png24(FILE * fd, const uint8_t * data,
                            const size_t width,
                            const size_t height,
                            const size_t rowsize,
                            const bool bgr)
{
    png_struct * ppng = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_info * pinfo = png_create_info_struct(ppng);

    // prepare png header
    png_init_io(ppng, fd);
    png_set_IHDR(ppng, pinfo, width, height, 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    png_write_info(ppng, pinfo);

    // send image buffer to file, one pixel row at once
    const uint8_t * row = data;

    for (size_t k = 0 ; k < height ; ++k) {
        if (bgr){
            uint32_t bgrtmp[8192];
            const uint32_t * s = reinterpret_cast<const uint32_t*>(row);
            uint32_t * t = bgrtmp;
            for (size_t n = 0; n < (width / 4) ; n++){
                unsigned bRGB = *s++;
                unsigned GBrg = *s++;
                unsigned rgbR = *s++;
                *t++ = ((GBrg << 16) & 0xFF000000)
                   | ((bRGB << 16) & 0x00FF0000)
                   | (bRGB         & 0x0000FF00)
                   | ((bRGB >> 16) & 0x000000FF) ;
                *t++ = (GBrg         & 0xFF000000)
                   | ((rgbR << 16) & 0x00FF0000)
                   | ((bRGB >> 16) & 0x0000FF00)
                   | ( GBrg        & 0x000000FF) ;
                *t++ = ((rgbR << 16) & 0xFF000000)
                   | (rgbR         & 0x00FF0000)
                   | ((rgbR >> 16) & 0x0000FF00)
                   | ((GBrg >> 16) & 0x000000FF) ;
            }
            png_write_row(ppng, (unsigned char*)bgrtmp);
        }
        else {
            png_write_row(ppng, (unsigned char*)row);
        }
        row += rowsize;
    }
    png_write_end(ppng, pinfo);
    png_destroy_write_struct(&ppng, &pinfo);
    // commented line below it to create row capture
    // fwrite(this->data, 3, this->width * this->height, fd);
}


inline void read_png24(FILE * fd, const uint8_t * data,
                      const size_t width,
                      const size_t height,
                      const size_t rowsize)
{
    png_struct * ppng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_info * pinfo = png_create_info_struct(ppng);
    png_init_io(ppng, fd);
    png_read_info(ppng, pinfo);

    unsigned char * row = (unsigned char*)data;
    for (size_t k = 0 ; k < height ; ++k) {
        png_read_row(ppng, row, NULL);
        row += rowsize;
    }
    png_read_end(ppng, pinfo);
    png_destroy_read_struct(&ppng, &pinfo, NULL);
}

static inline void png_read_data_fn(png_structp png_ptr, png_bytep data, png_size_t length) { 
    uint8_t * tmp_data = data;
    ((Transport *)(png_ptr->io_ptr))->recv(&tmp_data, length);

}


inline void transport_read_png24(Transport * trans, const uint8_t * data,
                      const size_t width,
                      const size_t height,
                      const size_t rowsize)
{
    png_struct * ppng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_set_read_fn(ppng, trans, &png_read_data_fn);
    png_info * pinfo = png_create_info_struct(ppng);
    png_read_info(ppng, pinfo);

    unsigned char * row = (unsigned char*)data;
    for (size_t k = 0 ; k < height ; ++k) {
        png_read_row(ppng, row, NULL);
        row += rowsize;
    }
    png_read_end(ppng, pinfo);
    png_destroy_read_struct(&ppng, &pinfo, NULL);
}


#endif
