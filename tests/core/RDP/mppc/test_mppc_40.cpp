/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#define RED_TEST_MODULE TestMPPC40
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/mppc/mppc_40.hpp"

RED_AUTO_TEST_CASE(TestRDP40BlukCompression4)
{
    #include "fixtures/test_mppc_4.hpp"

    rdp_mppc_40_enc mppc_enc;


    RED_CHECK_EQUAL(sizeof(historyBuffer),     RDP_40_HIST_BUF_LEN);
    RED_CHECK_EQUAL(sizeof(outputBufferPlus),  RDP_40_HIST_BUF_LEN + 64);
    RED_CHECK_EQUAL(sizeof(hash_table),        rdp_mppc_40_enc::hash_table_manager::get_table_size());
    RED_CHECK_EQUAL(sizeof(uncompressed_data), 204);
    RED_CHECK_EQUAL(sizeof(compressed_data),   18);


    memcpy(mppc_enc.historyBuffer,           historyBuffer,    RDP_40_HIST_BUF_LEN);
    memcpy(mppc_enc.outputBufferPlus,        outputBufferPlus, RDP_40_HIST_BUF_LEN + 64);
    mppc_enc.historyOffset = 2974;
    mppc_enc.bytes_in_opb  = 21;
    mppc_enc.flags         = 0x20;
    mppc_enc.flagsHold     = 0;
    mppc_enc.first_pkt     = false;
    mppc_enc.hash_tab_mgr.initialize_hash_table(hash_table);

    uint8_t  compressionFlags;
    uint16_t datalen;

    mppc_enc.compress(uncompressed_data, sizeof(uncompressed_data), compressionFlags, datalen,
        rdp_mppc_enc::MAX_COMPRESSED_DATA_SIZE_UNUSED);

    int flags = PACKET_COMPRESSED;

    RED_CHECK_EQUAL(flags, (compressionFlags & PACKET_COMPRESSED));
    RED_CHECK_EQUAL(18,    datalen);
    RED_CHECK_EQUAL(0,     memcmp( compressed_data, mppc_enc.outputBuffer
                                   , mppc_enc.bytes_in_opb));
}

RED_AUTO_TEST_CASE(TestRDP50BlukDecompression6)
{
    #include "fixtures/test_mppc_6.hpp"

    rdp_mppc_40_dec mppc_dec;


    RED_CHECK_EQUAL(sizeof(compressed_data),   2241);
    RED_CHECK_EQUAL(sizeof(uncompressed_data), 4312);


    memcpy(mppc_dec.history_buf, historyBuffer, RDP_40_HIST_BUF_LEN);
    mppc_dec.history_buf_end = mppc_dec.history_buf + 8191;
    mppc_dec.history_ptr     = mppc_dec.history_buf + 4314;

    uint8_t  compressionFlags = 0x60;

    const uint8_t * rdata;
    uint32_t        rlen;

    mppc_dec.decompress(compressed_data, sizeof(compressed_data), compressionFlags, rdata, rlen);

    RED_CHECK_EQUAL(4312, rlen);
    RED_CHECK_EQUAL(0,   memcmp(uncompressed_data, rdata, rlen));
}
