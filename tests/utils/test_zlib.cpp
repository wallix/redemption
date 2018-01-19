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
   Copyright (C) Wallix 2018
   Author(s): Christophe Grosjean

   Unit test for Zlib library

*/

#define RED_TEST_MODULE TestZlib
#include "system/redemption_unit_tests.hpp"

#include <zlib.h>
#include "utils/log.hpp"

RED_AUTO_TEST_CASE(TestZLIB0)
{
    // Create some easy to compress pattern
    uint8_t uncompressed[70000]; 

    uint8_t pattern[] = {
            'H', 'E', 'R', 'E', ' ', 'I', 'S', ' ', 'T', 'H', 'E', ' ', 'D', 'A', 'T', 'A',
            ' ', 'I', ' ', 'W', 'A', 'N', 'T', ' ', 'T', 'O', ' ', 'C', 'O', 'M', 'P', 'R',
            'E', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', '!',
    };

    size_t q = 0;
    for (; q + sizeof(pattern) < sizeof(uncompressed) ; q += sizeof(pattern)){
        memcpy(&uncompressed[q], pattern, sizeof(pattern));
    }
    memcpy(&uncompressed[q], pattern, sizeof(uncompressed)-q);


    z_stream strm;
    const size_t CHUNK = 16384; // magic number, 16384 is minimal value recommanded by zlib
    const size_t INCHUNK = 3265; // magic numbers, whatever could do
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    int ret = deflateInit(&strm, 1);
    if (ret != Z_OK){
        RED_ERROR("deflateInit failed");
    }

    /* compress until end of data */
    size_t total_compressed_size = 0;
    size_t qlen = 1;
    strm.avail_in = 0;
    for ( q = 0 ; qlen - strm.avail_in ; q += qlen - strm.avail_in) {
        strm.next_in = &uncompressed[q];
        strm.avail_in = qlen = (q + INCHUNK < sizeof(uncompressed))?INCHUNK:sizeof(uncompressed)-q;
        do {
            strm.avail_out = CHUNK;
            strm.next_out = &out[0];
            ret = deflate(&strm, Z_NO_FLUSH);
            total_compressed_size += CHUNK-strm.avail_out;
            // or copy result
        } while (CHUNK != strm.avail_out);
    }
    do {
        strm.avail_out = CHUNK;
        strm.next_out = &out[0];
        ret = deflate(&strm, Z_FINISH);
        total_compressed_size += CHUNK-strm.avail_out;
    } while (CHUNK != strm.avail_out);
    RED_CHECK_EQUAL(total_compressed_size, 482);
}
