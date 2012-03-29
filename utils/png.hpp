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

#if !defined(__UTILS_PNG_HPP__)
#define __UTILS_PNG_HPP__

#include <stdint.h>
#include <png.h>

static inline void dump_png24(FILE * fd, const uint8_t * data,
                            const size_t width,
                            const size_t height,
                            const size_t rowsize)
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
        png_write_row(ppng, (unsigned char*)row);
        row += rowsize;
    }
    png_write_end(ppng, pinfo);
    png_destroy_write_struct(&ppng, &pinfo);
    // commented line below it to create row capture
    // fwrite(this->data, 3, this->width * this->height, fd);
}

#endif
