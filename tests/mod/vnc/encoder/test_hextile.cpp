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
#include "system/redemption_unit_tests.hpp"
#include "test_only/fake_graphic.hpp"
#include "test_only/check_sig.hpp"

#include "utils/log.hpp"
#include "core/client_info.hpp"
#include "core/font.hpp"
#include "mod/vnc/vnc.hpp"
#include "test_only/transport/test_transport.hpp"
#include "mod/vnc/encoder/hextile.hpp"


class BlockWrap : public PartialReaderAPI
{
    const_byte_array & t;
    size_t pos;
public:
    BlockWrap(const_byte_array & t) : t(t), pos(0) {}
    
    size_t partial_read(byte_ptr buffer, size_t len) override
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
    bool empty() const {
        return this->t.size() == this->pos;
    }
};


RED_AUTO_TEST_CASE(TestHextile1)
{

    VNC::Encoder::Hextile encoder(16, 2, 0, 0, 44, 19, VNCVerbose::basic_trace);
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
    VNC::Encoder::Hextile encoder(16, 2, 0, 0, 48, 19, VNCVerbose::basic_trace);
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

//Background = 82 08

//7a e4 18 00 03 04 19 10 01 05 19 20 00 ...z.......... .
//0010 04 19 30 00 c4 18 40 00 c3 10 50 00 a3 10 60 11 ..0...@...P...`.
//0020 83 10 80 00 a3 10 90 01 83 10 a0 00 83 08 b0 00 ................
//0030 62 08 c0 10 83 08 f0 00 04 19 21 00 e4 18 31 00 b.........!...1.
//0040 c4 10 41 00 a3 10 51 50 83 10 b1 00 83 08 e1 04 ..A...QP........
//0050 a3 10 f1 05 e4 18 12 10 c4 18 32 00 a3 10 42 03 ..........2...B.
//0060 83 10 52 01 83 08 62 20 83 10 92 10 83 08 b2 00 ..R...b ........
//0070 62 08 c2 11 e4 18 13 00 c4 18 23 00 c3 10 33 00 b.........#...3.
//0080 83 08 63 11 83 08 93 10 e4 10 04 00 c4 10 14 00 ..c.............
//0090 c3 10 24 05 a3 10 34 11 a3 08 54 00 83 08 a4 00 ..$...4...T.....
//00a0 c4 10 05 00 c3 10 15 14 a3 10 55 02 83 08 65 00 ..........U...e.
//00b0 62 08 85 10 62 08 c5 01 c3 10 06 40 62 08 76 02 b...b......@b.v.
//00c0 62 00 86 00 62 08 96 40 e4 10 07 00 c3 10 37 10 b...b..@......7.
//00d0 62 08 87 10 a3 10 e7 00 c3 10 f7 00 e4 18 08 00 b...............
//00e0 e4 18 38 00 c3 10 48 00 62 08 68 10 a3 10 a8 00 ..8...H.b.h.....
//00f0 41 00 c8 00 c3 10 e8 01 a3 10 f8 00 c3 10 09 30 A..............0
//0100 a3 08 49 00 62 08 59 10 c3 10 89 01 a3 10 99 01 ..I.b.Y.........
//0110 62 08 b9 00 a3 10 d9 01 e4 18 f9 00 c3 10 0a 10 b...............
//0120 a3 10 2a 01 62 08 4a 00 62 00 5a 10 62 08 7a 01 ..*.b.J.b.Z.b.z.
//0130 41 00 aa 01 62 00 ba 00 c3 10 ca 00 a3 10 ea 00 A...b...........
//0140 04 19 fa 00 e4 18 0b 00 c3 10 1b 00 62 00 4b 00 ............b.K.
//0150 41 00 5b 00 62 00 6b 00 62 08 bb 00 e4 18 cb 00 A.[.b.k.b.......
//0160 62 08 eb 00 25 21 fb 00 e4 10 0c 00 e3 10 1c 00 b...%!..........
//0170 a3 08 2c 01 61 00 4c 00 62 00 5c 00 a3 08 9c 01 ..,.a.L.b.......
//0180 62 00 ac 00 e4 10 cc 00 a2 08 dc 00 c3 10 ec 00 b...............
//0190 c7 31 fc 00 c3 10 0d 10 62 00 4d 01 a3 08 7d 00 .1......b.M...}.
//01a0 a3 08 cd 10 45 21 ed 00 8a 4a fd 01 18 be 0e 00 ....E!...J......
//01b0 34 a5 1e 00 0c 5b 2e 00 a2 08 6e 10 c3 10 de 00 4....[....n.....
//01c0 a7 29 ee 01 ef 7b 0f 00 30 84 1f 00 55 a5 2f 00 .)...{..0...U./.
//01d0 18 be 3f 00 14 9d 4f 00 cb 52 5f 00 c3 10 af 10 ..?...O..R_.....
//01e0 a3 08 cf 00 e4 10 df 00 49 42 ff 00 01 a3 08 e4 ........IB......
//01f0 10 e8 31 4a 42 49 42 0c 5b 6a 4a 4d 63 8e 6b 71 ..1JBIB.[jJMc.kq
//0200 8c 51 84 71 8c b2 94 b2 8c d2 94 50 84 c3 08 25 .Q.q.......P...%
//0210 19 29 3a 49 42 8a 4a 2d 5b ab 52 4d 63 cf 73 72 .):IB.J-[.RMc.sr
//0220 8c 51 84 71 8c b2 8c 92 8c d3 94 92 8c a3 08 25 .Q.q...........%
//0230 19 49 42 29 3a 8a 4a 0c 5b cb 52 2d 63 af 73 51 .IB):.J.[.R-c.sQ
//0240 84 51 84 92 8c b2 94 91 8c 91 8c 71 84 a3 08 25 .Q.........q...%
//0250 19 49 42 29 3a cb 52 cb 52 cb 52 4d 63 30 84 51 .IB):.R.R.RMc0.Q
//0260 84 30 84 30 84 51 84 30 84 30 84 71 8c e4 10 86 .0.0.Q.0.0.q....
//0270 21 6a 42 6a 42 0c 5b ab 4a ec 5a af 73 92 8c 71 !jBjB.[.J.Z.s..q
//0280 8c 50 84 10 7c 30 84 10 7c 0f 7c 51 84 e4 10 c7 .P..|0..|.|Q....
//0290 29 69 42 8a 4a ec 52 ab 4a 0c 5b cf 73 72 8c 71 )iB.J.R.J.[.sr.q
//02a0 8c 71 8c 10 7c 30 84 51 84 ef 7b 30 84 e4 10 29 .q..|0.Q..{0...)
//02b0 3a 6a 42 8a 4a ab 52 0c 5b 2d 63 af 73 92 8c 92 :jB.J.R.[-c.s...
//02c0 8c 71 8c ef 7b 30 84 b2 94 71 8c 92 8c 24 19 cb .q..{0...q...$..
//02d0 52 cb 52 ab 4a ab 52 8e 6b af 73 cf 73 71 8c 92 R.R.J.R.k.s.sq..
//02e0 8c b2 94 50 84 92 8c 34 a5 b2 94 b2 94 86 29 49 ...P...4......)I
//02f0 42 cb 52 6a 42 2d 63 6e 6b ae 6b 6d 63 10 7c d3 B.RjB-cnk.kmc.|.
//0300 94 92 8c d2 94 14 a5 10 84 d3 9c 13 9d 49 42 aa .............IB.
//0310 4a 8a 4a aa 4a ec 5a 6e 6b ae 6b ef 7b 71 8c f4 J.J.J.Znk.k.{q..
//0320 9c b2 94 92 8c d3 94 10 84 91 8c cf 7b 49 42 8a ............{IB.
//0330 4a 69 42 0c 5b 0c 5b 8e 6b 8e 6b 30 7c 34 9d f3 JiB.[.[.k.k0|4..
//0340 94 d3 94 34 a5 f3 9c 8e 6b ae 73 4c 63 49 42 8a ...4....k.sLcIB.
//0350 4a 8a 4a eb 5a 4d 63 ae 73 cf 73 51 84 14 9d 92 J.J.ZMc.s.sQ....
//0360 8c 92 8c d2 94 30 84 0c 5b ae 73 ef 7b cb 52 eb .....0..[.s.{.R.
//0370 5a eb 52 8a 4a 4d 63 ae 6b 51 84 92 8c 10 7c 10 Z.R.JMc.kQ....|.
//0380 7c ef 7b 0c 5b eb 5a 6d 6b b2 94 91 8c ec 5a 0c |.{.[.Zmk.....Z.
//0390 5b 2c 5b aa 52 4d 63 6d 63 ef 7b 10 7c ef 73 ae [,[.RMcmc.{.|.s.
//03a0 73 ae 73 0c 5b 6d 6b 10 84 d3 9c 92 94 ab 52 cb s.s.[mk.......R.
//03b0 52 0c 5b 2c 5b 8e 6b 6d 63 8e 6b cf 73 51 84 ef R.[,[.kmc.k.sQ..
//03c0 7b 10 7c 30 84 b2 94 71 8c 71 8c b2 94 ec 5a eb {.|0...q.q....Z.
//03d0 5a aa 52 6d 63 6d 6b ae 6b ae 6b 71 8c 71 84 91 Z.Rmcmk.k.kq.q..
//03e0 8c b2 94 30 84 b2 94 92 94 b2 9c 14 a5 01 d2 8c ...0............
//03f0 f3 94 14 9d f3 94 b2 8c f3 94 55 a5 75 a5 96 ad ..........U.u...
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
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 16;
    info.width = 1308;
    info.height = 19;

    FakeGraphic drawable(info, 20);

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
    const_byte_array datas[1] = {
         make_array_view(tile),
    };
    BlockWrap bw(datas[0]);

    Buf64k buf;
    buf.read_from(bw);

    VNC::Encoder::Hextile encoder(16, 2, 0, 0, 44, 19, VNCVerbose::basic_trace);
    
    
//    encoder->consume(buf, drawable);

}

