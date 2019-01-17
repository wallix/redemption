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

#define RED_TEST_MODULE TestMPPC61
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/mppc/mppc_61.hpp"

RED_AUTO_TEST_CASE(TestRDP61BlukCompression)
{
    rdp_mppc_61_enc<rdp_mppc_61_enc_hash_based_match_finder> mppc_61_enc;
}


RED_AUTO_TEST_CASE(TestRDP61BlukCompressionSequentialSearchMatchFinder)
{
    rdp_mppc_61_enc_sequential_search_match_finder mppc_enc_match_finder_d;
    rdp_mppc_enc_match_finder & mppc_enc_match_finder = mppc_enc_match_finder_d;


    uint8_t historyBuffer[] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
    };
    mppc_enc_match_finder.find_match(historyBuffer, 4, 4);
    RED_CHECK_EQUAL(0, mppc_enc_match_finder.match_details_stream.get_offset());


    uint8_t historyBuffer1[] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
        'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',

        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
        'i'
    };
    mppc_enc_match_finder.find_match(historyBuffer1, 16, 9);
    RED_CHECK_EQUAL(8, mppc_enc_match_finder.match_details_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(mppc_enc_match_finder.match_details_stream.get_data(),
                                "\x09\x00\x00\x00\x00\x00\x00\x00", 8));


    uint8_t historyBuffer2[] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
        'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',

        'a', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
        'j', 'k'
    };
    mppc_enc_match_finder.find_match(historyBuffer2, 16, 10);
    RED_CHECK_EQUAL(8, mppc_enc_match_finder.match_details_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(mppc_enc_match_finder.match_details_stream.get_data(),
                                "\x09\x00\x01\x00\x02\x00\x00\x00", 8));


    uint8_t historyBuffer3[] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
        'i', 'j', '0', '1', '2', '3', '4', '5',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
        'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',

        'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
        'j', 'k'
    };
    mppc_enc_match_finder.find_match(historyBuffer3, 32, 10);
    RED_CHECK_EQUAL(8, mppc_enc_match_finder.match_details_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(mppc_enc_match_finder.match_details_stream.get_data(),
                                "\x0A\x00\x00\x00\x11\x00\x00\x00", 8));


    uint8_t historyBuffer4[] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
        'i', 'j', '0', '1', '2', '3', '4', '5',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
        'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',

        'a', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
        'j', 'k', 'b'
    };
    mppc_enc_match_finder.find_match(historyBuffer4, 32, 11);
    RED_CHECK_EQUAL(8, mppc_enc_match_finder.match_details_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(mppc_enc_match_finder.match_details_stream.get_data(),
                                "\x09\x00\x01\x00\x12\x00\x00\x00", 8));
}
