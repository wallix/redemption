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
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan,
              Jennifer Inthavong
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test for Lightweight UTF library

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
    const size_t CHUNK = 16384;
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
    q = 0;
    do {
       size_t datasize = (q + CHUNK > sizeof(uncompressed))?sizeof(uncompressed)-q:CHUNK;
       strm.avail_in = datasize;
       strm.next_in = &uncompressed[q];
        printf("datasize=%zu\n", datasize);
    
        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            // Output space is provided to deflate() by setting 
            // avail_out to the number of available output bytes 
            // and next_out to a pointer to that space.
            strm.avail_out = CHUNK;
            strm.next_out = &out[0];
            ret = deflate(&strm, true||(datasize != CHUNK) ? Z_FINISH : Z_NO_FLUSH);
            size_t have = CHUNK - strm.avail_out;
            printf("have=%zu avail_out=%zu\n", have, strm.avail_out);
            total_compressed_size += CHUNK-strm.avail_out;
        } while (CHUNK != strm.avail_out);
    } while (0);
    RED_CHECK_EQUAL(total_compressed_size, 170);
}
