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
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include <zlib.h>
#include "utils/zlib.hpp"

RED_AUTO_TEST_CASE(TestZLIB0)
{
    size_t total_size = 0;
    uint8_t all_out[32768];

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
    strm.zalloc = nullptr;
    strm.zfree = nullptr;
    strm.opaque = nullptr;
    int ret = deflateInit(&strm, 9);
    RED_REQUIRE_EQ(ret, Z_OK);

    /* compress until end of data */
    size_t total_compressed_size = 0;
    size_t qlen = 1;
    strm.avail_in = 0;
    size_t last_avail_out;
    for ( q = 0 ; qlen - strm.avail_in ; q += qlen - strm.avail_in) {
        strm.next_in = &uncompressed[q];
        strm.avail_in = qlen = (q + INCHUNK < sizeof(uncompressed))?INCHUNK:sizeof(uncompressed)-q;
        do {
            strm.avail_out = CHUNK;
            strm.next_out = &out[0];
            last_avail_out = strm.avail_out;
            ret = deflate(&strm, Z_NO_FLUSH);
            total_compressed_size += CHUNK-strm.avail_out;

            memcpy(&all_out[total_size], strm.next_out - last_avail_out + strm.avail_out, last_avail_out-strm.avail_out);
            total_size += last_avail_out - strm.avail_out;
            // or copy result
        } while (CHUNK != strm.avail_out);
    }
    do {
        strm.avail_out = CHUNK;
        strm.next_out = &out[0];
        last_avail_out = strm.avail_out;
        ret = deflate(&strm, Z_FINISH);
        total_compressed_size += last_avail_out-strm.avail_out;

        memcpy(&all_out[total_size], strm.next_out - last_avail_out + strm.avail_out, last_avail_out-strm.avail_out);
        total_size += last_avail_out - strm.avail_out;

    } while (CHUNK != strm.avail_out);

    ret = deflateEnd(&strm);

    RED_CHECK_EQUAL(total_compressed_size, 262);

    // hexdump(&all_out[0], total_size);

    RED_CHECK_EQUAL(total_size, 262);
}


RED_AUTO_TEST_CASE(TestZLIB1)
{
    // Create some easy to compress pattern
    size_t total_size = 0;
    uint8_t all_out[32768];
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
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = nullptr;
    strm.zfree = nullptr;
    strm.opaque = nullptr;
    int ret = deflateInit(&strm, 9);
    RED_REQUIRE_EQ(ret, Z_OK);

    /* compress until end of data */
    size_t total_compressed_size = 0;
    strm.avail_in = sizeof(uncompressed);
    strm.next_in = &uncompressed[0];
    strm.avail_out = CHUNK;
    strm.next_out = &out[0];

    size_t last_avail_out = 0;
    while (strm.avail_in) {
        last_avail_out = strm.avail_out;
        if (strm.avail_out == 0){
            strm.avail_out = CHUNK;
            strm.next_out = &out[0];
        }
        ret = deflate(&strm, Z_NO_FLUSH);
        total_compressed_size += last_avail_out-strm.avail_out;
        memcpy(&all_out[total_size], strm.next_out - last_avail_out + strm.avail_out, last_avail_out - strm.avail_out);
        total_size += last_avail_out - strm.avail_out;
    }
    do {
        strm.avail_out = CHUNK;
        strm.next_out = &out[0];
        ret = deflate(&strm, Z_FINISH);
        total_compressed_size += CHUNK-strm.avail_out;
        memcpy(&all_out[total_size], strm.next_out - last_avail_out + strm.avail_out, last_avail_out - strm.avail_out);
        total_size += last_avail_out - strm.avail_out;
    } while (CHUNK != strm.avail_out);
    RED_CHECK_EQUAL(ret, Z_STREAM_END);

    ret = deflateEnd(&strm);

    // hexdump(&all_out[0], total_size);

    RED_CHECK_EQUAL(total_size, 262);
    RED_CHECK_EQUAL(total_compressed_size, 262);
}

RED_AUTO_TEST_CASE(TestZLIB3)
{
    // Create some easy to compress pattern
    size_t total_size = 0;
    uint8_t all_out[32768];
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

    Zcompressor<> z;
    const size_t step = 1024;

    for (q = 0 ; q < sizeof(uncompressed) ; q += z.update(&uncompressed[q], std::min(step,sizeof(uncompressed)-q))){
        if (z.full()) {
            total_size += z.flush_ready(&all_out[total_size], sizeof(all_out)-total_size);
        }
    }
    while (!z.finish() || z.available()){
        total_size += z.flush_ready(&all_out[total_size], sizeof(all_out)-total_size);
    }

    // hexdump(&all_out[0], total_size);
    RED_CHECK_EQUAL(total_size, 262);

    Zdecompressor<> unz;
    const size_t step2 = 1024;

    uint8_t decompressed[sizeof(uncompressed)] = {};
    size_t inflated_size = 0;
    const size_t compressed_size = total_size;

    for (q = 0 ; q < compressed_size ; q += unz.update(&all_out[q], std::min(step2, compressed_size-q))){
        if (unz.full()) {
            inflated_size += unz.flush_ready(&decompressed[inflated_size], sizeof(decompressed)-inflated_size);
        }
    }
    while (!unz.finish() || unz.available()){
        inflated_size += unz.flush_ready(&decompressed[inflated_size], sizeof(decompressed)-inflated_size);
    }
    RED_CHECK_EQUAL(inflated_size, 70000);

    RED_CHECK(0 == memcmp(decompressed, uncompressed, 70000));
}
