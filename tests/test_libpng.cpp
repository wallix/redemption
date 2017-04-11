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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Martin Potier

   Unit test to write / read a "movie" from a file
   Using lib boost functions for testing
*/


#define RED_TEST_MODULE TestLibPng
#include "system/redemption_unit_tests.hpp"

// #define LOGNULL

#include <unistd.h>
#include <stdio.h>

#include <png.h>

RED_AUTO_TEST_CASE(TestCreateFrenchFlagPngFile)
{
    // Create a Blue/White/Red stripped image
    // minimal code, minimal options, no error checking, no nothing

   // create image buffer (Vertical stripes, blue, white, red)
    // french flag ? What an amazing surprise!
    const int Bpp = 3;
    const uint32_t width = 192;
    const uint32_t height = 128;

    uint8_t * image = new uint8_t[height*width*Bpp];
    for (uint32_t y = 0; y < height; ++y ){
        for (uint32_t x = 0; x < width/3; ++x ){
            const int pos = Bpp * (x + y*width);
            // BLUE
            image[pos+0] = 0x00; // RED
            image[pos+1] = 0x00; // GREEN
            image[pos+2] = 0xFF; // BLUE
            // WHITE
            image[pos+0+Bpp*(width/3)] = 0xFF; // RED
            image[pos+1+Bpp*(width/3)] = 0xFF; // GREEN
            image[pos+2+Bpp*(width/3)] = 0xFF; // BLUE
            // RED
            image[pos+0+Bpp*((2*width)/3)] = 0xFF; // RED
            image[pos+1+Bpp*((2*width)/3)] = 0x00; // GREEN
            image[pos+2+Bpp*((2*width)/3)] = 0x00; // BLUE
        }
    }

    // ------------------------------- Write png file -----------------------
    {
        FILE * fp = fopen("/tmp/fflag.png", "wb");
        png_struct * ppng = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        png_info * pinfo = png_create_info_struct(ppng);

        // prepare png header
        png_init_io(ppng, fp);
        int color_type = PNG_COLOR_TYPE_RGB;
        const int bit_depth = 8;
        png_set_IHDR(ppng, pinfo, width, height, bit_depth, color_type,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE,
                     PNG_FILTER_TYPE_BASE);
        png_write_info(ppng, pinfo);

        // send image buffer to file, one pixel row at once
        for (uint32_t k = 0; k < height; ++k ) {
            png_write_row(ppng, image + k*width*Bpp);
        }
        png_write_end(ppng, pinfo);
        png_destroy_write_struct(&ppng, &pinfo);
        fclose(fp);
    }
    delete[] image;

    // ------------------------------- Then read it back --------------------
    {
        FILE * fp = fopen("/tmp/fflag.png", "rb");

        const int PNG_BYTES_TO_CHECK = 4;
        uint8_t buf[PNG_BYTES_TO_CHECK];

        RED_CHECK_EQUAL(PNG_BYTES_TO_CHECK, fread(buf, 1, PNG_BYTES_TO_CHECK, fp));

        RED_CHECK_EQUAL(0, png_sig_cmp(buf, 0, PNG_BYTES_TO_CHECK));

        png_struct * ppng = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        png_info * pinfo = png_create_info_struct(ppng);
        // using standard C Stream io
        png_init_io(ppng, fp);
        // tell to libpng we have read some signature bytes directly
        png_set_sig_bytes(ppng, PNG_BYTES_TO_CHECK);

        png_read_info(ppng, pinfo);

        png_uint_32 width;
        png_uint_32 height;
        int bit_depth;
        int color_type;
        int interlace_type;
        int compression_type;
        int filter_type;
        png_get_IHDR(ppng, pinfo, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_type);
        RED_CHECK_EQUAL(192, width);
        RED_CHECK_EQUAL(128, height);
        RED_CHECK_EQUAL(PNG_COLOR_TYPE_RGB, color_type);
        RED_CHECK_EQUAL(PNG_INTERLACE_NONE, interlace_type);
        RED_CHECK_EQUAL(PNG_COMPRESSION_TYPE_BASE, compression_type);
        RED_CHECK_EQUAL(PNG_FILTER_TYPE_BASE, filter_type);

        uint8_t * row_pointers[128];
        for (uint32_t row = 0; row < height; row++){
            row_pointers[row] = reinterpret_cast<uint8_t *>(png_malloc(ppng, png_get_rowbytes(ppng, pinfo)));
        }
        png_read_image(ppng, row_pointers);

        // check that we got back what we put in the file
        for (uint32_t y = 0; y < height; ++y ){
            uint8_t * row = row_pointers[y];
            for (uint32_t x = 0; x < width/3; ++x ){
                const int pos = Bpp * x;
                // BLUE
                RED_CHECK_EQUAL(0x00, row[pos+0]); // RED
                RED_CHECK_EQUAL(0x00, row[pos+1]); // GREEN
                RED_CHECK_EQUAL(0xFF, row[pos+2]); // BLUE
                // WHITE
                RED_CHECK_EQUAL(0xFF, row[pos+0+Bpp*(width/3)]); // RED
                RED_CHECK_EQUAL(0xFF, row[pos+1+Bpp*(width/3)]); // GREEN
                RED_CHECK_EQUAL(0xFF, row[pos+2+Bpp*(width/3)]); // BLUE
                // RED
                RED_CHECK_EQUAL(0xFF, row[pos+0+Bpp*((2*width)/3)]); // RED
                RED_CHECK_EQUAL(0x00, row[pos+1+Bpp*((2*width)/3)]); // GREEN
                RED_CHECK_EQUAL(0x00, row[pos+2+Bpp*((2*width)/3)]); // BLUE
            }
        }
        for (uint32_t row = 0; row < height; row++){
            png_free(ppng, row_pointers[row]);
        }
        png_read_end(ppng, pinfo);
        png_destroy_read_struct(&ppng, &pinfo, nullptr);
        fclose(fp);

        unlink("/tmp/fflag.png");
    }
    // ----------------------------------------------------------------------

}
