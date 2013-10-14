/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean
   Based on unit tests imported from FreeRDP (test_mppc*)
   from Laxmikant Rashinkar.

   Unit test for MPPC compression
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestMPPC
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include <stdint.h>
#include <sys/time.h>
#include "RDP/mppc.hpp"

BOOST_AUTO_TEST_CASE(TestMPPC)
{
    // Load compressed_rd5 and decompressed_rd5
    #include "../../fixtures/test_mppc_TestMPPC.hpp"

/*
    uint32_t roff;
    uint32_t rlen;
*/
    const uint8_t * rdata;
    uint32_t        rlen;
    long int dur;

    struct timeval start_time;
    struct timeval end_time;


    /* save starting time */
    gettimeofday(&start_time, NULL);

    for (int x = 0; x < 1000 ; x++){
        struct rdp_mppc_dec* rmppc = new rdp_mppc_unified_dec();

        /* uncompress data */
        BOOST_CHECK_EQUAL(true, rmppc->decompress(compressed_rd5, sizeof(compressed_rd5), PACKET_COMPRESSED | PACKET_COMPR_TYPE_64K, rdata, rlen));

        BOOST_CHECK_EQUAL(0, memcmp(decompressed_rd5, rdata, sizeof(decompressed_rd5)));
        delete rmppc;
    }

    /* get end time */
    gettimeofday(&end_time, NULL);

    /* print time taken */
    dur = ((end_time.tv_sec - start_time.tv_sec) * 1000000) + (end_time.tv_usec - start_time.tv_usec);
    LOG(LOG_INFO, "test_mppc: decompressed data in %ld micro seconds", dur);
}

BOOST_AUTO_TEST_CASE(TestMPPC_enc)
{
    // Load decompressed_rd5_data
    #include "../../fixtures/test_mppc_TestMPPC_enc.hpp"

//    enum { BUF_SIZE = 1024 };

/*
    uint32_t roff = 0;
    uint32_t rlen = 0;
*/
    const uint8_t * rdata;
    uint32_t        rlen;

    /* required for timing the test */
    struct timeval start_time;
    struct timeval end_time;

    /* setup decoder */
    struct rdp_mppc_dec * rmppc = new rdp_mppc_unified_dec();

    /* setup encoder for RDP 5.0 */
    struct rdp_mppc_50_enc * enc = new rdp_mppc_50_enc();

    int data_len = sizeof(decompressed_rd5_data);
    LOG(LOG_INFO, "test_mppc_enc: testing with embedded data of %d bytes", data_len);

    /* save starting time */
    gettimeofday(&start_time, NULL);

    uint8_t  compressionFlags;
    uint16_t datalen;

    BOOST_CHECK_EQUAL(true, enc->compress(decompressed_rd5_data, data_len, compressionFlags, datalen));

    BOOST_CHECK(0 != (compressionFlags & PACKET_COMPRESSED));
    BOOST_CHECK_EQUAL(true,
        rmppc->decompress((uint8_t*)enc->outputBuffer, enc->bytes_in_opb, enc->flags, rdata, rlen));
    BOOST_CHECK_EQUAL(data_len, rlen);
    BOOST_CHECK_EQUAL(0, memcmp(decompressed_rd5_data, rdata, rlen));

    /* get end time */
    gettimeofday(&end_time, NULL);

    /* print time taken */
    long int dur = ((end_time.tv_sec - start_time.tv_sec) * 1000000) + (end_time.tv_usec - start_time.tv_usec);
    LOG(LOG_INFO, "test_mppc_enc: compressed %d bytes in %f seconds\n", data_len, (float) (dur) / 1000000.0F);

    delete enc;
    delete rmppc;
}

BOOST_AUTO_TEST_CASE(TestBitsSerializer)
{
    char outputBuffer[256] ={};
    int bits_left = 8;
    int opb_index = 0;
    rdp_mppc_enc::insert_n_bits(2, 3, outputBuffer, bits_left, opb_index);
    BOOST_CHECK_EQUAL(6, bits_left);
    BOOST_CHECK_EQUAL(0, opb_index);
    BOOST_CHECK_EQUAL(192, outputBuffer[0] & 0xFF);

    rdp_mppc_enc::insert_n_bits(2, 3, outputBuffer, bits_left, opb_index);
    BOOST_CHECK_EQUAL(4, bits_left);
    BOOST_CHECK_EQUAL(0, opb_index);
    BOOST_CHECK_EQUAL(0xF0, outputBuffer[0] & 0xFF);

    rdp_mppc_enc::insert_n_bits(2, 3, outputBuffer, bits_left, opb_index);
    BOOST_CHECK_EQUAL(2, bits_left);
    BOOST_CHECK_EQUAL(0, opb_index);
    BOOST_CHECK_EQUAL(0xFc, outputBuffer[0] & 0xFF);

    rdp_mppc_enc::insert_n_bits(2, 3, outputBuffer, bits_left, opb_index);
    BOOST_CHECK_EQUAL(8, bits_left);
    BOOST_CHECK_EQUAL(1, opb_index);
    BOOST_CHECK_EQUAL(0xFF, outputBuffer[0] & 0xFF);
}

BOOST_AUTO_TEST_CASE(TestRDP50BlukCompression2)
{
    #include "../../fixtures/test_mppc_2.hpp"

    rdp_mppc_50_enc * mppc_enc = new rdp_mppc_50_enc();


    BOOST_CHECK_EQUAL(sizeof(historyBuffer),     mppc_enc->buf_len);
    BOOST_CHECK_EQUAL(sizeof(outputBufferPlus),  mppc_enc->buf_len + 64);
    BOOST_CHECK_EQUAL(sizeof(hash_table),        mppc_enc->buf_len * 2);
    BOOST_CHECK_EQUAL(sizeof(uncompressed_data), 4037);
    BOOST_CHECK_EQUAL(sizeof(compressed_data),   3015);


    memcpy(mppc_enc->historyBuffer,    historyBuffer,    mppc_enc->buf_len);
    memcpy(mppc_enc->outputBufferPlus, outputBufferPlus, mppc_enc->buf_len + 64);
    mppc_enc->historyOffset = 61499;
    mppc_enc->buf_len       = 65536;
    mppc_enc->bytes_in_opb  = 2834;
    mppc_enc->flags         = 33;
    mppc_enc->flagsHold     = 0;
    mppc_enc->first_pkt     = 0;
    memcpy(mppc_enc->hash_table,       hash_table,       mppc_enc->buf_len * 2);

    uint8_t  compressionFlags;
    uint16_t datalen;

    mppc_enc->compress(uncompressed_data, sizeof(uncompressed_data), compressionFlags, datalen);

    int flags = PACKET_COMPRESSED;

    BOOST_CHECK_EQUAL(flags, (compressionFlags & PACKET_COMPRESSED));
    BOOST_CHECK_EQUAL(3015,  datalen);
    BOOST_CHECK_EQUAL(0,     memcmp( compressed_data, mppc_enc->outputBuffer
                                   , mppc_enc->bytes_in_opb));

    delete(mppc_enc);
}

BOOST_AUTO_TEST_CASE(TestRDP50BlukCompression3)
{
    #include "../../fixtures/test_mppc_3.hpp"

    rdp_mppc_50_enc * mppc_enc = new rdp_mppc_50_enc();


    BOOST_CHECK_EQUAL(sizeof(historyBuffer),     mppc_enc->buf_len);
    BOOST_CHECK_EQUAL(sizeof(outputBufferPlus),  mppc_enc->buf_len + 64);
    BOOST_CHECK_EQUAL(sizeof(hash_table),        mppc_enc->buf_len * 2);
    BOOST_CHECK_EQUAL(sizeof(uncompressed_data), 12851);
    BOOST_CHECK_EQUAL(sizeof(compressed_data),   8893);


    memcpy(mppc_enc->historyBuffer,    historyBuffer,    mppc_enc->buf_len);
    memcpy(mppc_enc->outputBufferPlus, outputBufferPlus, mppc_enc->buf_len + 64);
    mppc_enc->historyOffset = 0;
    mppc_enc->buf_len       = 65536;
    mppc_enc->bytes_in_opb  = 0;
    mppc_enc->flags         = 0;
    mppc_enc->flagsHold     = 0;
    mppc_enc->first_pkt     = 1;
    memcpy(mppc_enc->hash_table,       hash_table,       mppc_enc->buf_len * 2);

    uint8_t  compressionFlags;
    uint16_t datalen;

    mppc_enc->compress(uncompressed_data, sizeof(uncompressed_data), compressionFlags, datalen);

    int flags = PACKET_COMPRESSED;

    BOOST_CHECK_EQUAL(flags, (compressionFlags & PACKET_COMPRESSED));

    BOOST_CHECK_EQUAL(8893,  datalen);
    BOOST_CHECK_EQUAL(0,     memcmp( compressed_data, mppc_enc->outputBuffer
                                   , mppc_enc->bytes_in_opb));

    delete(mppc_enc);
}
