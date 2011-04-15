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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test for bitmap cache class

*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE TestBitmap
#include <boost/test/auto_unit_test.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "bitmap.hpp"
#include "colors.hpp"
#include "config.hpp"
#include <sys/time.h>

BOOST_AUTO_TEST_CASE(TestBitmapCompressHardenned)
{
    const unsigned white = 0xFF;

    // test COLOR COUNT EMPTY
    {
        Bitmap bmp(8, 0, 4);
        BOOST_CHECK_EQUAL(0, bmp.get_color_count(bmp.data_co, 0xFF));
    }

    // test COLOR COUNT
    {
        uint8_t data[4*4] = {
            0x05, 0x05, 0x05, 0x05,
            0x05, 0x05, 0x05, 0x05,
            0x05, 0x05, 0x05, 0x05,
            0x05, 0x05, 0x05, 0x05};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        BOOST_CHECK_EQUAL(16, bmp.get_color_count(bmp.data_co, 0x05));
        BOOST_CHECK_EQUAL(0, bmp.get_color_count(bmp.data_co, 0xFE));
    }

    // test COLOR COUNT 2
    {
        uint8_t data[4*4] = {
            0x01, 0x02, 0x02, 0x04,
            0x05, 0x06, 0x07, 0x08,
            0x09, 0x0A, 0x0B, 0x0C,
            0x0D, 0x0E, 0x0F, 0x10};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        BOOST_CHECK_EQUAL(1, bmp.get_color_count(bmp.data_co, 0x01));
        BOOST_CHECK_EQUAL(2, bmp.get_color_count(bmp.data_co + 1, 0x02));
    }

    // test BICOLOR COUNT EMPTY
    {
        Bitmap bmp(8, 0, 4);
        BOOST_CHECK_EQUAL(0, bmp.get_bicolor_count(bmp.data_co, 0xEF, 0xFE));
    }

    // test BICOLOR COUNT
    {
        uint8_t data[4*4] = {
            0x01, 0x05, 0x01, 0x05,
            0x01, 0x05, 0x01, 0x05,
            0x01, 0x05, 0x01, 0x05,
            0x01, 0x05, 0x01, 0x05};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        BOOST_CHECK_EQUAL(16, bmp.get_bicolor_count(bmp.data_co, 0x01, 0x05));
        BOOST_CHECK_EQUAL(14, bmp.get_bicolor_count(bmp.data_co+1, 0x05, 0x01));
        BOOST_CHECK_EQUAL(0 , bmp.get_bicolor_count(bmp.data_co, 0x05, 0x01));
    }

    // test BICOLOR COUNT
    {
        uint8_t data[4*4] = {
            0x01, 0x05, 0x01, 0x05,
            0x01, 0x05, 0x01, 0x05,
            0x01, 0x05, 0x01, 0x01,
            0x01, 0x05, 0x01, 0x05};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        BOOST_CHECK_EQUAL(10, bmp.get_bicolor_count(bmp.data_co, 0x01, 0x05));
        BOOST_CHECK_EQUAL(10, bmp.get_bicolor_count(bmp.data_co + 1, 0x05, 0x01));
    }

    // test FILL COUNT
    {
        Bitmap bmp(8, 0, 4);
        BOOST_CHECK_EQUAL(0, bmp.get_fill_count(bmp.data_co));
    }

    // test FILL COUNT
    {
        uint8_t data[4*4] = {
            0x00, 0x00, 0x00, 0x00,
            0x01, 0x05, 0x01, 0x05,
            0x01, 0x05, 0x01, 0x01,
            0x01, 0x05, 0x01, 0x05};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));

        // Line above first line is black
        BOOST_CHECK_EQUAL(4, bmp.get_fill_count(bmp.data_co));
        BOOST_CHECK_EQUAL(3, bmp.get_fill_count(bmp.data_co+1));

        // 3rd line, compared to 2nd line
        BOOST_CHECK_EQUAL(3, bmp.get_fill_count(bmp.data_co+8));
    }

    // test FILL COUNT
    {
        uint8_t data[4*4] = {
            0x01, 0x00, 0x01, 0x00,
            0x01, 0x05, 0x01, 0x01,
            0x01, 0x05, 0x01, 0x01,
            0x01, 0x05, 0x01, 0x01};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));

        // Line above first line is black
        BOOST_CHECK_EQUAL(0, bmp.get_fill_count(bmp.data_co));
        BOOST_CHECK_EQUAL(1, bmp.get_fill_count(bmp.data_co+1));
        BOOST_CHECK_EQUAL(0, bmp.get_fill_count(bmp.data_co+2));
        BOOST_CHECK_EQUAL(2, bmp.get_fill_count(bmp.data_co+3));

        // until the end
        BOOST_CHECK_EQUAL(8, bmp.get_fill_count(bmp.data_co+8));

    }

    // test MIX COUNT
    {
        Bitmap bmp(8, 0, 4);
        BOOST_CHECK_EQUAL(0, bmp.get_mix_count(bmp.data_co, white));
    }

    // test MIX COUNT
    {
        uint8_t data[4*4] = {
            0xFF, 0xFF, 0xFF, 0xFF,
            0x01, 0x05, 0x01, 0x05,
            0xFE, 0xFA, 0xFE, 0x01,
            0x01, 0x05, 0x01, 0x05};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));

        // Line above first line is black
        BOOST_CHECK_EQUAL(4, bmp.get_mix_count(bmp.data_co, white));
        BOOST_CHECK_EQUAL(3, bmp.get_mix_count(bmp.data_co+1, white));

        // 3rd line, compared to 2nd line
        BOOST_CHECK_EQUAL(3, bmp.get_mix_count(bmp.data_co+8, white));
    }

    // test MIX COUNT
    {
        uint8_t data[4*4] = {
            0x01, 0xFF, 0x01, 0xFF,
            0xFE, 0x05, 0x01, 0x01,
            0x01, 0xFA, 0xFE, 0xFE,
            0xFE, 0x05, 0x01, 0x01};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));

        // Line above first line is black
        BOOST_CHECK_EQUAL(0, bmp.get_mix_count(bmp.data_co, white));
        BOOST_CHECK_EQUAL(1, bmp.get_mix_count(bmp.data_co+1, white));
        BOOST_CHECK_EQUAL(0, bmp.get_mix_count(bmp.data_co+2, white));
        BOOST_CHECK_EQUAL(2, bmp.get_mix_count(bmp.data_co+3, white));

        // until the end
        BOOST_CHECK_EQUAL(8, bmp.get_mix_count(bmp.data_co+8, white));
    }

    // test FILL OR MIX COUNT
    {
        uint8_t data[4*4] = {
            0x02, 0x03, 0x04, 0x05,
            0xFD, 0x03, 0xFB, 0x05,
            0xFD, 0xFC, 0xFB, 0xFA,
            0x02, 0x03, 0x04, 0xFA};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));

        // Line above first line is black
        uint8_t masks[512];
        BOOST_CHECK_EQUAL(0, bmp.get_fom_count(bmp.data_co+15, white));
        BOOST_CHECK_EQUAL(2, bmp.get_fom_count(bmp.data_co+14, white));
        bmp.get_fom_masks(bmp.data_co+14, masks, 2);
        BOOST_CHECK_EQUAL(0x01, masks[0]);


        BOOST_CHECK_EQUAL(4, bmp.get_fom_count(bmp.data_co+12, white));
        bmp.get_fom_masks(bmp.data_co+12, masks, 4);
        BOOST_CHECK_EQUAL(0x07, masks[0]);

        BOOST_CHECK_EQUAL(5, bmp.get_fom_count(bmp.data_co+11, white));
        BOOST_CHECK_EQUAL(6, bmp.get_fom_count(bmp.data_co+10, white));

        BOOST_CHECK_EQUAL(12, bmp.get_fom_count(bmp.data_co+4, white));
        bmp.get_fom_masks(bmp.data_co+4, masks, 12);
        BOOST_CHECK_EQUAL(0xA5, masks[0]);
        BOOST_CHECK_EQUAL(0x07, masks[1]);
    }

    {
        BOOST_CHECK_EQUAL(1, 1);
        uint8_t data[4*4] = {
            0x01, 0x02, 0x03, 0x04,
            0x05, 0x06, 0x07, 0x08,
            0x09, 0x0A, 0x0B, 0x0C,
            0x0D, 0x0E, 0x0F, 0x10};
        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));

        unsigned foreground = white;
        unsigned flags = 0;
        BOOST_CHECK_EQUAL(4, bmp.get_mix_count_set(bmp.data_co+3, foreground));
        BOOST_CHECK_EQUAL(0x04, foreground);
        foreground = white;
        BOOST_CHECK_EQUAL(4, bmp.get_fom_count_set(bmp.data_co+3, foreground, flags));
        BOOST_CHECK_EQUAL(0x04, foreground);
    }


    {
        BOOST_CHECK_EQUAL(1, 1);
        uint8_t multicolor[4*4] = {
            0x01, 0x02, 0x03, 0x04,
            0x01, 0x01, 0x01, 0x01,
            0x05, 0x06, 0x07, 0x08,
            0x01, 0x01, 0x01, 0x01};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, multicolor, sizeof(multicolor));

        unsigned foreground = white;
        BOOST_CHECK_EQUAL(1, bmp.get_mix_count_set(bmp.data_co+3, foreground));
        BOOST_CHECK_EQUAL(4, foreground);
        foreground = white;
        unsigned flags = 0;
        BOOST_CHECK_EQUAL(2, bmp.get_fom_count_set(bmp.data_co+3, foreground, flags));
        BOOST_CHECK_EQUAL(4, foreground);
        BOOST_CHECK_EQUAL(3, flags); // MIX then FILL

    }


}

BOOST_AUTO_TEST_CASE(TestBitmapCompress)
{
    // test COPY
    {
        BOOST_CHECK_EQUAL(1, 1);
        uint8_t data[4*4] = {
            0x01, 0x02, 0x03, 0x04,
            0x06, 0x07, 0x08, 0x09,
            0x0A, 0x0C, 0x0D, 0x0E,
            0x0F, 0x10, 0x12, 0x13};

//        printf("------- Raw pixels ---------\n");
//        for (int i = 0; i < sizeof(data); i++){
//            printf("%.2x ", data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        Stream out(256);
        bmp.compress(out);
        uint8_t expected[] = {
            0x84, 0x01, 0x02, 0x03, 0x04,
            0x8c, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x12, 0x13
        };


        BOOST_CHECK_EQUAL(sizeof(expected), out.p - out.data);
        BOOST_CHECK(0 == memcmp(out.data, expected, sizeof(expected)));

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        Bitmap bmp2(8, 4, 4);
        bmp2.decompress(out.data, out.p - out.data);
        BOOST_CHECK_EQUAL(bmp2.bmp_size, sizeof(data));
        BOOST_CHECK(0 == memcmp(bmp2.data_co, data, sizeof(data)));

//        printf("------- Decompress ---------\n");
//        for (int i = 0; i < bmp2.bmp_size; i++){
//            printf("%.2x ", bmp2.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");
    }

    // test COPY 16 bits
    {
        uint8_t data[4*4*2] = {
            0x01, 0, 0x02, 0, 0x03, 0, 0x04, 0,
            0x05, 0, 0x06, 0, 0x07, 0, 0x08, 0,
            0x09, 0, 0x0A, 0, 0x0B, 0, 0x0C, 0,
            0x0D, 0, 0x0E, 0, 0x0F, 0, 0x10, 0};

//        printf("------- Raw pixels ---------\n");
//        for (int i = 0; i < sizeof(data); i++){
//            printf("%.2x ", data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        Bitmap bmp(16, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        Stream out(256);
        bmp.compress(out);
        uint8_t expected[] = {
            0x84, 0x01, 0, 0x02, 0, 0x03, 0, 0x04, 0, // 4 COPY

            0xc3, 0x04, 0, // 3 MIX SET
            0x81, 0x08, 0, // 1 COPY

            0xc3, 0x0c, 0, // 3 MIX SET
            0x81, 0x0C, 0, // 1 COPY

            0xc3, 0x04, 0, // 3 MIX SET
            0x81, 0x10, 0, // 1 COPY
        };


        BOOST_CHECK_EQUAL(sizeof(expected), out.p - out.data);
        BOOST_CHECK(0 == memcmp(out.data, expected, sizeof(expected)));

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        Bitmap bmp2(16, 4, 4);
        memset(bmp2.data_co, 0, 2*4*4);
        bmp2.decompress(out.data, out.p - out.data);
        BOOST_CHECK_EQUAL(bmp2.bmp_size, sizeof(data));
        BOOST_CHECK(0 == memcmp(bmp2.data_co, data, sizeof(data)));

//        printf("------- Decompressed ---------\n");
//        for (int i = 0; i < bmp2.bmp_size; i++){
//            printf("%.2x ", bmp2.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");

    }

    // test COLOR
    {
        uint8_t data[4*4] = {
            0x01, 0x02, 0x03, 0x04,
            0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        Stream out(256);
        bmp.compress(out);
        uint8_t expected[] = {
            0x84, 0x01, 0x02, 0x03, 0x04,
            0x64, 0x01,
            0x04,
            0x04};

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        BOOST_CHECK_EQUAL(sizeof(expected), out.p - out.data);
        BOOST_CHECK(0 == memcmp(out.data, expected, sizeof(expected)));

        Bitmap bmp2(8, 4, 4);
        bmp2.decompress(out.data, out.p - out.data);
        BOOST_CHECK_EQUAL(bmp2.bmp_size, sizeof(data));
        BOOST_CHECK(0 == memcmp(bmp2.data_co, data, sizeof(data)));

    }

    // test COLOR then COPY
    {
        uint8_t data[4*4] = {
            0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x0F};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        Stream out(256);
        bmp.compress(out);

        printf("------- Compressed ---------\n");
        for (int i = 0; i < (out.p - out.data); i++){
            printf("%.2x, ", out.data[i]);
        }
        printf("\n");
        printf("\n----------------------------\n");
        printf("\n");

        uint8_t expected[] = {
            0xC4, 0x01,
            0x04,
            0x04,
            0x03, 0x81, 0x0F};

        BOOST_CHECK_EQUAL(sizeof(expected), out.p - out.data);
        BOOST_CHECK(0 == memcmp(out.data, expected, sizeof(expected)));

        Bitmap bmp2(8, 4, 4);
        bmp2.decompress(out.data, out.p - out.data);
        BOOST_CHECK_EQUAL(bmp2.bmp_size, sizeof(data));
        BOOST_CHECK(0 == memcmp(bmp2.data_co, data, sizeof(data)));
    }

    // test interleaved COLOR and COPY
    {
        BOOST_CHECK_EQUAL(1, 1);
        uint8_t data[4*4] = {
            0x01, 0x02, 0x03, 0x04,
            0x01, 0x01, 0x01, 0x01,
            0x05, 0x06, 0x07, 0x08,
            0x01, 0x01, 0x01, 0x01};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        Stream out(256);
        bmp.compress(out);

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        uint8_t expected[] = {
            0x84, 0x01, 0x02, 0x03, 0x04, // COPY
            0x64, 0x01,                   // COLOR
            0x84, 0x05, 0x06, 0x07, 0x08, // COPY
            0x64, 0x01                    // COLOR
        };

        BOOST_CHECK_EQUAL(sizeof(expected), out.p - out.data);
        BOOST_CHECK(0 == memcmp(out.data, expected, sizeof(expected)));

        Bitmap bmp2(8, 4, 4);
        bmp2.decompress(out.data, out.p - out.data);
        BOOST_CHECK_EQUAL(bmp2.bmp_size, sizeof(data));
        BOOST_CHECK(0 == memcmp(bmp2.data_co, data, sizeof(data)));

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

    }

    // test BICOLOR
    {
        uint8_t data[24] = {
            0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02,
            0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02,
            0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02,

        };

//        printf("------- Raw pixels ---------\n");
//        for (int i = 0; i < sizeof(data); i++){
//            printf("%.2x ", data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        Bitmap bmp(8, 24, 1);
        memcpy(bmp.data_co, data, sizeof(data));
        Stream out(256);
        bmp.compress(out);
        uint8_t expected[] = {
            0xeC, 0x01, 0x02 // BICOLOR
        };
        BOOST_CHECK_EQUAL(sizeof(expected), out.p - out.data);
        BOOST_CHECK(0 == memcmp(out.data, expected, sizeof(expected)));

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        Bitmap bmp2(8, 24, 1);
        bmp2.decompress(out.data, out.p - out.data);
        BOOST_CHECK_EQUAL(bmp2.bmp_size, sizeof(data));
        BOOST_CHECK(0 == memcmp(bmp2.data_co, data, sizeof(data)));

//        printf("------- Decompressed ---------\n");
//        for (int i = 0; i < bmp2.bmp_size; i++){
//            printf("%.2x ", bmp2.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");

    }

    // test FILL
    {
        uint8_t data[4*4] = {
            0x02, 0x03, 0x04, 0x05,
            0x02, 0x03, 0x04, 0x05,
            0x02, 0x03, 0x04, 0x05,
            0x02, 0x03, 0x04, 0x05};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        Stream out(256);
        bmp.compress(out);

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        uint8_t expected[] = {
            0x84, 0x02, 0x03, 0x04, 0x05, // 4 COPY
            0x04, // 4 FILL
            0x04, // 4 FILL
            0x04, // 4 FILL
        };
        BOOST_CHECK_EQUAL(sizeof(expected), out.p - out.data);
        BOOST_CHECK(0 == memcmp(out.data, expected, sizeof(expected)));

        Bitmap bmp2(8, 4, 4);
        bmp2.decompress(out.data, out.p - out.data);
        BOOST_CHECK_EQUAL(bmp2.bmp_size, sizeof(data));
        BOOST_CHECK(0 == memcmp(bmp2.data_co, data, sizeof(data)));
    }

    // test MIX
    {
        uint8_t data[4*4] = {
            0x02, 0x03, 0x04, 0x05,
            0xFD, 0xFC, 0xFB, 0xFA,
            0x02, 0x03, 0x04, 0x05,
            0xFD, 0xFC, 0xFB, 0xFA};

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        Stream out(256);
        bmp.compress(out);
        uint8_t expected[] = {
            0x84, 0x02, 0x03, 0x04, 0x05, // 4 COPY
            0x24, // 4 MIX
            0x24, // 4 MIX
            0x24, // 4 MIX
        };

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        BOOST_CHECK_EQUAL(sizeof(expected), out.p - out.data);
        BOOST_CHECK(0 == memcmp(out.data, expected, sizeof(expected)));

        Bitmap bmp2(8, 4, 4);
        bmp2.decompress(out.data, out.p - out.data);
        BOOST_CHECK_EQUAL(bmp2.bmp_size, sizeof(data));
        BOOST_CHECK(0 == memcmp(bmp2.data_co, data, sizeof(data)));
    }


    // test FOM
    {
        uint8_t data[4*4] = {
            0x02, 0x03, 0x04, 0x05,
            0xFD, 0x03, 0xFB, 0x05,
            0xFD, 0xFC, 0xFB, 0xFA,
            0x03, 0x03, 0x03, 0x03};

//        printf("------- Raw pixels ---------\n");
//        for (int i = 0; i < sizeof(data); i++){
//            printf("%.2x ", data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        Stream out(256);
        bmp.compress(out);
        uint8_t expected[] = {
            0x84, 0x02, 0x03, 0x04, 0x05, // 4 COPY
            0x40, 0x03, 0x05,             // 4 FOM 0101
            0x40, 0x03, 0x0a,             // 4 FOM 1010
            0x64, 0x03                    // 4 COLOR
        };

        BOOST_CHECK_EQUAL(sizeof(expected), out.p - out.data);
        BOOST_CHECK(0 == memcmp(out.data, expected, sizeof(expected)));

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        Bitmap bmp2(8, 4, 4);
        bmp2.decompress(out.data, out.p - out.data);
        BOOST_CHECK_EQUAL(bmp2.bmp_size, sizeof(data));
        BOOST_CHECK(0 == memcmp(bmp2.data_co, data, sizeof(data)));

//        printf("------- Decompressed ---------\n");
//        for (int i = 0; i < bmp2.bmp_size; i++){
//            printf("%.2x ", bmp2.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");

    }

    // test FOM 2
    {
        uint8_t data[4*4] = {
            0x02, 0x03, 0x04, 0x05,
            0xFD, 0x03, 0xFB, 0x05,
            0xFD, 0xFC, 0xFB, 0xFA,
            0x02, 0xFC, 0x04, 0xFA};

//        printf("------- Raw pixels ---------\n");
//        for (int i = 0; i < sizeof(data); i++){
//            printf("%.2x ", data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        Bitmap bmp(8, 4, 4);
        memcpy(bmp.data_co, data, sizeof(data));
        Stream out(256);
        bmp.compress(out);
        uint8_t expected[] = {
            0x84, 0x02, 0x03, 0x04, 0x05, // COPY
            0x40, 0x03, 0x05,  // then Fill Or Mix, (3 + 1) pixels
            0x40, 0x03, 0x0a,  // then Fill Or Mix, (3 + 1) pixels
            0x40, 0x03, 0x05,  // then Fill Or Mix, (3 + 1) pixels
        };
        BOOST_CHECK_EQUAL(sizeof(expected), out.p - out.data);
        BOOST_CHECK(0 == memcmp(out.data, expected, sizeof(expected)));

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        Bitmap bmp2(8, 4, 4);
        bmp2.decompress(out.data, out.p - out.data);
        BOOST_CHECK_EQUAL(bmp2.bmp_size, sizeof(data));
        BOOST_CHECK(0 == memcmp(bmp2.data_co, data, sizeof(data)));

//        printf("------- Decompressed ---------\n");
//        for (int i = 0; i < bmp2.bmp_size; i++){
//            printf("%.2x ", bmp2.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");

    }

    {
        uint8_t compressed[] = {
            0x40, 0x08, 0x33,                               // 9 FOM 9
            0x0f, 0x66, 0xbb,                               // 15 FILL
            0xd6, 0x62, 0x00, 0x00, 0x40, 0x09, 0x99, 0x00, // 48 FOM SET
            0x66, 0xbb,                                     // 6 COLOR
            0xd6, 0xd0, 0x04, 0xbb, 0xd6, 0x0c, 0x81, 0x00, // 48 FOM SET
        };

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        BOOST_CHECK(1);
        Bitmap bmp2(8, 256, 3);
        bmp2.decompress(compressed, sizeof(compressed));

//        printf("------- Decompressed ---------\n");
//        for (int i = 0; i < bmp2.bmp_size; i++){
//            printf("%.2x ", bmp2.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");
    }


    // functional test , 24 bits, various orders with SET variants
    {
        uint8_t raw[2*16*3] = {
        0x0c, 0x0c, 0xea,  0x0c, 0x0c, 0xea,  0x0c, 0x0c, 0xea,  0x0c, 0x0c, 0xea,
        0x07, 0x07, 0x8e,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,  0xc8, 0x2a, 0x0a,  0xea, 0x31, 0x0c,  0xea, 0x31, 0x0c,

        0x0c, 0x0c, 0xea,  0x0c, 0x0c, 0xea,  0x0c, 0x0c, 0xea,  0x09, 0x09, 0xbe,
        0x00, 0x00, 0x0b,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,  0x9f, 0x21, 0x08,  0xea, 0x31, 0x0c,  0xea, 0x31, 0x0c
        };


//        printf("------- Raw pixels ---------\n");
//        for (int i = 0; i < sizeof(raw); i++){
//            printf("%.2x ", raw[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");


        Bitmap bmp(24, 16, 2);
        memcpy(bmp.data_co, raw, sizeof(raw));
        Stream out(256);
        bmp.compress(out);

        uint8_t expected[] = {
            0xc4, 0x0c, 0x0c, 0xea,                   // 4 MIX SET 0xea0c0c
            0xd0, 0x08, 0x07, 0x07, 0x8e, 0x01, 0x00, // 9 FOM SET 0x8e0707 0b00000001 0b00000000
            0x81, 0xc8, 0x2a, 0x0a,                   // 1 COPY 0x0a2ac8

//            0xd0, 0x04, 0xea, 0x31, 0x0c, 0x03,       // 5 FOM SET 0x0C31EA 0b00000011
            0xC2, 0xEA, 0x31, 0x0C,                   // 2 MIX SET 0x0C31EA
            0x03,                                     // 3 FILL

            0x81, 0x09, 0x09, 0xbe,                   // 1 COPY 0xbe0909
            0xd0, 0x08, 0x07, 0x07, 0x85, 0x01, 0x00, // 9 FOM SET 0xB0000^0x8e0707 0b00000001 0b00000000
            0xd0, 0x02, 0x57, 0x0b, 0x02, 0x01        // 3 FOM SET 0x08219F 0b00000001
        };

// c2 ea 31 0c 03

        BOOST_CHECK_EQUAL(sizeof(expected), out.p - out.data);
        BOOST_CHECK(0 == memcmp(out.data, expected, sizeof(expected)));

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        Bitmap bmp2(24, 16, 2);
        bmp2.decompress(out.data, out.p - out.data);
        BOOST_CHECK_EQUAL(bmp2.bmp_size, sizeof(raw));
        BOOST_CHECK(0 == memcmp(bmp2.data_co, raw, sizeof(raw)));

//        printf("------- Decompressed ---------\n");
//        for (int i = 0; i < bmp2.bmp_size; i++){
//            printf("%.2x ", bmp2.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");
    }


    {
        uint8_t compressed[] = {
            // FOM SET 16px Mix 0x0842 MASK 0x00, 00
            0xd0, 0x0F, 0x08, 0x42, 0x0F, 0x0F,
            // COPY 4px
            0x81, 0x01, 0x02
        };

        Bitmap bmp(16, 64, 10);
        bmp.decompress(compressed, 9);

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        uint8_t expected[] = {
            0x08, 0x42, 0x08, 0x42, 0x08, 0x42, 0x08, 0x42,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x08, 0x42, 0x08, 0x42, 0x08, 0x42, 0x08, 0x42,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x02
        };

//        printf("------- Decompressed ---------\n");
//        for (int i = 0; i < bmp.bmp_size; i++){
//            printf("%.2x ", bmp.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");

        BOOST_CHECK(0 == memcmp(bmp.data_co, expected, sizeof(expected)));
    }


    {
        uint8_t compressed[] = {
            // 1_COPY 31_FILL 1_MIX 31_FILL
            // the mix is magically inserted because we have 2 back to back FILL
            // on the first scanline (don't ask why. See RDPBCGR 2.2.9.1.1.3.1.2.4
            // Background Run Orders).
            0x81, 0x01, 0x02,  0x1F,  0x1F,
            0x00, 0x40, // FILL 64
            0x00, 0x40, // FILL 64
            0x00, 0x40, // FILL 64
            0x00, 0x40, // FILL 64
            0x00, 0x40, // FILL 64
            0x00, 0x40, // FILL 64
            0x00, 0x40, // FILL 64
            0x00, 0x40, // FILL 64
            0x00, 0x40, // FILL 64
        };

        Bitmap bmp(16, 64, 10);
        bmp.decompress(compressed, 9);

        uint8_t expected[] = {
        };

//        printf("------- Decompressed ---------\n");
//        for (int i = 0; i < bmp.bmp_size; i++){
//            printf("%.2x ", bmp.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");

//        BOOST_CHECK(0 == memcmp(bmp.data_co, expected, sizeof(expected)));
    }

    {
        uint8_t compressed[] = {
            0x00, 0x00, 0x01F,
        };

        Bitmap bmp(8, 64, 10);
        bmp.decompress(compressed, 9);

//        printf("------- Compressed ---------\n");
//        for (int i = 0; i < (out.p - out.data); i++){
//            printf("%.2x, ", out.data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        uint8_t expected[] = {
            // FILL 32
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

            0xFF, // Magic pixel

            // FILL 31
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        };

//        printf("------- Decompressed ---------\n");
//        for (int i = 0; i < bmp.bmp_size; i++){
//            printf("%.2x ", bmp.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");

        BOOST_CHECK(0 == memcmp(bmp.data_co, expected, sizeof(expected)));
    }


//    {
//        uint8_t compressed[] = {
//            0x60, 0x20, 0xFF, 0x00, 0x00,
//            0x00, 0x00, 0x01F,
//        };

//        Bitmap bmp(8, 64, 10);
//        bmp.decompress(compressed, 9);

//        uint8_t expected[] = {
//            // FILL 32
//            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

//            0xFF, // Magic pixel

//            // FILL 31
//            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        };

//        printf("------- Decompressed ---------\n");
//        for (int i = 0; i < bmp.bmp_size; i++){
//            printf("%.2x ", bmp.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");

//        BOOST_CHECK(0 == memcmp(bmp.data_co, expected, sizeof(expected)));
//    }


    {
        uint8_t compressed[] = {
            // FOM (24 * 8) pix
            0x58, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x0f, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0xb0, 0x01, 0x01,
            0x82, 0x01, 0x02, 0x03, 0x04
        };

        Bitmap bmp(16, 288, 13);
        bmp.decompress(compressed, sizeof(compressed));

        uint8_t expected[] = {
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03, 0x04,
        };

//        printf("------- Decompressed ---------\n");
//        for (int i = 0; i < bmp.bmp_size; i++){
//            if (i % 16 == 0) printf("\n");
//            printf("0x%.2x, ", bmp.data_co[i]);
//        }
//        printf("\n----------------------------\n");
//        printf("\n");

        BOOST_CHECK(0 == memcmp(bmp.data_co, expected, sizeof(expected)));
    }

}
