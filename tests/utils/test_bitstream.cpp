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
   Author(s): David Fort

   Unit test for bistream
   Using lib boost functions for testing
*/
#define RED_TEST_MODULE BitStream

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/bitstream.hpp"

RED_AUTO_TEST_CASE(TestBitStream) {
    uint8_t data[] = {0x4f, 0x08, 0xff};

    InBitStream bs(data, 3);
    // 0100 1111 0000 1000 1111 1111
    RED_CHECK_EQ(bs.getBits(4), 0x4);
    // 1111 0000 1000 1111 1111
    RED_CHECK_EQ(bs.getBits(4), 0xf);

    // 0000 1000 1111 1111
    RED_CHECK_EQ(bs.getBits(3), 0x0);
    for (int i = 0; i < 100; i++)
        RED_CHECK_EQ(bs.peekBits(1), 0x0); // peek is idempotent

    // 0 1000 1111 1111
    RED_CHECK_EQ(bs.getBits(1), 0x0);
    // 1000 1111 1111
    RED_CHECK_EQ(bs.getBits(2), 0x2);

    // 00 1111 1111
    RED_CHECK_EQ(bs.getBits(1), 0x0);

    // 0 1111 1111
    RED_CHECK_EQ(bs.getBits(4), 0x7);
    // 1 1111
    RED_CHECK_EQ(bs.getBits(5), 0x1f);

    // 01001111 00001000
    // ^------------^
    InBitStream bs2(data, 3);
    RED_CHECK_EQ(bs2.getBits(13), 0x9e1);
    bs2.shift(3);
    RED_CHECK_EQ(bs2.peekBits(8), 0xff);


    // test the sequence involved in rlgr algorithm
    uint8_t rlgr_data[] = { 0x9f, 0x7f, 0xff, 0xff, 0xff, 0xf5, 0x69, 0xa8, 0xa8, 0xc3, 0xc0, 0x07, 0x44, 0x52, 0x02, 0x83 };
    //     0x9f      0x7f     0xff      0xff      0xff      0xf5      0x69     0xa8
    //  1001 1111|0111 1111|1111 1111|1111 1111|1111 1111|1111 0101|0110 1001|1010 1000

    InBitStream bs3(rlgr_data, sizeof(rlgr_data));

    uint32_t v1 = bs3.peekBits(32);
    bs3.shift(0);
    //  001 1111|0111 1111|1111 1111|1111 1111|1111 1111|1111 0101|0110 1001|1010 1000
    //  0011 1110 1111 1111 1111 1111 1111 1111
    bs3.shift(1);
    uint32_t v2 = bs3.peekBits(32);

    RED_CHECK_EQ(v1, 0x9f7fffff);
    RED_CHECK_EQ(v1, 0b10011111011111111111111111111111);
    RED_CHECK_EQ(v2, 0x3effffff);
    RED_CHECK_EQ(v2, 0b00111110111111111111111111111111);

}
