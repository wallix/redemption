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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean
*/

#define RED_TEST_MODULE TestVNCHextileEncoder
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/vnc/encoder/hextile.cpp"


class BlockWrap
{
    const_bytes_view & t;
    size_t pos;
public:
    BlockWrap(const_bytes_view & t) : t(t), pos(0) {}

    size_t operator()(byte_ptr buffer, size_t len)
    {
        const size_t available = this->t.size() - this->pos;
        if (len >= available){
            std::memcpy(&buffer[0], &this->t[this->pos], available);
            this->pos += available;
            return available;
        }
        std::memcpy(&buffer[0], &this->t[this->pos], len);
        this->pos += len;
        return len;
    }
};


RED_AUTO_TEST_CASE(TestHextile1)
{
    VNC::Encoder::Hextile encoder(BitsPerPixel{16}, BytesPerPixel{2}, {0, 0, 44, 19}, VNCVerbose::basic_trace);
    RED_CHECK_EQUAL(Rect( 0,  0, 16, 16), encoder.current_tile());
    RED_CHECK_EQUAL(true, encoder.next_tile());
    RED_CHECK_EQUAL(Rect(16,  0, 16, 16), encoder.current_tile());
    RED_CHECK_EQUAL(true, encoder.next_tile());
    RED_CHECK_EQUAL(Rect(32,  0, 12, 16), encoder.current_tile());
    RED_CHECK_EQUAL(true, encoder.next_tile());
    RED_CHECK_EQUAL(Rect( 0, 16, 16, 3), encoder.current_tile());
    RED_CHECK_EQUAL(true, encoder.next_tile());
    RED_CHECK_EQUAL(Rect(16, 16, 16, 3), encoder.current_tile());
    RED_CHECK_EQUAL(true, encoder.next_tile());
    RED_CHECK_EQUAL(Rect(32, 16, 12, 3), encoder.current_tile());
    RED_CHECK_EQUAL(false, encoder.next_tile());
}

RED_AUTO_TEST_CASE(TestHextile2)
{
    VNC::Encoder::Hextile encoder(BitsPerPixel{16}, BytesPerPixel{2}, {0, 0, 48, 19}, VNCVerbose::basic_trace);
    RED_CHECK_EQUAL(Rect( 0,  0, 16, 16), encoder.current_tile());
    RED_CHECK_EQUAL(true, encoder.next_tile());
    RED_CHECK_EQUAL(Rect(16,  0, 16, 16), encoder.current_tile());
    RED_CHECK_EQUAL(true, encoder.next_tile());
    RED_CHECK_EQUAL(Rect(32,  0, 16, 16), encoder.current_tile());
    RED_CHECK_EQUAL(true, encoder.next_tile());
    RED_CHECK_EQUAL(Rect( 0, 16, 16, 3), encoder.current_tile());
    RED_CHECK_EQUAL(true, encoder.next_tile());
    RED_CHECK_EQUAL(Rect(16, 16, 16, 3), encoder.current_tile());
    RED_CHECK_EQUAL(true, encoder.next_tile());
    RED_CHECK_EQUAL(Rect(32, 16, 16, 3), encoder.current_tile());
    RED_CHECK_EQUAL(false, encoder.next_tile());


//    encoder->consume(buf, drawable);

}


//Hextile encoding type=1a data=1353
//Rect=Rect(0 0 1920 34) Tile = Rect(48 0 16 16) cx_remain=1872, cy_remain=34

//background tile
//SubrectsColoured
//Smalltile (0,0,1,4)
//Smalltile (0,0,1,4) done
//SubrectsColoured
//Smalltile (1,0,1,2)
//Smalltile (1,0,1,2) done
//SubrectsColoured
//Smalltile (2,0,1,1)
//Smalltile (2,0,1,1) done
//SubrectsColoured
//Smalltile (3,0,1,1)
//Smalltile (3,0,1,1) done
//SubrectsColoured
//Smalltile (4,0,1,1)
//Smalltile (4,0,1,1) done
//SubrectsColoured

//0000 1a : 16 + 8 + 2 =
//      2   BackgroundSpecified
//      8   AnySubrects
//      16  SubrectsColoured

// Background = 82 08
// nbSubrects = 7a = 122

// Subrect Colored = e4 18 Rect( 0,  0, 1, 4) 00 03
// Subrect Colored = 04 19 Rect( 1,  0, 1, 2) 10 01
// Subrect Colored = 05 19 Rect( 2,  0, 1, 1) 20 00
// Subrect Colored = 04 19 Rect( 3,  0, 1, 1) 30 00
// Subrect Colored = c4 18 Rect( 4,  0, 1, 1) 40 00
// Subrect Colored = c3 10 Rect( 5,  0, 1, 1) 50 00
// Subrect Colored = a3 10 Rect( 6,  0, 2, 2) 60 11
// Subrect Colored = 83 10 Rect( 8,  0, 1, 1) 80 00
// Subrect Colored = a3 10 Rect( 9,  0, 1, 2) 90 01
// Subrect Colored = 83 10 Rect(10,  0, 1, 1) a0 00

// Subrect Colored = 83 08 Rect(11,  0, 1, 1) b0 00
// Subrect Colored = 62 08 Rect(12,  0, 2, 1) c0 10
// Subrect Colored = 83 08 Rect(15,  0, 1, 1) f0 00
// Subrect Colored = 04 19 Rect( 2,  1, 1, 1) 21 00
// Subrect Colored = e4 18 Rect( 3,  1, 1, 1) 31 00
// Subrect Colored = c4 10 Rect( 4,  1, 1, 1) 41 00
// Subrect Colored = a3 10 Rect( 5,  1, 6, 1) 51 50
// Subrect Colored = 83 10 Rect(11,  1, 1, 1) b1 00
// Subrect Colored = 83 08 Rect(14,  1, 1, 5) e1 04
// Subrect Colored = a3 10 Rect(15,  1, 1, 6) f1 05

// Subrect Colored = e4 18 Rect( 1,  2, 2, 1) 12 10
// Subrect Colored = c4 18 Rect( 3,  2, 1, 1) 32 00
// Subrect Colored = a3 10 Rect( 4,  2, 1, 3) 42 03
// Subrect Colored = 83 10 Rect( 5,  2, 1, 2) 52 01
// Subrect Colored = 83 08 Rect( 6,  2, 3, 1) 62 20
// Subrect Colored = 83 10 Rect( 9,  2, 2, 1) 92 10
// Subrect Colored = 83 08 Rect(11,  2, 1, 1) b2 00
// Subrect Colored = 62 08 Rect(12,  2, 2, 2) c2 11
// Subrect Colored = e4 18 Rect( 1,  3, 1, 1) 13 00
// Subrect Colored = c4 18 Rect( 2,  3, 1, 1) 23 00

// Subrect Colored = c3 10 Rect( 3,  3, 1, 1) 33 00
// Subrect Colored = 83 08 Rect( 6,  3, 2, 2) 63 11
// Subrect Colored = 83 08 Rect( 9,  3, 2, 1) 93 10
// Subrect Colored = e4 10 Rect( 0,  4, 1, 1) 04 00
// Subrect Colored = c4 10 Rect( 3,  4, 1, 1) 14 00
// Subrect Colored = c3 10 Rect( 2,  4, 1, 6) 24 05
// Subrect Colored = a3 10 Rect( 3,  4, 2, 2) 34 11
// Subrect Colored = a3 08 Rect( 5,  4, 1, 1) 54 00
// Subrect Colored = 83 08 Rect(10,  4, 1, 1) a4 00
// Subrect Colored = c4 10 Rect( 0,  5, 1, 1) 05 00

// Subrect Colored = c3 10 Rect( 1,  5, 2, 5) 15 14
// Subrect Colored = a3 10 Rect( 5,  5, 1, 3) 55 02
// Subrect Colored = 83 08 Rect( 6,  5, 1, 1) 65 00
// Subrect Colored = 62 08 Rect( 8,  5, 2, 1) 85 10
// Subrect Colored = 62 08 Rect(12,  5, 1, 2) c5 01
// Subrect Colored = c3 10 Rect( 0,  6, 5, 1) 06 40
// Subrect Colored = 62 08 Rect( 7,  6, 1, 3) 76 02
// Subrect Colored = 62 00 Rect( 8,  6, 1, 1) 86 00
// Subrect Colored = 62 08 Rect( 9,  6, 5, 1) 96 40
// Subrect Colored = e4 10 Rect( 0,  7, 1, 1) 07 00

// Subrect Colored = c3 10 Rect( 3,  7, 2, 1) 37 10
// Subrect Colored = 62 08 Rect( 8,  7, 2, 1) 87 10
// Subrect Colored = a3 10 Rect(14,  7, 1, 1) e7 00
// Subrect Colored = c3 10 Rect(15,  7, 1, 1) f7 00
// Subrect Colored = e4 18 Rect( 0,  8, 1, 1) 08 00
// Subrect Colored = e4 18 Rect( 3,  8, 1, 1) 38 00
// Subrect Colored = c3 10 Rect( 4,  8, 1, 1) 48 00
// Subrect Colored = 62 08 Rect( 6,  8, 2, 1) 68 10
// Subrect Colored = a3 10 Rect(10,  8, 1, 1) a8 00
// Subrect Colored = 41 00 Rect(12,  8, 1, 1) c8 00

// Subrect Colored = c3 10 Rect(14,  8, 1, 2) e8 01
// Subrect Colored = a3 10 Rect(15,  8, 1, 1) f8 00
// Subrect Colored = c3 10 Rect( 0,  9, 3, 1) 09 30
// Subrect Colored = a3 08 Rect( 4,  9, 1, 1) 49 00
// Subrect Colored = 62 08 Rect( 5,  9, 2, 1) 59 10
// Subrect Colored = c3 10 Rect( 8,  9, 1, 2) 89 01
// Subrect Colored = a3 10 Rect( 9,  9, 1, 2) 99 01
// Subrect Colored = 62 08 Rect(11,  9, 1, 1) b9 00
// Subrect Colored = a3 10 Rect(13,  9, 1, 2) d9 01
// Subrect Colored = e4 18 Rect(15,  9, 1, 1) f9 00

// Subrect Colored = c3 10 Rect( 0, 10, 2, 1) 0a 10
// Subrect Colored = a3 10 Rect( 2, 10, 1, 2) 2a 01
// Subrect Colored = 62 08 Rect( 4, 10, 1, 1) 4a 00
// Subrect Colored = 62 00 Rect( 5, 10, 1, 1) 5a 10
// Subrect Colored = 62 08 Rect( 7, 10, 1, 2) 7a 01
// Subrect Colored = 41 00 Rect(10, 10, 1, 2) aa 01
// Subrect Colored = 62 00 Rect(11, 10, 1, 1) ba 00
// Subrect Colored = c3 10 Rect(12, 10, 1, 1) ca 00
// Subrect Colored = a3 10 Rect(14, 10, 1, 1) ea 00
// Subrect Colored = 04 19 Rect(15, 10, 1, 1) fa 00

// Subrect Colored = e4 18 Rect( 0, 11, 1, 1) 0b 00
// Subrect Colored = c3 10 Rect( 1, 11, 1, 1) 1b 00
// Subrect Colored = 62 00 Rect( 4, 11, 1, 1) 4b 00
// Subrect Colored = 41 00 Rect( 5, 11, 1, 1) 5b 00
// Subrect Colored = 62 00 Rect( 6, 11, 1, 1) 6b 00
// Subrect Colored = 62 08 Rect(11, 11, 1, 1) bb 00
// Subrect Colored = e4 18 Rect(12, 11, 1, 1) cb 00
// Subrect Colored = 62 08 Rect(14, 11, 1, 1) eb 00
// Subrect Colored = 25 21 Rect(15, 11, 1, 1) fb 00
// Subrect Colored = e4 10 Rect( 0, 12, 1, 1) 0c 00

// Subrect Colored = e3 10 Rect( 1, 12, 1, 1) 1c 00
// Subrect Colored = a3 08 Rect( 2, 12, 1, 2) 2c 01
// Subrect Colored = 61 00 Rect( 4, 12, 1, 1) 4c 00
// Subrect Colored = 62 00 Rect( 5, 12, 1, 1) 5c 00
// Subrect Colored = a3 08 Rect( 9, 12, 1, 2) 9c 01
// Subrect Colored = 62 00 Rect(10, 12, 1, 1) ac 00
// Subrect Colored = e4 10 Rect(12, 12, 1, 1) cc 00
// Subrect Colored = a2 08 Rect(14, 12, 1, 1) dc 00
// Subrect Colored = c3 10 Rect(14, 12, 1, 1) ec 00
// Subrect Colored = c7 31 Rect(15, 12, 1, 1) fc 00

// Subrect Colored = c3 10 Rect( 0, 13, 2, 1) 0d 10
// Subrect Colored = 62 00 Rect( 4, 13, 1, 2) 4d 01
// Subrect Colored = a3 08 Rect( 7, 13, 1, 1) 7d 00
// Subrect Colored = a3 08 Rect(12, 13, 2, 1) cd 10
// Subrect Colored = 45 21 Rect(14, 13, 1, 1) ed 00
// Subrect Colored = 8a 4a Rect(15, 14, 1, 2) fd 01
// Subrect Colored = 18 be Rect( 0, 14, 1, 1) 0e 00
// Subrect Colored = 34 a5 Rect( 1, 14, 1, 1) 1e 00
// Subrect Colored = 0c 5b Rect( 2, 14, 1, 1) 2e 00
// Subrect Colored = a2 08 Rect( 6, 14, 2, 1) 6e 10

// Subrect Colored = c3 10 Rect(13, 14, 1, 1) de 00
// Subrect Colored = a7 29 Rect(14, 14, 1, 2) ee 01
// Subrect Colored = ef 7b Rect( 0, 15, 1, 1) 0f 00
// Subrect Colored = 30 84 Rect( 1, 15, 1, 1) 1f 00
// Subrect Colored = 55 a5 Rect( 2, 15, 1, 1) 2f 00
// Subrect Colored = 18 be Rect( 3, 15, 1, 1) 3f 00
// Subrect Colored = 14 9d Rect( 9, 13, 1, 1) 4f 00
// Subrect Colored = cb 52 Rect( 5, 15, 1, 1) 5f 00
// Subrect Colored = c3 10 Rect(10, 15, 1, 1) af 10
// Subrect Colored = a3 08 Rect(12, 15, 1, 1) cf 00

// Subrect Colored = e4 10 Rect(13, 15, 1, 1) df 00
// Subrect Colored = 49 42 Rect(15, 15, 1, 1) ff 00

// 01
// a3 08 e4 10 e8 31 4a 42 49 42 0c 5b 6a 4a 4d 63
// 8e 6b 71 8c 51 84 71 8c b2 94 b2 8c d2 94 50 84
// c3 08 25 19 29 3a 49 42 8a 4a 2d 5b ab 52 4d 63
// cf 73 72 8c 51 84 71 8c b2 8c 92 8c d3 94 92 8c
// a3 08 25 19 49 42 29 3a 8a 4a 0c 5b cb 52 2d 63
// af 73 51 84 51 84 92 8c b2 94 91 8c 91 8c 71 84
// a3 08 25 19 49 42 29 3a cb 52 cb 52 cb 52 4d 63
// 30 84 51 84 30 84 30 84 51 84 30 84 30 84 71 8c
// e4 10 86 21 6a 42 6a 42 0c 5b ab 4a ec 5a af 73
// 92 8c 71 8c 50 84 10 7c 30 84 10 7c 0f 7c 51 84
// e4 10 c7 29 69 42 8a 4a ec 52 ab 4a 0c 5b cf 73
// 72 8c 71 8c 71 8c 10 7c 30 84 51 84 ef 7b 30 84
// e4 10 29 3a 6a 42 8a 4a ab 52 0c 5b 2d 63 af 73
// 92 8c 92 8c 71 8c ef 7b 30 84 b2 94 71 8c 92 8c
// 24 19 cb 52 cb 52 ab 4a ab 52 8e 6b af 73 cf 73
// 71 8c 92 8c b2 94 50 84 92 8c 34 a5 b2 94 b2 94
// 86 29 49 42 cb 52 6a 42 2d 63 6e 6b ae 6b 6d 63
// 10 7c d3 94 92 8c d2 94 14 a5 10 84 d3 9c 13 9d
// 49 42 aa 4a 8a 4a aa 4a ec 5a 6e 6b ae 6b ef 7b
// 71 8c f4 9c b2 94 92 8c d3 94 10 84 91 8c cf 7b
// 49 42 8a 4a 69 42 0c 5b 0c 5b 8e 6b 8e 6b 30 7c
// 34 9d f3 94 d3 94 34 a5 f3 9c 8e 6b ae 73 4c 63
// 49 42 8a 4a 8a 4a eb 5a 4d 63 ae 73 cf 73 51 84
// 14 9d 92 8c 92 8c d2 94 30 84 0c 5b ae 73 ef 7b
// cb 52 eb 5a eb 52 8a 4a 4d 63 ae 6b 51 84 92 8c
// 10 7c 10 7c ef 7b 0c 5b eb 5a 6d 6b b2 94 91 8c
// ec 5a 0c 5b 2c 5b aa 52 4d 63 6d 63 ef 7b 10 7c
// ef 73 ae 73 ae 73 0c 5b 6d 6b 10 84 d3 9c 92 94
// ab 52 cb 52 0c 5b 2c 5b 8e 6b 6d 63 8e 6b cf 73
// 51 84 ef 7b 10 7c 30 84 b2 94 71 8c 71 8c b2 94
// ec 5a eb 5a aa 52 6d 63 6d 6b ae 6b ae 6b 71 8c
// 71 84 91 8c b2 94 30 84 b2 94 92 94 b2 9c 14 a5

// 01 d2 8c
//03f0 f3 94 14 9d f3 94 b2 8c f3 94 55 a5 75 a5 96 ad
//Smalltile (5,0,1,1)
//Smalltile (5,0,1,1) done
//SubrectsColoured
//Smalltile (6,0,2,2)
//Smalltile (6,0,2,2) done
//SubrectsColoured
//Smalltile (8,0,1,1)
//Smalltile (8,0,1,1) done
//SubrectsColoured
//Smalltile (9,0,1,2)
//Smalltile (9,0,1,2) done
//SubrectsColoured
//Smalltile (10,0,1,1)
//Smalltile (10,0,1,1) done
//SubrectsColoured
//Smalltile (11,0,1,1)
//Smalltile (11,0,1,1) done
//SubrectsColoured
//Smalltile (12,0,2,1)
//Smalltile (12,0,2,1) done
//SubrectsColoured
//Smalltile (15,0,1,1)
//Smalltile (15,0,1,1) done
//SubrectsColoured
//Smalltile (2,1,1,1)
//Smalltile (2,1,1,1) done
//SubrectsColoured
//Smalltile (3,1,1,1)
//Smalltile (3,1,1,1) done
//SubrectsColoured
//Smalltile (4,1,1,1)
//Smalltile (4,1,1,1) done
//SubrectsColoured
//Smalltile (5,1,6,1)
//Smalltile (5,1,6,1) done
//SubrectsColoured
//Smalltile (11,1,1,1)
//Smalltile (11,1,1,1) done
//SubrectsColoured
//Smalltile (14,1,1,5)
//Smalltile (14,1,1,5) done
//SubrectsColoured
//Smalltile (15,1,1,6)
//Smalltile (15,1,1,6) done
//SubrectsColoured
//Smalltile (1,2,2,1)
//Smalltile (1,2,2,1) done
//SubrectsColoured
//Smalltile (3,2,1,1)
//Smalltile (3,2,1,1) done
//SubrectsColoured
//Smalltile (4,2,1,4)
//Smalltile (4,2,1,4) done
//SubrectsColoured
//Smalltile (5,2,1,2)
//Smalltile (5,2,1,2) done
//SubrectsColoured
//Smalltile (6,2,3,1)
//Smalltile (6,2,3,1) done
//SubrectsColoured
//Smalltile (9,2,2,1)
//Smalltile (9,2,2,1) done
//SubrectsColoured
//Smalltile (11,2,1,1)
//Smalltile (11,2,1,1) done
//SubrectsColoured
//Smalltile (12,2,2,2)
//Smalltile (12,2,2,2) done
//SubrectsColoured
//Smalltile (1,3,1,1)
//Smalltile (1,3,1,1) done
//SubrectsColoured
//Smalltile (2,3,1,1)
//Smalltile (2,3,1,1) done
//SubrectsColoured
//Smalltile (3,3,1,1)
//Smalltile (3,3,1,1) done
//SubrectsColoured
//Smalltile (6,3,2,2)
//Smalltile (6,3,2,2) done
//SubrectsColoured
//Smalltile (9,3,2,1)
//Smalltile (9,3,2,1) done
//SubrectsColoured
//Smalltile (0,4,1,1)
//Smalltile (0,4,1,1) done
//SubrectsColoured
//Smalltile (1,4,1,1)
//Smalltile (1,4,1,1) done
//SubrectsColoured
//Smalltile (2,4,1,6)
//Smalltile (2,4,1,6) done
//SubrectsColoured
//Smalltile (3,4,2,2)
//Smalltile (3,4,2,2) done
//SubrectsColoured
//Smalltile (5,4,1,1)
//Smalltile (5,4,1,1) done
//SubrectsColoured
//Smalltile (10,4,1,1)
//Smalltile (10,4,1,1) done
//SubrectsColoured
//Smalltile (0,5,1,1)
//Smalltile (0,5,1,1) done
//SubrectsColoured
//Smalltile (1,5,2,5)
//Smalltile (1,5,2,5) done
//SubrectsColoured
//Smalltile (5,5,1,3)
//Smalltile (5,5,1,3) done
//SubrectsColoured
//Smalltile (6,5,1,1)
//Smalltile (6,5,1,1) done
//SubrectsColoured
//Smalltile (8,5,2,1)
//Smalltile (8,5,2,1) done
//SubrectsColoured
//Smalltile (12,5,1,2)
//Smalltile (12,5,1,2) done
//SubrectsColoured
//Smalltile (0,6,5,1)
//Smalltile (0,6,5,1) done
//SubrectsColoured
//Smalltile (7,6,1,3)
//Smalltile (7,6,1,3) done
//SubrectsColoured
//Smalltile (8,6,1,1)
//Smalltile (8,6,1,1) done
//SubrectsColoured
//Smalltile (9,6,5,1)
//Smalltile (9,6,5,1) done
//SubrectsColoured
//Smalltile (0,7,1,1)
//Smalltile (0,7,1,1) done
//SubrectsColoured
//Smalltile (3,7,2,1)
//Smalltile (3,7,2,1) done
//SubrectsColoured
//Smalltile (8,7,2,1)
//Smalltile (8,7,2,1) done
//SubrectsColoured
//Smalltile (14,7,1,1)
//Smalltile (14,7,1,1) done
//SubrectsColoured
//Smalltile (15,7,1,1)
//Smalltile (15,7,1,1) done
//SubrectsColoured
//Smalltile (0,8,1,1)
//Smalltile (0,8,1,1) done
//SubrectsColoured
//Smalltile (3,8,1,1)
//Smalltile (3,8,1,1) done
//SubrectsColoured
//Smalltile (4,8,1,1)
//Smalltile (4,8,1,1) done
//SubrectsColoured
//Smalltile (6,8,2,1)
//Smalltile (6,8,2,1) done
//SubrectsColoured
//Smalltile (10,8,1,1)
//Smalltile (10,8,1,1) done
//SubrectsColoured
//Smalltile (12,8,1,1)
//Smalltile (12,8,1,1) done
//SubrectsColoured
//Smalltile (14,8,1,2)
//Smalltile (14,8,1,2) done
//SubrectsColoured
//Smalltile (15,8,1,1)
//Smalltile (15,8,1,1) done
//SubrectsColoured
//Smalltile (0,9,4,1)
//Smalltile (0,9,4,1) done
//SubrectsColoured
//Smalltile (4,9,1,1)
//Smalltile (4,9,1,1) done
//SubrectsColoured
//Smalltile (5,9,2,1)
//Smalltile (5,9,2,1) done
//SubrectsColoured
//Smalltile (8,9,1,2)
//Smalltile (8,9,1,2) done
//SubrectsColoured
//Smalltile (9,9,1,2)
//Smalltile (9,9,1,2) done
//SubrectsColoured
//Smalltile (11,9,1,1)
//Smalltile (11,9,1,1) done
//SubrectsColoured
//Smalltile (13,9,1,2)
//Smalltile (13,9,1,2) done
//SubrectsColoured
//Smalltile (15,9,1,1)
//Smalltile (15,9,1,1) done
//SubrectsColoured
//Smalltile (0,10,2,1)
//Smalltile (0,10,2,1) done
//SubrectsColoured
//Smalltile (2,10,1,2)
//Smalltile (2,10,1,2) done
//SubrectsColoured
//Smalltile (4,10,1,1)
//Smalltile (4,10,1,1) done
//SubrectsColoured
//Smalltile (5,10,2,1)
//Smalltile (5,10,2,1) done
//SubrectsColoured
//Smalltile (7,10,1,2)
//Smalltile (7,10,1,2) done
//SubrectsColoured
//Smalltile (10,10,1,2)
//Smalltile (10,10,1,2) done
//SubrectsColoured
//Smalltile (11,10,1,1)
//Smalltile (11,10,1,1) done
//SubrectsColoured
//Smalltile (12,10,1,1)
//Smalltile (12,10,1,1) done
//SubrectsColoured
//Smalltile (14,10,1,1)
//Smalltile (14,10,1,1) done
//SubrectsColoured
//Smalltile (15,10,1,1)
//Smalltile (15,10,1,1) done
//SubrectsColoured
//Smalltile (0,11,1,1)
//Smalltile (0,11,1,1) done
//SubrectsColoured
//Smalltile (1,11,1,1)
//Smalltile (1,11,1,1) done
//SubrectsColoured
//Smalltile (4,11,1,1)
//Smalltile (4,11,1,1) done
//SubrectsColoured
//Smalltile (5,11,1,1)
//Smalltile (5,11,1,1) done
//SubrectsColoured
//Smalltile (6,11,1,1)
//Smalltile (6,11,1,1) done
//SubrectsColoured
//Smalltile (11,11,1,1)
//Smalltile (11,11,1,1) done
//SubrectsColoured
//Smalltile (12,11,1,1)
//Smalltile (12,11,1,1) done
//SubrectsColoured
//Smalltile (14,11,1,1)
//Smalltile (14,11,1,1) done
//SubrectsColoured
//Smalltile (15,11,1,1)
//Smalltile (15,11,1,1) done
//SubrectsColoured
//Smalltile (0,12,1,1)
//Smalltile (0,12,1,1) done
//SubrectsColoured
//Smalltile (1,12,1,1)
//Smalltile (1,12,1,1) done
//SubrectsColoured
//Smalltile (2,12,1,2)
//Smalltile (2,12,1,2) done
//SubrectsColoured
//Smalltile (4,12,1,1)
//Smalltile (4,12,1,1) done
//SubrectsColoured
//Smalltile (5,12,1,1)
//Smalltile (5,12,1,1) done
//SubrectsColoured
//Smalltile (9,12,1,2)
//Smalltile (9,12,1,2) done
//SubrectsColoured
//Smalltile (10,12,1,1)
//Smalltile (10,12,1,1) done
//SubrectsColoured
//Smalltile (12,12,1,1)
//Smalltile (12,12,1,1) done
//SubrectsColoured
//Smalltile (13,12,1,1)
//Smalltile (13,12,1,1) done
//SubrectsColoured
//Smalltile (14,12,1,1)
//Smalltile (14,12,1,1) done
//SubrectsColoured
//Smalltile (15,12,1,1)
//Smalltile (15,12,1,1) done
//SubrectsColoured
//Smalltile (0,13,2,1)
//Smalltile (0,13,2,1) done
//SubrectsColoured
//Smalltile (4,13,1,2)
//Smalltile (4,13,1,2) done
//SubrectsColoured
//Smalltile (7,13,1,1)
//Smalltile (7,13,1,1) done
//SubrectsColoured
//Smalltile (12,13,2,1)
//Smalltile (12,13,2,1) done
//SubrectsColoured
//Smalltile (14,13,1,1)
//Smalltile (14,13,1,1) done
//SubrectsColoured
//Smalltile (15,13,1,2)
//Smalltile (15,13,1,2) done
//SubrectsColoured
//Smalltile (0,14,1,1)
//Smalltile (0,14,1,1) done
//SubrectsColoured
//Smalltile (1,14,1,1)
//Smalltile (1,14,1,1) done
//SubrectsColoured
//Smalltile (2,14,1,1)
//Smalltile (2,14,1,1) done
//SubrectsColoured
//Smalltile (6,14,2,1)
//Smalltile (6,14,2,1) done
//SubrectsColoured
//Smalltile (13,14,1,1)
//Smalltile (13,14,1,1) done
//SubrectsColoured
//Smalltile (14,14,1,2)
//Smalltile (14,14,1,2) done
//SubrectsColoured
//Smalltile (0,15,1,1)
//Smalltile (0,15,1,1) done
//SubrectsColoured
//Smalltile (1,15,1,1)
//Smalltile (1,15,1,1) done
//SubrectsColoured
//Smalltile (2,15,1,1)
//Smalltile (2,15,1,1) done
//SubrectsColoured
//Smalltile (3,15,1,1)
//Smalltile (3,15,1,1) done
//SubrectsColoured
//Smalltile (4,15,1,1)
//Smalltile (4,15,1,1) done
//SubrectsColoured
//Smalltile (5,15,1,1)
//Smalltile (5,15,1,1) done
//SubrectsColoured
//Smalltile (10,15,2,1)
//Smalltile (10,15,2,1) done
//SubrectsColoured
//Smalltile (12,15,1,1)
//Smalltile (12,15,1,1) done
//SubrectsColoured
//Smalltile (13,15,1,1)
//Smalltile (13,15,1,1) done
//SubrectsColoured
//Smalltile (15,15,1,1)
//Smalltile (15,15,1,1) done
//Rect=Rect(0 0 1920 34) Tile = Rect(64 0 16 16) cx_remain=1856, cy_remain=34


RED_AUTO_TEST_CASE(TestHextile)
{
    // FakeGraphic drawable(16, 1308, 19, 20);

    const uint8_t tile[] = {
/* 000 */ "\x01\x04\x19\xc3\x10\xe4\x18\xc3\x10\x25\x21\xe4\x18\x46\x29\x86"
/* 010 */ "\x29\x66\x29\xe8\x39\xc8\x39\x66\x29\x86\x29\xc7\x31\xc7\x31\xa7"
/* 020 */ "\x31\x86\x29\x25\x21\x45\x21\x25\x21\x05\x21\xc3\x10\x25\x21\x66"
/* 030 */ "\x29\xc7\x31\xa7\x31\x66\x29\x66\x29\xa7\x31\x86\x29\x86\x29\xc7"
/* 040 */ "\x31\x25\x21\xc4\x18\x25\x21\x86\x29\xe4\x18\xe4\x18\x45\x21\x86"
/* 050 */ "\x29\x86\x29\x45\x21\x05\x21\x66\x29\xe8\x39\xe8\x39\xc7\x31\x08"
/* 060 */ "\x3a\x46\x29\x46\x29\xa7\x31\x09\x42\x04\x19\x46\x29\x86\x29\x45"
/* 070 */ "\x21\x04\x19\x45\x21\x45\x21\x86\x29\x09\x42\x29\x42\xc7\x31\x66"
/* 080 */ "\x29\xa7\x31\xc7\x31\xe8\x39\xc7\x31\xa3\x10\x45\x21\x66\x29\xc4"
/* 090 */ "\x18\xa3\x10\x04\x19\x45\x21\xa7\x31\x29\x42\x08\x3a\x86\x29\x45"
/* 0a0 */ "\x21\x87\x31\x86\x29\x66\x29\x04\x19\x62\x08\x25\x21\x66\x29\xe4"
/* 0b0 */ "\x18\xa3\x10\xa3\x10\x05\x21\xe8\x39\x29\x42\x66\x29\x25\x21\xc7"
/* 0c0 */ "\x31\x08\x3a\xa7\x31\x45\x21\xc4\x18\xe4\x18\x25\x21\x86\x29\x25"
/* 0d0 */ "\x21\x04\x19\x05\x21\x86\x29\x08\x3a\xc8\x39\xe4\x18\xe4\x18\xe8"
/* 0e0 */ "\x39\xe8\x39\x25\x21\xe4\x18\xa3\x10\x25\x21\xe4\x18\x05\x21\x04"
/* 0f0 */ "\x19\xc3\x10\x66\x29\xc8\x39\xe8\x39\x66\x29\xe4\x18\x25\x21\x08"
/* 100 */ "\x3a\x86\x29\x66\x29\x86\x29\xc3\x10\xc3\x10\xe4\x18\x25\x21\xa3"
/* 110 */ "\x10\xc3\x10\x04\x19\x86\x29\x04\x19\x45\x21\xe8\x39\xa7\x31\xe8"
/* 120 */ "\x39\xc7\x31\x46\x29\x25\x21\xe4\x18\xe4\x18\xe4\x18\x25\x21\xe4"
/* 130 */ "\x18\x66\x29\xa7\x31\xc8\x39\x45\x21\x66\x29\x08\x3a\xe8\x39\x66"
/* 140 */ "\x29\xc7\x31\x45\x21\x25\x21\xa7\x31\xa7\x31\xc8\x39\xe8\x39\xe8"
/* 150 */ "\x39\x08\x3a\x66\x29\x45\x21\xe8\x39\xe8\x39\xe8\x39\x49\x42\x08"
/* 160 */ "\x3a\x45\x21\x05\x21\x25\x21\x09\x42\x08\x3a\x6a\x4a\x09\x42\x08"
/* 170 */ "\x3a\xc7\x31\xe4\x18\xc3\x10\xc8\x39\xe8\x39\xc7\x31\x8a\x4a\x49"
/* 180 */ "\x42\xa7\x31\xa7\x31\xa7\x31\x29\x42\xe8\x39\x49\x42\xe8\x39\x86"
/* 190 */ "\x29\x25\x21\x25\x21\x05\x21\x66\x29\x66\x29\xe8\x39\x6a\x4a\x45"
/* 1a0 */ "\x21\x09\x42\x29\x42\x29\x42\x08\x3a\xa7\x31\x08\x3a\xe8\x39\x66"
/* 1b0 */ "\x29\x04\x19\x25\x21\x45\x21\x66\x29\x66\x29\xa7\x31\xc7\x31\xc3"
/* 1c0 */ "\x10\x66\x29\x66\x29\xa7\x31\x66\x29\x45\x21\x46\x29\xa7\x31\x66"
/* 1d0 */ "\x29\x66\x29\x04\x19\x25\x21\x45\x21\x66\x29\x66\x29\x25\x21\x25"
/* 1e0 */ "\x21\xe4\x18\xc3\x10\x46\x29\x45\x21\x66\x29\x04\x19\x86\x29\x86"
/* 1f0 */ "\x29\x86\x29\x46\x29\x45\x21\x82\x08\xc3\x10\x66\x29\x25\x21\x25"
/* 200 */ "\x21\x01\x66\x29\x45\x21\xc8\x39\xc7\x31\xe8\x39\x86\x29\xc7\x31"
/* 210 */ "\x86\x29\x66\x29\xa7\x31\xc7\x31\x66\x29\x66\x29\x86\x29\xc7\x31"
/* 220 */ "\x86\x29\x66\x29\x66\x29\x86\x29\x25\x21\x66\x29\xc7\x31\x6a\x4a"
/* 230 */ "\x49\x42\x08\x3a\xe8\x39\xc7\x31\x86\x29\xe4\x18\x25\x21\x45\x21"
/* 240 */ "\x25\x21\xc7\x31\x08\x3a\x49\x42\x49\x42\x08\x3a\xc8\x39\xa7\x31"
/* 250 */ "\xe8\x39\x66\x29\xa7\x31\xa7\x31\xe8\x39\x29\x42\x29\x42\xa7\x31"
/* 260 */ "\x29\x42\x29\x42\xe8\x39\xc7\x31\x49\x42\xc7\x31\x08\x3a\x08\x3a"
/* 270 */ "\x29\x42\x29\x42\xe8\x39\x86\x29\x29\x42\x8b\x52\x29\x42\xa7\x31"
/* 280 */ "\x86\x29\x45\x21\xa7\x31\x09\x42\x49\x42\xe8\x39\x66\x29\xa7\x31"
/* 290 */ "\x29\x42\x29\x42\xa7\x31\x66\x29\x45\x21\x66\x29\xa7\x31\x49\x42"
/* 2a0 */ "\x49\x42\xa7\x31\x66\x29\xe8\x39\x08\x3a\x66\x29\x04\x19\xa7\x31"
/* 2b0 */ "\x29\x42\x8b\x52\x08\x3a\x08\x3a\xc7\x31\x66\x29\xa7\x31\xe8\x39"
/* 2c0 */ "\x66\x29\x87\x29\x09\x42\x6a\x4a\x49\x42\xe8\x39\xe8\x39\x09\x3a"
/* 2d0 */ "\x09\x3a\x09\x3a\x09\x42\x29\x42\xa7\x31\x49\x42\x29\x42\xe8\x39"
/* 2e0 */ "\x46\x21\x05\x21\x46\x29\x87\x31\xa7\x31\xc8\x39\xc7\x31\x25\x21"
/* 2f0 */ "\x25\x21\x29\x42\xa7\x31\x05\x21\xc4\x10\xe4\x18\x45\x21\x66\x29"
/* 300 */ "\x66\x29\x05\x19\x46\x29\xe4\x18\x25\x21\x46\x21\xe4\x18\xc4\x10"
/* 310 */ "\xe4\x18\x05\x21\x25\x21\x66\x29\x87\x29\x66\x29\x09\x3a\x09\x3a"
/* 320 */ "\x09\x3a\xe4\x18\x04\x19\xe4\x18\xc4\x10\xe4\x18\x25\x21\x66\x29"
/* 330 */ "\x66\x29\x87\x29\x09\x3a\x4a\x42\xc8\x31\x66\x29\x66\x29\x05\x19"
/* 340 */ "\xc3\x10\x45\x21\x09\x3a\x09\x3a\x66\x29\x66\x29\xa7\x31\xe8\x39"
/* 350 */ "\x66\x29\xc7\x31\x86\x29\x25\x21\x25\x21\xe8\x39\x8b\x52\x4a\x42"
/* 360 */ "\x87\x31\x09\x3a\x4a\x42\x4a\x4a\x09\x42\xa7\x31\x46\x29\x45\x21"
/* 370 */ "\xa7\x31\xe8\x39\xe8\x39\xe8\x39\xe8\x39\x29\x42\x8b\x52\x4a\x42"
/* 380 */ "\x29\x42\x01\x2c\x6b\x14\x9d\xaa\x5a\x69\x52\x49\x4a\xcb\x5a\x08"
/* 390 */ "\x42\x69\x4a\xc7\x31\xa7\x31\xe8\x39\x28\x42\x28\x42\x49\x4a\x69"
/* 3a0 */ "\x4a\x49\x4a\x6d\x73\xeb\x62\xaa\x5a\x28\x42\x49\x4a\x2d\x63\x29"
/* 3b0 */ "\x42\x49\x4a\xe8\x39\xe7\x39\x08\x42\x29\x42\x49\x4a\x8a\x52\xab"
/* 3c0 */ "\x52\x8a\x52\xcf\x83\xf3\x94\x69\x52\xc7\x39\xe8\x41\xeb\x5a\xe7"
/* 3d0 */ "\x39\x08\x3a\x29\x42\x08\x3a\x49\x42\x6a\x4a\x8a\x4a\xcb\x5a\xec"
/* 3e0 */ "\x5a\xcb\x5a\x01\xaa\x52\x4d\x6b\x0c\x5b\xf3\x9c\xf7\xbd\xbb\xd6"
/* 03f0 */"\xba\xd6\x75\xad\xcf\x7b\x4d\x6b\x4d\x6b\x4d\x6b\x4d\x63\x4d\x63"
    };
    const_bytes_view datas[1] = {
         make_array_view(tile),
    };
    BlockWrap bw(datas[0]);

    Buf64k buf;
    buf.read_with(bw);

    VNC::Encoder::Hextile encoder(BitsPerPixel{16}, BytesPerPixel{2}, {0, 0, 44, 19}, VNCVerbose::basic_trace);

//    encoder->consume(buf, drawable);

}

