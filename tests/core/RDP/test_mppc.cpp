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

#define UNIT_TEST_MODULE TestMPPC
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include <sys/time.h>
#include "core/RDP/mppc_unified_dec.hpp"

RED_AUTO_TEST_CASE(TestMPPC)
{
    // Load compressed_rd5 and decompressed_rd5
    #include "../../fixtures/test_mppc_TestMPPC.hpp"

    const uint8_t * rdata;
    uint32_t        rlen;
    long int dur;

    timeval start_time;
    timeval end_time;


    /* save starting time */
    gettimeofday(&start_time, nullptr);

    for (int x = 0; x < 1000 ; x++){
        rdp_mppc_unified_dec rmppc_d;
        rdp_mppc_dec & rmppc = rmppc_d;

        /* uncompress data */
        RED_CHECK_EQUAL(true, rmppc.decompress(compressed_rd5, sizeof(compressed_rd5), PACKET_COMPRESSED | PACKET_COMPR_TYPE_64K, rdata, rlen));

        RED_CHECK_EQUAL(0, memcmp(decompressed_rd5, rdata, sizeof(decompressed_rd5)));
    }

    /* get end time */
    gettimeofday(&end_time, nullptr);

    /* print time taken */
    dur = ((end_time.tv_sec - start_time.tv_sec) * 1000000) + (end_time.tv_usec - start_time.tv_usec);
    LOG(LOG_INFO, "test_mppc: decompressed data in %ld micro seconds", dur);
}

RED_AUTO_TEST_CASE(TestMPPC_enc)
{
    // Load decompressed_rd5_data
    #include "../../fixtures/test_mppc_TestMPPC_enc.hpp"

    const uint8_t * rdata;
    uint32_t        rlen;

    /* required for timing the test */
    timeval start_time;
    timeval end_time;

    /* setup decoder */
    rdp_mppc_unified_dec rmppc_d;
    rdp_mppc_dec & rmppc = rmppc_d;

    /* setup encoder for RDP 5.0 */
    rdp_mppc_50_enc enc;

    int data_len = sizeof(decompressed_rd5_data);
    LOG(LOG_INFO, "test_mppc_enc: testing with embedded data of %d bytes", data_len);

    /* save starting time */
    gettimeofday(&start_time, nullptr);

    uint8_t  compressionFlags;
    uint16_t datalen;

    enc.compress(decompressed_rd5_data, data_len, compressionFlags, datalen,
        rdp_mppc_enc::MAX_COMPRESSED_DATA_SIZE_UNUSED);

    RED_CHECK(0 != (compressionFlags & PACKET_COMPRESSED));
    RED_CHECK_EQUAL(true, rmppc.decompress(enc.outputBuffer, enc.bytes_in_opb, enc.flags, rdata, rlen));
    RED_CHECK_EQUAL(data_len, rlen);
    RED_CHECK_EQUAL(0, memcmp(decompressed_rd5_data, rdata, rlen));

    /* get end time */
    gettimeofday(&end_time, nullptr);

    /* print time taken */
    long int dur = ((end_time.tv_sec - start_time.tv_sec) * 1000000) + (end_time.tv_usec - start_time.tv_usec);
    LOG(LOG_INFO, "test_mppc_enc: compressed %d bytes in %f seconds\n", data_len, dur / 1000000.0);
}

RED_AUTO_TEST_CASE(TestBitsSerializer)
{
    uint8_t  outputBuffer[256] ={};
    uint8_t  bits_left = 8;
    uint16_t opb_index = 0;

    insert_n_bits_40_50(2, 3, outputBuffer, bits_left, opb_index, sizeof(outputBuffer));
    RED_CHECK_EQUAL(6, bits_left);
    RED_CHECK_EQUAL(0, opb_index);
    RED_CHECK_EQUAL(192, outputBuffer[0] & 0xFF);

    insert_n_bits_40_50(2, 3, outputBuffer, bits_left, opb_index, sizeof(outputBuffer));
    RED_CHECK_EQUAL(4, bits_left);
    RED_CHECK_EQUAL(0, opb_index);
    RED_CHECK_EQUAL(0xF0, outputBuffer[0] & 0xFF);

    insert_n_bits_40_50(2, 3, outputBuffer, bits_left, opb_index, sizeof(outputBuffer));
    RED_CHECK_EQUAL(2, bits_left);
    RED_CHECK_EQUAL(0, opb_index);
    RED_CHECK_EQUAL(0xFc, outputBuffer[0] & 0xFF);

    insert_n_bits_40_50(2, 3, outputBuffer, bits_left, opb_index, sizeof(outputBuffer));
    RED_CHECK_EQUAL(8, bits_left);
    RED_CHECK_EQUAL(1, opb_index);
    RED_CHECK_EQUAL(0xFF, outputBuffer[0] & 0xFF);
}

RED_AUTO_TEST_CASE(TestHashTableManager)
{
    const unsigned int length_of_data_to_sign = 3;
    const unsigned int max_undo_element       = 8;

    typedef uint16_t                                     offset_type;
    typedef rdp_mppc_enc_hash_table_manager<offset_type> hash_table_manager;
    typedef hash_table_manager::hash_type                hash_type;

    hash_table_manager hash_tab_mgr(
        length_of_data_to_sign, max_undo_element);

    uint8_t data[] = "0123456789ABCDEF";

    hash_type   hash;
    hash_type   hash_save;
    offset_type offset;
    offset_type offset_save;


    // Test of insertion (explicit hash value).
    hash_tab_mgr.reset();
    offset = 1;
    hash = hash_tab_mgr.sign(data + offset);
    hash_tab_mgr.update(hash, offset);
    RED_CHECK_EQUAL(offset, hash_tab_mgr.get_offset(hash));


    // Test of insertion (implicit hash value).
    hash_tab_mgr.reset();
    offset = 1;
    hash = hash_tab_mgr.sign(data + offset);
    hash_tab_mgr.update_indirect(data, offset);
    RED_CHECK_EQUAL(offset, hash_tab_mgr.get_offset(hash));


    // Test of undoing last changes.
    hash_tab_mgr.reset();
    offset = 1;
    hash = hash_tab_mgr.sign(data + offset);
    hash_tab_mgr.update_indirect(data, offset);
    RED_CHECK_EQUAL(offset, hash_tab_mgr.get_offset(hash));
    hash_tab_mgr.clear_undo_history();
    hash_save   = hash;
    offset_save = offset;

    offset = 3;
    hash = hash_tab_mgr.sign(data + offset);
    hash_tab_mgr.update_indirect(data, offset);
    RED_CHECK_EQUAL(offset,      hash_tab_mgr.get_offset(hash));
    RED_CHECK_EQUAL(true,        hash_tab_mgr.undo_last_changes());
    RED_CHECK_EQUAL(0,           hash_tab_mgr.get_offset(hash));
    RED_CHECK_EQUAL(offset_save, hash_tab_mgr.get_offset(hash_save));


    // Test of undoing last changes (out of undo buffer).
    hash_tab_mgr.reset();
    for (int i = 0; i < 10; i++)
        hash_tab_mgr.update_indirect(data + i, offset);
    RED_CHECK_EQUAL(false, hash_tab_mgr.undo_last_changes());
}
