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
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/nla/nla_server_ntlm.hpp"
#include "core/RDP/nla/nla_client_ntlm.hpp"
#include "core/RDP/tpdu_buffer.hpp"

#include "test_only/transport/test_transport.hpp"

#include "test_only/lcg_random.hpp"


RED_TEST_DELEGATE_PRINT_ENUM(credssp::State);

RED_AUTO_TEST_CASE(TestNlaclient)
{
    auto public_key = std::vector<uint8_t>{} << bytes_view("1245789652325415"_av); 
    std::string user("Ulysse");
    std::string domain("Ithaque");
    uint8_t pass[] = "Pénélope";
    uint8_t host[] = "Télémaque";
    LCGRandom rand(0);
    LCGTime timeobj;
    std::string extra_message;
    rdpClientNTLM credssp(user, domain, pass, host, "107.0.0.1", public_key, false, rand, timeobj);


    std::vector<uint8_t> expected_negotiate{
/* 0000 */ 0x30, 0x37, 0xa0, 0x03, 0x02, 0x01, 0x06, 0xa1, 0x30, 0x30, 0x2e, 0x30, 0x2c, 0xa0, 0x2a, 0x04,  // 07......00.0,.*.
/* 0010 */ 0x28, 0x4e, 0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50, 0x00, 0x01, 0x00, 0x00, 0x00, 0xb7, 0x82, 0x08,  // (NTLMSSP........
/* 0020 */ 0xe2, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00,  // .....(.......(..
/* 0030 */ 0x00, 0x06, 0x01, 0xb1, 0x1d, 0x00, 0x00, 0x00, 0x0f,  
    };

    StaticOutStream<65536> buffer_to_send_negotiate;
    credssp.client_authenticate_start(buffer_to_send_negotiate);
    RED_CHECK_HMEM(buffer_to_send_negotiate.get_bytes(), expected_negotiate);

    std::vector<uint8_t> server_answer_challenge{
    /* 0000 */ 0x30, 0x81, 0x88, 0xa0, 0x03, 0x02, 0x01, 0x02, 0xa1, 0x81, 0x80, 0x30, 0x7e, 0x30, 0x7c, 0xa0,  // 0..........0~0|.
    /* 0010 */ 0x7a, 0x04, 0x78, 0x4e, 0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,  // z.xNTLMSSP......
    /* 0020 */ 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0xb7, 0x82, 0x08, 0xe2, 0xb8, 0x6c, 0xda, 0xa6, 0xf0,  // ...8........l...
    /* 0030 */ 0xf6, 0x30, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x40, 0x00, 0x38,  // .0.........@.@.8
    /* 0040 */ 0x00, 0x00, 0x00, 0x05, 0x01, 0x28, 0x0a, 0x00, 0x00, 0x00, 0x0f, 0x01, 0x00, 0x08, 0x00, 0x57,  // .....(.........W
    /* 0050 */ 0x00, 0x49, 0x00, 0x4e, 0x00, 0x37, 0x00, 0x02, 0x00, 0x08, 0x00, 0x57, 0x00, 0x49, 0x00, 0x4e,  // .I.N.7.....W.I.N
    /* 0060 */ 0x00, 0x37, 0x00, 0x03, 0x00, 0x08, 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00, 0x04,  // .7.....w.i.n.7..
    /* 0070 */ 0x00, 0x08, 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00, 0x07, 0x00, 0x08, 0x00, 0x67,  // ...w.i.n.7.....g
    /* 0080 */ 0x95, 0x0e, 0x5a, 0x4e, 0x56, 0x76, 0xd6, 0x00, 0x00, 0x00, 0x00,                                // ..ZNVv.....
    };

    std::vector<uint8_t> expected_authenticate{
    /* 0000 */ 0x30, 0x82, 0x01, 0x59, 0xa0, 0x03, 0x02, 0x01, 0x06, 0xa1, 0x82, 0x01, 0x2c, 0x30, 0x82, 0x01,  // 0..Y........,0..
    /* 0010 */ 0x28, 0x30, 0x82, 0x01, 0x24, 0xa0, 0x82, 0x01, 0x20, 0x04, 0x82, 0x01, 0x1c, 0x4e, 0x54, 0x4c,  // (0..$... ....NTL
    /* 0020 */ 0x4d, 0x53, 0x53, 0x50, 0x00, 0x03, 0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x58, 0x00, 0x00,  // MSSP.........X..
    /* 0030 */ 0x00, 0x70, 0x00, 0x70, 0x00, 0x70, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x0e, 0x00, 0xe0, 0x00, 0x00,  // .p.p.p..........
    /* 0040 */ 0x00, 0x0c, 0x00, 0x0c, 0x00, 0xee, 0x00, 0x00, 0x00, 0x12, 0x00, 0x12, 0x00, 0xfa, 0x00, 0x00,  // ................
    /* 0050 */ 0x00, 0x10, 0x00, 0x10, 0x00, 0x0c, 0x01, 0x00, 0x00, 0x35, 0xa2, 0x88, 0xe2, 0x06, 0x01, 0xb1,  // .........5......
    /* 0060 */ 0x1d, 0x00, 0x00, 0x00, 0x0f, 0x9d, 0x98, 0xe1, 0x30, 0xdc, 0x7a, 0xce, 0x81, 0xa9, 0x7a, 0x11,  // ........0.z...z.
    /* 0070 */ 0x14, 0x7d, 0x5d, 0x73, 0xd0, 0x34, 0xbe, 0xb5, 0x6e, 0xce, 0xec, 0x0a, 0x50, 0x2d, 0x29, 0x63,  // .}]s.4..n...P-)c
    /* 0080 */ 0x7f, 0xcc, 0x5e, 0xe7, 0x18, 0xb8, 0x6c, 0xda, 0xa6, 0xf0, 0xf6, 0x30, 0x8d, 0xa6, 0x41, 0xcc,  // ..^...l....0..A.
    /* 0090 */ 0x7a, 0x52, 0x8e, 0x7a, 0xb3, 0x06, 0x7d, 0x0b, 0xe0, 0x00, 0xd5, 0xf6, 0x13, 0x01, 0x01, 0x00,  // zR.z..}.........
    /* 00a0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x67, 0x95, 0x0e, 0x5a, 0x4e, 0x56, 0x76, 0xd6, 0xb8, 0x6c, 0xda,  // .....g..ZNVv..l.
    /* 00b0 */ 0xa6, 0xf0, 0xf6, 0x30, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0x57, 0x00, 0x49,  // ...0.........W.I
    /* 00c0 */ 0x00, 0x4e, 0x00, 0x37, 0x00, 0x02, 0x00, 0x08, 0x00, 0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x37,  // .N.7.....W.I.N.7
    /* 00d0 */ 0x00, 0x03, 0x00, 0x08, 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00, 0x04, 0x00, 0x08,  // .....w.i.n.7....
    /* 00e0 */ 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00, 0x07, 0x00, 0x08, 0x00, 0x67, 0x95, 0x0e,  // .w.i.n.7.....g..
    /* 00f0 */ 0x5a, 0x4e, 0x56, 0x76, 0xd6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x00, 0x74,  // ZNVv.........I.t
    /* 0100 */ 0x00, 0x68, 0x00, 0x61, 0x00, 0x71, 0x00, 0x75, 0x00, 0x65, 0x00, 0x55, 0x00, 0x6c, 0x00, 0x79,  // .h.a.q.u.e.U.l.y
    /* 0110 */ 0x00, 0x73, 0x00, 0x73, 0x00, 0x65, 0x00, 0x54, 0x00, 0xe9, 0x00, 0x6c, 0x00, 0xe9, 0x00, 0x6d,  // .s.s.e.T...l...m
    /* 0120 */ 0x00, 0x61, 0x00, 0x71, 0x00, 0x75, 0x00, 0x65, 0x00, 0x44, 0xbc, 0x4d, 0x7a, 0x13, 0x3f, 0x6b,  // .a.q.u.e.D.Mz.?k
    /* 0130 */ 0x81, 0xdb, 0x1d, 0x2b, 0x7b, 0xbf, 0x1e, 0x18, 0x0f, 0xa3, 0x22, 0x04, 0x20, 0x01, 0x00, 0x00,  // ...+{.....". ...
    /* 0140 */ 0x00, 0x89, 0xe2, 0xda, 0x48, 0x17, 0x29, 0xb5, 0x08, 0x00, 0x00, 0x00, 0x00, 0x20, 0x59, 0x27,  // ....H.)...... Y'
    /* 0150 */ 0x3f, 0x08, 0xd0, 0xc2, 0xe4, 0x75, 0x66, 0x10, 0x49, 0x7b, 0xbd, 0x8d, 0xf7,                    // ?....uf.I{...
    };
    StaticOutStream<65536> buffer_to_send_authenticate;
    credssp::State st1 = credssp.client_authenticate_next(server_answer_challenge, buffer_to_send_authenticate);
    RED_CHECK(credssp::State::Cont == st1);
    RED_CHECK_HMEM(buffer_to_send_authenticate.get_bytes(), expected_authenticate);

    std::vector<uint8_t> server_answer_pubauthkey{
        /* 0000 */ 0x30, 0x29, 0xa0, 0x03, 0x02, 0x01, 0x02, 0xa3, 0x22, 0x04, 0x20, 0x01, 0x00, 0x00, 0x00, 0xa2,  // 0)......". .....
        /* 0010 */ 0xe0, 0x5b, 0x50, 0x97, 0x8e, 0x99, 0x27, 0x00, 0x00, 0x00, 0x00, 0xdc, 0xa7, 0x0b, 0xfe, 0x37,  // .[P...'........7
        /* 0020 */ 0x45, 0x3d, 0x1b, 0x05, 0x15, 0xce, 0x56, 0x0a, 0x54, 0xa1, 0xf1,                                // E=....V.T..
    };
    
    std::vector<uint8_t> expected_tscredentials{
        /* 0000 */ 0x30, 0x5c, 0xa0, 0x03, 0x02, 0x01, 0x06, 0xa2, 0x55, 0x04, 0x53, 0x01, 0x00, 0x00, 0x00, 0xaf,  // 0.......U.S.....
        /* 0010 */ 0xad, 0x46, 0x2a, 0x6a, 0x9d, 0xf7, 0x88, 0x01, 0x00, 0x00, 0x00, 0xd5, 0x4f, 0xc8, 0xd0, 0xbd,  // .F*j........O...
        /* 0020 */ 0x89, 0x60, 0xe0, 0x71, 0x60, 0x31, 0x7a, 0xcc, 0xec, 0xc5, 0xbf, 0x23, 0x4b, 0xe5, 0xf9, 0xa5,  // .`.q`1z....#K...
        /* 0030 */ 0x8c, 0x21, 0x66, 0xa6, 0x78, 0xda, 0xd1, 0xbd, 0xef, 0xa4, 0xfd, 0x47, 0xa6, 0xf1, 0x56, 0xa5,  // .!f.x......G..V.
        /* 0040 */ 0xd9, 0x52, 0x72, 0x92, 0xfa, 0x41, 0xa5, 0xb4, 0x9d, 0x94, 0xfb, 0x0e, 0xe2, 0x61, 0xba, 0xfc,  // .Rr..A.......a..
        /* 0050 */ 0xd5, 0xf3, 0xa7, 0xb5, 0x33, 0xd5, 0x62, 0x8d, 0x93, 0x18, 0x54, 0x39, 0x8a, 0xe7,              // ....3.b...T9..
    };
    
    StaticOutStream<65536> buffer_to_send_tscredentials;
    credssp::State st2 = credssp.client_authenticate_next(server_answer_pubauthkey, buffer_to_send_tscredentials);
    RED_CHECK(credssp::State::Finish == st2);
    RED_CHECK_HMEM(buffer_to_send_tscredentials.get_bytes(), expected_tscredentials);
}



RED_AUTO_TEST_CASE(TestNlaserver)
{
    auto client =
        // negotiate
    "\x30\x37\xa0\x03\x02\x01\x06\xa1\x30\x30\x2e\x30\x2c\xa0\x2a\x04" //07......00.0,.*. !
    "\x28\x4e\x54\x4c\x4d\x53\x53\x50\x00\x01\x00\x00\x00\xb7\x82\x08" //(NTLMSSP........ !
    "\xe2\x00\x00\x00\x00\x28\x00\x00\x00\x00\x00\x00\x00\x28\x00\x00" //.....(.......(.. !
    "\x00\x06\x01\xb1\x1d\x00\x00\x00\x0f" //......... !
        // authenticate + pubauthkey
    "\x30\x82\x01\x59\xa0\x03\x02\x01\x02\xa1\x82\x01\x2c\x30\x82\x01" //0..Y........,0.. !
    "\x28\x30\x82\x01\x24\xa0\x82\x01\x20\x04\x82\x01\x1c\x4e\x54\x4c" //(0..$... ....NTL !
    "\x4d\x53\x53\x50\x00\x03\x00\x00\x00\x18\x00\x18\x00\x58\x00\x00" //MSSP.........X.. !
    "\x00\x70\x00\x70\x00\x70\x00\x00\x00\x0e\x00\x0e\x00\xe0\x00\x00" //.p.p.p.......... !
    "\x00\x0c\x00\x0c\x00\xee\x00\x00\x00\x12\x00\x12\x00\xfa\x00\x00" //................ !
    "\x00\x10\x00\x10\x00\x0c\x01\x00\x00\x35\xa2\x88\xe2\x06\x01\xb1" //.........5...... !
    "\x1d\x00\x00\x00\x0f\x2e\x5b\xe2\x1f\x57\x20\x79\xa8\x5c\x70\x2d" //......[..W y..p- !
    "\x3d\xb6\x46\x81\x9a\x34\xbe\xb5\x6e\xce\xec\x0a\x50\x2d\x29\x63" //=.F..4..n...P-)c !
    "\x7f\xcc\x5e\xe7\x18\xb8\x6c\xda\xa6\xf0\xf6\x30\x8d\xa6\x41\xcc" //.^...l....0..A. !
    "\x7a\x52\x8e\x7a\xb3\x06\x7d\x0b\xe0\x00\xd5\xf6\x13\x01\x01\x00" //zR.z..}......... !
    "\x00\x00\x00\x00\x00\x67\x95\x0e\x5a\x4e\x56\x76\xd6\xb8\x6c\xda" //.....g..ZNVv..l. !
    "\xa6\xf0\xf6\x30\x8d\x00\x00\x00\x00\x01\x00\x08\x00\x57\x00\x49" //...0.........W.I !
    "\x00\x4e\x00\x37\x00\x02\x00\x08\x00\x57\x00\x49\x00\x4e\x00\x37" //.N.7.....W.I.N.7 !
    "\x00\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x04\x00\x08" //.....w.i.n.7.... !
    "\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x07\x00\x08\x00\x67\x95\x0e" //.w.i.n.7.....g.. !
    "\x5a\x4e\x56\x76\xd6\x00\x00\x00\x00\x00\x00\x00\x00\x49\x00\x74" //ZNVv.........I.t !
    "\x00\x68\x00\x61\x00\x71\x00\x75\x00\x65\x00\x55\x00\x6c\x00\x79" //.h.a.q.u.e.U.l.y !
    "\x00\x73\x00\x73\x00\x65\x00\x54\x00\xe9\x00\x6c\x00\xe9\x00\x6d" //.s.s.e.T...l...m !
    "\x00\x61\x00\x71\x00\x75\x00\x65\x00\x44\xbc\x4d\x7a\x13\x3f\x6b" //.a.q.u.e.D.Mz.?k !
    "\x81\xdb\x1d\x2b\x7b\xbf\x1e\x18\x0f\xa3\x22\x04\x20\x01\x00\x00" //...+{.....". ... !
    "\x00\x89\xe2\xda\x48\x17\x29\xb5\x08\x00\x00\x00\x00\x20\x59\x27" //....H.)...... Y' !
    "\x3f\x08\xd0\xc2\xe4\x75\x66\x10\x49\x7b\xbd\x8d\xf7" //?....uf.I{... !]

        // ts credentials (authinfo)
/* 0000 */ "\x30\x5c\xa0\x03\x02\x01\x02\xa2\x55\x04\x53\x01\x00\x00\x00\xaf" // 0.......U.S.....
/* 0010 */ "\xad\x46\x2a\x6a\x9d\xf7\x88\x01\x00\x00\x00\xd5\x4f\xc8\xd0\xbd" // .F*j........O...
/* 0020 */ "\x89\x60\xe0\x71\x60\x31\x7a\xcc\xec\xc5\xbf\x23\x4b\xe5\xf9\xa5" // .`.q`1z....#K...
/* 0030 */ "\x8c\x21\x66\xa6\x78\xda\xd1\xbd\xef\xa4\xfd\x47\xa6\xf1\x56\xa5" // .!f.x......G..V.
/* 0040 */ "\xd9\x52\x72\x92\xfa\x41\xa5\xb4\x9d\x94\xfb\x0e\xe2\x61\xba\xfc" // .Rr..A.......a..
/* 0050 */ "\xd5\xf3\xa7\xb5\x33\xd5\x62\x8d\x93\x18\x54\x39\x8a\xe7"         // ....3.b...T9..
    ""_av
        ;

    auto server =
        // challenge
    "\x30\x81\x88\xa0\x03\x02\x01\x06\xa1\x81\x80\x30\x7e\x30\x7c\xa0" //0..........0~0|. !
    "\x7a\x04\x78\x4e\x54\x4c\x4d\x53\x53\x50\x00\x02\x00\x00\x00\x00" //z.xNTLMSSP...... !
    "\x00\x00\x00\x38\x00\x00\x00\xb7\x82\x08\xe2\xb8\x6c\xda\xa6\xf0" //...8........l... !
    "\xf6\x30\x8d\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x40\x00\x38" //.0.........@.@.8 !
    "\x00\x00\x00\x06\x01\xb1\x1d\x00\x00\x00\x0f\x01\x00\x08\x00\x57" //...............W !
    "\x00\x49\x00\x4e\x00\x37\x00\x02\x00\x08\x00\x57\x00\x49\x00\x4e" //.I.N.7.....W.I.N !
    "\x00\x37\x00\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x04" //.7.....w.i.n.7.. !
    "\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x07\x00\x08\x00\x67" //...w.i.n.7.....g !
    "\x95\x0e\x5a\x4e\x56\x76\xd6\x00\x00\x00\x00" //..ZNVv..... !]
//         // pubauthkey
/* 0000 */ "\x30\x29\xa0\x03\x02\x01\x06\xa3\x22\x04\x20\x01\x00\x00\x00\xa2" //0)......". .....
/* 0010 */ "\xe0\x5b\x50\x97\x8e\x99\x27\x00\x00\x00\x00\xdc\xa7\x0b\xfe\x37" //.[P...'........7
/* 0020 */ "\x45\x3d\x1b\x05\x15\xce\x56\x0a\x54\xa1\xf1"                     //E=....V.T..
    ""_av
        ;

    TestTransport logtrans(client, server);
    logtrans.set_public_key("1245789652325415"_av);
    auto user = "Ulysse"_av;
    auto domain = "Ithaque"_av;
    auto pass = "Pénélope"_av;
//    auto host = "Télémaque"_av;
    LCGRandom rand(0);
    LCGTime timeobj;
    std::string extra_message;
    Translation::language_t lang = Translation::EN;

    auto key = logtrans.get_public_key();

    uint8_t front_public_key[1024] = {};
    array_view_u8 front_public_key_av = {};
    memcpy(front_public_key, key.data(), key.size());
    front_public_key_av = array_view(front_public_key, key.size());

    rdpCredsspServerNTLM credssp(
        front_public_key_av, rand, timeobj, extra_message, lang,
        [&](bytes_view user_av, bytes_view domain_av, std::vector<uint8_t> & password_array){
            std::vector<uint8_t> vec;
            vec.resize(user.size() * 2);
            UTF8toUTF16(user, vec.data(), vec.size());
            RED_CHECK_MEM(user_av, vec);
            vec.resize(domain.size() * 2);
            UTF8toUTF16(domain, vec.data(), vec.size());
            RED_CHECK_MEM(domain_av, vec);
            size_t user_len = UTF8Len(byte_ptr_cast(pass.data()));
            LOG(LOG_INFO, "callback lambda: user_len=%lu", user_len);
            password_array = std::vector<uint8_t>(user_len * 2);
            UTF8toUTF16({pass.data(), strlen(char_ptr_cast(byte_ptr_cast(pass.data())))}, password_array.data(), user_len * 2);
            return PasswordCallback::Ok;
        }, true
    );

    credssp::State st = credssp::State::Cont;
    TpduBuffer buf;
    buf.load_data(logtrans);
    RED_CHECK_EQUAL(true, buf.next(TpduBuffer::CREDSSP));
    RED_CHECK_EQUAL(int(credssp::State::Cont), int(st));
    // Recv negotiate, send challenge
    {
        StaticOutStream<65536> out_stream;
        LOG(LOG_INFO, "Recv Negotiate");
        hexdump_c(buf.current_pdu_buffer());
        st = credssp.credssp_server_authenticate_next(buf.current_pdu_buffer(), out_stream);
        RED_CHECK_EQUAL(buf.remaining(), 500);
        RED_CHECK_EQUAL(out_stream.get_bytes().size(), 139);
        logtrans.send(out_stream.get_bytes());
    }
    RED_CHECK_EQUAL(true, buf.next(TpduBuffer::CREDSSP));
    RED_CHECK_EQUAL(int(credssp::State::Cont), int(st));
    // Recv authenticate + pubauthkey, send pubauthkey
    {
        StaticOutStream<65536> out_stream;
        LOG(LOG_INFO, "Recv authenticate");
        hexdump_c(buf.current_pdu_buffer());
        RED_CHECK_EQUAL(buf.remaining(), 443);
        st = credssp.credssp_server_authenticate_next(buf.current_pdu_buffer(), out_stream);
        RED_CHECK_EQUAL(out_stream.get_bytes().size(), 43);
        logtrans.send(out_stream.get_bytes());
    }
    RED_CHECK_EQUAL(true, buf.next(TpduBuffer::CREDSSP));
    RED_CHECK_EQUAL(int(credssp::State::Cont), int(st));
    // Recv ts_credential, -> finished
    {
        StaticOutStream<65536> out_stream;
        LOG(LOG_INFO, "Recv ts_credential");
        hexdump_c(buf.current_pdu_buffer());
        RED_CHECK_EQUAL(buf.remaining(), 94);
        LOG(LOG_INFO, "Calling credssp_server_authenticate_next");
        st = credssp.credssp_server_authenticate_next(buf.current_pdu_buffer(), out_stream);
        LOG(LOG_INFO, "Call to credssp_server_authenticate_next done");
        RED_CHECK_EQUAL(out_stream.get_bytes().size(), 0);
        logtrans.send(out_stream.get_bytes());
    }
    RED_CHECK_EQUAL(false, buf.next(TpduBuffer::CREDSSP));
    RED_CHECK_EQUAL(0, buf.remaining());
    RED_CHECK_EQUAL(int(st), int(credssp::State::Finish));
}

RED_AUTO_TEST_CASE(TestNlaClientDebug)
{
// Client --> Server

// negotiate
//Sending Credssp
// Credssp Header
// 30 37 a0 03 02 01 02 a1 30 30 2e 30 2c a0 2a 04 28   
// NTLM NEGOTIATE_MESSAGE
// 4e 54 4c 4d 53 53 50 00                              NTLMSSP
// 01 00 00 00                                          MessageType
// b7 82 08 e2                                          NegotiateFlags
// 00 00 00 00 00 00 00 00                              DomainNameFields
// 00 00 00 00 00 00 00 00                              WorkStationFields
// 06 01                                                Windows 6.1
// b1 1d                                                ProductBuild
// 00 00 00                                             Reserved
// 0f                                                   NTLMSSP_REVISION_W2K3 

// Server --> Client

// challenge





//connected to 10.10.47.76:3389
//transport_newtransport_connect><transport_connect_nlatransport_connect_tlscredssp_new
//credssp_authenticate
//....credssp_client_authenticate
//........credssp_ntlm_client_init
//User: a Domain: (null) Password: S
//NEGOTIATE_MESSAGE (length = 40)
//0000 4e 54 4c 4d 53 53 50 00 01 00 00 00 b7 82 08 e2 NTLMSSP.........
//0010 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
//0020 06 01 b1 1d 00 00 00 0f                         ........

//VERSION =
//{
//	ProductMajorVersion: 6
//	ProductMinorVersion: 1
//	ProductBuild: 7601
//	Reserved: 0x000000
//	NTLMRevisionCurrent: 0x0F
//Sending Authentication Token
//0000 4e 54 4c 4d 53 53 50 00 01 00 00 00 b7 82 08 e2 NTLMSSP.........
//0010 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
//0020 06 01 b1 1d 00 00 00 0f                         ........
//............credssp_send
//................credssp_sizeof_nego_tokens(
//....................credssp_sizeof_nego_token
//................credssp_sizeof_ts_request
//............-- TSRequest
//............-- [0] version
//............-- [1] negoTokens (NegoData)
//............-- [2] authInfo (OCTET STRING)
//............-- [3] pubKeyAuth (OCTET STRING)
//================= Sending Credssp =====================
//0000 30 37 a0 03 02 01 02 a1 30 30 2e 30 2c a0 2a 04 07......00.0,.*.
//0010 28 4e 54 4c 4d 53 53 50 00 01 00 00 00 b7 82 08 (NTLMSSP........
//0020 e2 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
//0030 00 06 01 b1 1d 00 00 00 0f                      .........
//================= ================ =====================
//>............-- done credssp_send
//credssp_buffer_free
//............credssp_recv
//================= Receving Credssp =====================
//0000 30 81 bc a0 03 02 01 06 a1 81 b4 30 81 b1 30 81 0..........0..0.
//0010 ae a0 81 ab 04 81 a8 4e 54 4c 4d 53 53 50 00 02 .......NTLMSSP..
//0020 00 00 00 10 00 10 00 38 00 00 00 35 82 8a e2 4d .......8...5...M
//0030 04 c8 d5 d7 d9 fd 2d 00 00 00 00 00 00 00 00 60 ......-........`
//0040 00 60 00 48 00 00 00 06 03 80 25 00 00 00 0f 53 .`.H......%....S
//0050 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 02 .A.-.S.V.R.0.1..
//0060 00 10 00 53 00 41 00 2d 00 53 00 56 00 52 00 30 ...S.A.-.S.V.R.0
//0070 00 31 00 01 00 10 00 53 00 41 00 2d 00 53 00 56 .1.....S.A.-.S.V
//0080 00 52 00 30 00 31 00 04 00 10 00 53 00 41 00 2d .R.0.1.....S.A.-
//0090 00 53 00 56 00 52 00 30 00 31 00 03 00 10 00 53 .S.V.R.0.1.....S
//00a0 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 07 .A.-.S.V.R.0.1..
//00b0 00 08 00 15 27 80 39 4e bd d3 01 00 00 00 00    ....'.9N.......
//================= ================ =====================
//<............-- TSrequest
//............-- [1] negoTokens (NegoData)
//............-- [2] authInfo (OCTET STRING)
//............-- [3] pubKeyAuth (OCTET STRING)
//............-- done credssp_recv
//Receiving Authentication Token (168)
//0000 4e 54 4c 4d 53 53 50 00 02 00 00 00 10 00 10 00 NTLMSSP.........
//0010 38 00 00 00 35 82 8a e2 4d 04 c8 d5 d7 d9 fd 2d 8...5...M......-
//0020 00 00 00 00 00 00 00 00 60 00 60 00 48 00 00 00 ........`.`.H...
//0030 06 03 80 25 00 00 00 0f 53 00 41 00 2d 00 53 00 ...%....S.A.-.S.
//0040 56 00 52 00 30 00 31 00 02 00 10 00 53 00 41 00 V.R.0.1.....S.A.
//0050 2d 00 53 00 56 00 52 00 30 00 31 00 01 00 10 00 -.S.V.R.0.1.....
//0060 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//0070 04 00 10 00 53 00 41 00 2d 00 53 00 56 00 52 00 ....S.A.-.S.V.R.
//0080 30 00 31 00 03 00 10 00 53 00 41 00 2d 00 53 00 0.1.....S.A.-.S.
//0090 56 00 52 00 30 00 31 00 07 00 08 00 15 27 80 39 V.R.0.1......'.9
//00a0 4e bd d3 01 00 00 00 00                         N.......
//CHALLENGE_MESSAGE (length = 168)
//0000 4e 54 4c 4d 53 53 50 00 02 00 00 00 10 00 10 00 NTLMSSP.........
//0010 38 00 00 00 35 82 8a e2 4d 04 c8 d5 d7 d9 fd 2d 8...5...M......-
//0020 00 00 00 00 00 00 00 00 60 00 60 00 48 00 00 00 ........`.`.H...
//0030 06 03 80 25 00 00 00 0f 53 00 41 00 2d 00 53 00 ...%....S.A.-.S.
//0040 56 00 52 00 30 00 31 00 02 00 10 00 53 00 41 00 V.R.0.1.....S.A.
//0050 2d 00 53 00 56 00 52 00 30 00 31 00 01 00 10 00 -.S.V.R.0.1.....
//0060 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//0070 04 00 10 00 53 00 41 00 2d 00 53 00 56 00 52 00 ....S.A.-.S.V.R.
//0080 30 00 31 00 03 00 10 00 53 00 41 00 2d 00 53 00 0.1.....S.A.-.S.
//0090 56 00 52 00 30 00 31 00 07 00 08 00 15 27 80 39 V.R.0.1......'.9
//00a0 4e bd d3 01 00 00 00 00                         N.......

//negotiateFlags "0xE28A8235"{
//	NTLMSSP_NEGOTIATE_56 (0),
//	NTLMSSP_NEGOTIATE_KEY_EXCH (1),
//	NTLMSSP_NEGOTIATE_128 (2),
//	NTLMSSP_NEGOTIATE_VERSION (6),
//	NTLMSSP_NEGOTIATE_TARGET_INFO (8),
//	NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY (12),
//	NTLMSSP_TARGET_TYPE_SERVER (14),
//	NTLMSSP_NEGOTIATE_ALWAYS_SIGN (16),
//	NTLMSSP_NEGOTIATE_NTLM (22),
//	NTLMSSP_NEGOTIATE_SEAL (26),
//	NTLMSSP_NEGOTIATE_SIGN (27),
//	NTLMSSP_REQUEST_TARGET (29),
//	NTLMSSP_NEGOTIATE_UNICODE (31),
//}
//VERSION =
//{
//	ProductMajorVersion: 6
//	ProductMinorVersion: 3
//	ProductBuild: 9600
//	Reserved: 0x000000
//	NTLMRevisionCurrent: 0x0F
//TargetName (Len: 16 MaxLen: 16 BufferOffset: 56)
//0000 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.

//TargetInfo (Len: 96 MaxLen: 96 BufferOffset: 72)
//0000 02 00 10 00 53 00 41 00 2d 00 53 00 56 00 52 00 ....S.A.-.S.V.R.
//0010 30 00 31 00 01 00 10 00 53 00 41 00 2d 00 53 00 0.1.....S.A.-.S.
//0020 56 00 52 00 30 00 31 00 04 00 10 00 53 00 41 00 V.R.0.1.....S.A.
//0030 2d 00 53 00 56 00 52 00 30 00 31 00 03 00 10 00 -.S.V.R.0.1.....
//0040 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//0050 07 00 08 00 15 27 80 39 4e bd d3 01 00 00 00 00 .....'.9N.......

//ChallengeTargetInfo (96):
//AV_PAIRs =
//{
//	MsvAvNbDomainName AvId: 2 AvLen: 16
//0000 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//	MsvAvNbComputerName AvId: 1 AvLen: 16
//0000 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//	MsvAvDnsDomainName AvId: 4 AvLen: 16
//0000 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//	MsvAvDnsComputerName AvId: 3 AvLen: 16
//0000 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//	MsvAvTimestamp AvId: 7 AvLen: 8
//0000 15 27 80 39 4e bd d3 01                         .'.9N...
//}
//Password (length = 28)
//0000 53 00 65 00 63 00 75 00 72 00 65 00 4c 00 69 00 S.e.c.u.r.e.L.i.
//0010 6e 00 75 00 78 00 24 00 34 00 32 00             n.u.x.$.4.2.

//Username (length = 28)
//0000 61 00 64 00 6d 00 69 00 6e 00 69 00 73 00 74 00 a.d.m.i.n.i.s.t.
//0010 72 00 61 00 74 00 65 00 75 00 72 00             r.a.t.e.u.r.

//Domain (length = 0)

//Workstation (length = 12)
//0000 63 00 67 00 72 00 74 00 68 00 63 00             c.g.r.t.h.c.

//NTOWFv2, NTLMv2 Hash
//0000 fd f6 00 f1 51 94 3c 72 8f c6 02 05 65 59 9f 2d ....Q.<r....eY.-

//NTLMv2 Response Temp Blob
//0000 01 01 00 00 00 00 00 00 15 27 80 39 4e bd d3 01 .........'.9N...
//0010 aa aa aa aa aa aa aa aa 00 00 00 00 02 00 10 00 ................
//0020 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//0030 01 00 10 00 53 00 41 00 2d 00 53 00 56 00 52 00 ....S.A.-.S.V.R.
//0040 30 00 31 00 04 00 10 00 53 00 41 00 2d 00 53 00 0.1.....S.A.-.S.
//0050 56 00 52 00 30 00 31 00 03 00 10 00 53 00 41 00 V.R.0.1.....S.A.
//0060 2d 00 53 00 56 00 52 00 30 00 31 00 07 00 08 00 -.S.V.R.0.1.....
//0070 15 27 80 39 4e bd d3 01 06 00 04 00 02 00 00 00 .'.9N...........
//0080 0a 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
//0090 00 00 00 00 09 00 26 00 54 00 45 00 52 00 4d 00 ......&.T.E.R.M.
//00a0 53 00 52 00 56 00 2f 00 31 00 30 00 2e 00 31 00 S.R.V./.1.0...1.
//00b0 30 00 2e 00 34 00 37 00 2e 00 37 00 36 00 00 00 0...4.7...7.6...
//00c0 00 00 00 00 00 00 00 00 00 00 00 00 00 00       ..............

//ClientChallenge
//0000 aa aa aa aa aa aa aa aa                         ........

//ServerChallenge
//0000 4d 04 c8 d5 d7 d9 fd 2d                         M......-

//SessionBaseKey
//0000 8f 9f fe e2 86 25 32 16 7f 1b e1 b0 c4 36 9c 84 .....%2......6..

//KeyExchangeKey
//0000 8f 9f fe e2 86 25 32 16 7f 1b e1 b0 c4 36 9c 84 .....%2......6..

//ExportedSessionKey
//0000 aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa ................

//RandomSessionKey
//0000 aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa ................

//ClientSigningKey
//0000 70 bf 8d 0c ee 83 5f 26 5e d6 f2 15 c7 d2 72 f8 p....._&^.....r.

//ClientSealingKey
//0000 78 ea 2b df c8 28 b1 5d ca 30 1a da 99 f1 6c c8 x.+..(.].0....l.

//ServerSigningKey
//0000 c9 85 4b e8 74 c7 1c 9d a1 15 14 01 35 09 43 4e ..K.t.......5.CN

//ServerSealingKey
//0000 06 e1 de 5c a5 00 c3 e8 75 65 05 f1 03 6e 81 f8 ...\....ue...n..

//Timestamp
//0000 15 27 80 39 4e bd d3 01                         .'.9N...

//AUTHENTICATE_MESSAGE (length = 390)
//0000 4e 54 4c 4d 53 53 50 00 03 00 00 00 18 00 18 00 NTLMSSP.........
//0010 80 00 00 00 de 00 de 00 98 00 00 00 00 00 00 00 ................
//0020 58 00 00 00 1c 00 1c 00 58 00 00 00 0c 00 0c 00 X.......X.......
//0030 74 00 00 00 10 00 10 00 76 01 00 00 35 a2 88 e2 t.......v...5...
//0040 06 01 b1 1d 00 00 00 0f 23 02 14 ae 47 fc e3 07 ........#...G...
//0050 0c f4 97 e3 08 71 eb cf 61 00 64 00 6d 00 69 00 .....q..a.d.m.i.
//0060 6e 00 69 00 73 00 74 00 72 00 61 00 74 00 65 00 n.i.s.t.r.a.t.e.
//0070 75 00 72 00 63 00 67 00 72 00 74 00 68 00 63 00 u.r.c.g.r.t.h.c.
//0080 8b 09 e7 80 37 80 86 73 59 aa 27 ad 72 63 d2 7f ....7..sY.'.rc..
//0090 aa aa aa aa aa aa aa aa 8c c2 c6 d1 ee 99 be a1 ................
//00a0 70 f2 72 5b c3 30 af 53 01 01 00 00 00 00 00 00 p.r[.0.S........
//00b0 15 27 80 39 4e bd d3 01 aa aa aa aa aa aa aa aa .'.9N...........
//00c0 00 00 00 00 02 00 10 00 53 00 41 00 2d 00 53 00 ........S.A.-.S.
//00d0 56 00 52 00 30 00 31 00 01 00 10 00 53 00 41 00 V.R.0.1.....S.A.
//00e0 2d 00 53 00 56 00 52 00 30 00 31 00 04 00 10 00 -.S.V.R.0.1.....
//00f0 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//0100 03 00 10 00 53 00 41 00 2d 00 53 00 56 00 52 00 ....S.A.-.S.V.R.
//0110 30 00 31 00 07 00 08 00 15 27 80 39 4e bd d3 01 0.1......'.9N...
//0120 06 00 04 00 02 00 00 00 0a 00 10 00 00 00 00 00 ................
//0130 00 00 00 00 00 00 00 00 00 00 00 00 09 00 26 00 ..............&.
//0140 54 00 45 00 52 00 4d 00 53 00 52 00 56 00 2f 00 T.E.R.M.S.R.V./.
//0150 31 00 30 00 2e 00 31 00 30 00 2e 00 34 00 37 00 1.0...1.0...4.7.
//0160 2e 00 37 00 36 00 00 00 00 00 00 00 00 00 00 00 ..7.6...........
//0170 00 00 00 00 00 00 a5 2e ba 9b 1d 1c ca b1 f5 33 ...............3
//0180 c1 e5 53 77 94 f0                               ..Sw..

//negotiateFlags "0xE288A235"{
//	NTLMSSP_NEGOTIATE_56 (0),
//	NTLMSSP_NEGOTIATE_KEY_EXCH (1),
//	NTLMSSP_NEGOTIATE_128 (2),
//	NTLMSSP_NEGOTIATE_VERSION (6),
//	NTLMSSP_NEGOTIATE_TARGET_INFO (8),
//	NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY (12),
//	NTLMSSP_NEGOTIATE_ALWAYS_SIGN (16),
//	NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED (18),
//	NTLMSSP_NEGOTIATE_NTLM (22),
//	NTLMSSP_NEGOTIATE_SEAL (26),
//	NTLMSSP_NEGOTIATE_SIGN (27),
//	NTLMSSP_REQUEST_TARGET (29),
//	NTLMSSP_NEGOTIATE_UNICODE (31),
//}
//VERSION =
//{
//	ProductMajorVersion: 6
//	ProductMinorVersion: 1
//	ProductBuild: 7601
//	Reserved: 0x000000
//	NTLMRevisionCurrent: 0x0F
//AuthenticateTargetInfo (178):
//AV_PAIRs =
//{
//	MsvAvNbDomainName AvId: 2 AvLen: 16
//0000 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//	MsvAvNbComputerName AvId: 1 AvLen: 16
//0000 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//	MsvAvDnsDomainName AvId: 4 AvLen: 16
//0000 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//	MsvAvDnsComputerName AvId: 3 AvLen: 16
//0000 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//	MsvAvTimestamp AvId: 7 AvLen: 8
//0000 15 27 80 39 4e bd d3 01                         .'.9N...
//	MsvAvFlags AvId: 6 AvLen: 4
//0000 02 00 00 00                                     ....
//	MsvChannelBindings AvId: 10 AvLen: 16
//0000 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
//	MsvAvTargetName AvId: 9 AvLen: 38
//0000 54 00 45 00 52 00 4d 00 53 00 52 00 56 00 2f 00 T.E.R.M.S.R.V./.
//0010 31 00 30 00 2e 00 31 00 30 00 2e 00 34 00 37 00 1.0...1.0...4.7.
//0020 2e 00 37 00 36 00                               ..7.6.
//}
//DomainName (Len: 0 MaxLen: 0 BufferOffset: 88)

//UserName (Len: 28 MaxLen: 28 BufferOffset: 88)
//0000 61 00 64 00 6d 00 69 00 6e 00 69 00 73 00 74 00 a.d.m.i.n.i.s.t.
//0010 72 00 61 00 74 00 65 00 75 00 72 00             r.a.t.e.u.r.

//Workstation (Len: 12 MaxLen: 12 BufferOffset: 116)
//0000 63 00 67 00 72 00 74 00 68 00 63 00             c.g.r.t.h.c.

//LmChallengeResponse (Len: 24 MaxLen: 24 BufferOffset: 128)
//0000 8b 09 e7 80 37 80 86 73 59 aa 27 ad 72 63 d2 7f ....7..sY.'.rc..
//0010 aa aa aa aa aa aa aa aa                         ........

//NtChallengeResponse (Len: 222 MaxLen: 222 BufferOffset: 152)
//0000 8c c2 c6 d1 ee 99 be a1 70 f2 72 5b c3 30 af 53 ........p.r[.0.S
//0010 01 01 00 00 00 00 00 00 15 27 80 39 4e bd d3 01 .........'.9N...
//0020 aa aa aa aa aa aa aa aa 00 00 00 00 02 00 10 00 ................
//0030 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//0040 01 00 10 00 53 00 41 00 2d 00 53 00 56 00 52 00 ....S.A.-.S.V.R.
//0050 30 00 31 00 04 00 10 00 53 00 41 00 2d 00 53 00 0.1.....S.A.-.S.
//0060 56 00 52 00 30 00 31 00 03 00 10 00 53 00 41 00 V.R.0.1.....S.A.
//0070 2d 00 53 00 56 00 52 00 30 00 31 00 07 00 08 00 -.S.V.R.0.1.....
//0080 15 27 80 39 4e bd d3 01 06 00 04 00 02 00 00 00 .'.9N...........
//0090 0a 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
//00a0 00 00 00 00 09 00 26 00 54 00 45 00 52 00 4d 00 ......&.T.E.R.M.
//00b0 53 00 52 00 56 00 2f 00 31 00 30 00 2e 00 31 00 S.R.V./.1.0...1.
//00c0 30 00 2e 00 34 00 37 00 2e 00 37 00 36 00 00 00 0...4.7...7.6...
//00d0 00 00 00 00 00 00 00 00 00 00 00 00 00 00       ..............

//EncryptedRandomSessionKey (Len: 16 MaxLen: 16 BufferOffset: 374)
//0000 a5 2e ba 9b 1d 1c ca b1 f5 33 c1 e5 53 77 94 f0 .........3..Sw..

//MessageIntegrityCheck (length = 16)
//0000 23 02 14 ae 47 fc e3 07 0c f4 97 e3 08 71 eb cf #...G........q..

//credssp_encrypt_public_key_echo
//Data Buffer (length = 270)
//0000 30 82 01 0a 02 82 01 01 00 de fa 41 80 c6 5e 85 0..........A..^.
//0010 95 c7 5c cd ab f0 0a 26 2e e9 f6 84 82 27 ad fc ..\....&.....'..
//0020 56 a1 c7 7e 5c 9b 8c f2 e1 87 6f ab ed f2 b4 64 V..~\.....o....d
//0030 70 5f 31 a3 86 69 d6 c5 e9 af 5a 68 8d a2 4d 14 p_1..i....Zh..M.
//0040 69 ae d5 b1 25 9d ee f8 5e 59 8c 44 6b 8b b5 35 i...%...^Y.Dk..5
//0050 23 fe 87 e6 f5 eb 9d b2 24 dd 91 cf 93 ad 6e 1b #.......$.....n.
//0060 22 cb a4 9e 07 0c 7c 97 fa f5 b0 6d 5a 52 a7 c7 ".....|....mZR..
//0070 9a 0a d8 0a d5 10 9e 1c 9d f5 7f 11 46 9f 9f c5 ............F...
//0080 7b 36 9a c8 5b c2 c3 a9 90 62 7e fe 43 65 14 55 {6..[....b~.Ce.U
//0090 42 12 02 7c ce 40 23 70 ae b5 23 96 c2 8f 41 f9 B..|.@#p..#...A.
//00a0 3c ec 1a d7 b4 3c 4d 7d 08 fb 45 2b 27 ca 02 20 <....<M}..E+'.. 
//00b0 86 12 65 03 16 51 0a 6e b4 42 9d f9 1b 73 a4 d3 ..e..Q.n.B...s..
//00c0 9b 5c b0 e6 75 49 0c 88 c4 3c 06 35 2e cf 1d 2a .\..uI...<.5...*
//00d0 5a 95 ca b1 8e 54 92 15 a7 8f a2 c7 61 dd df 54 Z....T......a..T
//00e0 cc 0f 9d cb 20 2a 03 ea ac 0c 7f c4 00 a7 41 3e .... *........A>
//00f0 2d 16 22 ac 3d b1 cd 9e c9 6e 0d af 9b f7 de a7 -.".=....n......
//0100 2e 3b e1 80 f2 2c 48 df db 02 03 01 00 01       .;...,H.......

//Encrypted Data Buffer (length = 270)
//0000 90 c9 03 9d a2 c0 33 f0 e9 2e 3d 56 32 bb 36 8c ......3...=V2.6.
//0010 28 30 ea e0 9c 18 f6 29 9e 95 b1 30 89 46 88 ae (0.....)...0.F..
//0020 38 00 95 01 91 ff 37 38 ac 39 08 ee 67 be 95 60 8.....78.9..g..`
//0030 7f 5a ae 2a 1b 53 3b c0 dc 64 13 9c b7 82 c9 03 .Z.*.S;..d......
//0040 ae 60 0e 7b 8d 9f c6 24 46 b5 d1 05 df 82 92 06 .`.{...$F.......
//0050 ea 17 3f 80 9b 2c 47 53 ef f6 18 49 78 ff e9 60 ..?..,GS...Ix..`
//0060 39 21 69 8b f2 06 fa be 98 f8 e6 7b 9a 72 71 ae 9!i........{.rq.
//0070 93 d3 dd aa 8a c5 2c e4 dc 5c 45 9a 64 2d 91 47 ......,..\E.d-.G
//0080 a4 f4 c5 6f cf ba 70 2c ac bf 49 fb 9d 9b 73 2b ...o..p,..I...s+
//0090 df 5b 71 00 a0 2e 6a c3 43 4b fc fb ae 8f 2f c7 .[q...j.CK..../.
//00a0 1d 1b 13 89 a1 80 ae b3 bc d5 a9 f8 fc 48 a7 93 .............H..
//00b0 20 e3 41 a4 db 4c c2 40 9d e5 f6 1a a7 36 4c 73  .A..L.@.....6Ls
//00c0 ac 72 0b de c5 04 0d 4d 90 2c 50 06 f1 31 7a 95 .r.....M.,P..1z.
//00d0 26 17 c4 f8 c8 7a 12 1e 0a 47 b0 f4 2e 82 a7 9e &....z...G......
//00e0 5f 5f 4c 37 f0 12 7b 16 53 cb 1d 19 d8 17 13 39 __L7..{.S......9
//00f0 56 53 f9 e8 37 91 71 6b d9 9e 7e 25 7e 8c 18 0d VS..7.qk..~%~...
//0100 23 d7 9a 56 85 90 f2 88 9a 06 08 0d 7c 51       #..V........|Q

//Signature (length = 16)
//0000 01 00 00 00 c6 bd 2a 5b af 7a 2e 45 00 00 00 00 ......*[.z.E....

//Sending Authentication Token
//0000 4e 54 4c 4d 53 53 50 00 03 00 00 00 18 00 18 00 NTLMSSP.........
//0010 80 00 00 00 de 00 de 00 98 00 00 00 00 00 00 00 ................
//0020 58 00 00 00 1c 00 1c 00 58 00 00 00 0c 00 0c 00 X.......X.......
//0030 74 00 00 00 10 00 10 00 76 01 00 00 35 a2 88 e2 t.......v...5...
//0040 06 01 b1 1d 00 00 00 0f 23 02 14 ae 47 fc e3 07 ........#...G...
//0050 0c f4 97 e3 08 71 eb cf 61 00 64 00 6d 00 69 00 .....q..a.d.m.i.
//0060 6e 00 69 00 73 00 74 00 72 00 61 00 74 00 65 00 n.i.s.t.r.a.t.e.
//0070 75 00 72 00 63 00 67 00 72 00 74 00 68 00 63 00 u.r.c.g.r.t.h.c.
//0080 8b 09 e7 80 37 80 86 73 59 aa 27 ad 72 63 d2 7f ....7..sY.'.rc..
//0090 aa aa aa aa aa aa aa aa 8c c2 c6 d1 ee 99 be a1 ................
//00a0 70 f2 72 5b c3 30 af 53 01 01 00 00 00 00 00 00 p.r[.0.S........
//00b0 15 27 80 39 4e bd d3 01 aa aa aa aa aa aa aa aa .'.9N...........
//00c0 00 00 00 00 02 00 10 00 53 00 41 00 2d 00 53 00 ........S.A.-.S.
//00d0 56 00 52 00 30 00 31 00 01 00 10 00 53 00 41 00 V.R.0.1.....S.A.
//00e0 2d 00 53 00 56 00 52 00 30 00 31 00 04 00 10 00 -.S.V.R.0.1.....
//00f0 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 00 S.A.-.S.V.R.0.1.
//0100 03 00 10 00 53 00 41 00 2d 00 53 00 56 00 52 00 ....S.A.-.S.V.R.
//0110 30 00 31 00 07 00 08 00 15 27 80 39 4e bd d3 01 0.1......'.9N...
//0120 06 00 04 00 02 00 00 00 0a 00 10 00 00 00 00 00 ................
//0130 00 00 00 00 00 00 00 00 00 00 00 00 09 00 26 00 ..............&.
//0140 54 00 45 00 52 00 4d 00 53 00 52 00 56 00 2f 00 T.E.R.M.S.R.V./.
//0150 31 00 30 00 2e 00 31 00 30 00 2e 00 34 00 37 00 1.0...1.0...4.7.
//0160 2e 00 37 00 36 00 00 00 00 00 00 00 00 00 00 00 ..7.6...........
//0170 00 00 00 00 00 00 a5 2e ba 9b 1d 1c ca b1 f5 33 ...............3
//0180 c1 e5 53 77 94 f0                               ..Sw..
//............credssp_send
//................credssp_sizeof_nego_tokens(
//....................credssp_sizeof_nego_token
//credssp_sizeof_pub_key_auth
//................credssp_sizeof_ts_request
//............-- TSRequest
//............-- [0] version
//............-- [1] negoTokens (NegoData)
//............-- [2] authInfo (OCTET STRING)
//............-- [3] pubKeyAuth (OCTET STRING)
//================= Sending Credssp =====================
//0000 30 82 02 c5 a0 03 02 01 02 a1 82 01 96 30 82 01 0............0..
//0010 92 30 82 01 8e a0 82 01 8a 04 82 01 86 4e 54 4c .0...........NTL
//0020 4d 53 53 50 00 03 00 00 00 18 00 18 00 80 00 00 MSSP............
//0030 00 de 00 de 00 98 00 00 00 00 00 00 00 58 00 00 .............X..
//0040 00 1c 00 1c 00 58 00 00 00 0c 00 0c 00 74 00 00 .....X.......t..
//0050 00 10 00 10 00 76 01 00 00 35 a2 88 e2 06 01 b1 .....v...5......
//0060 1d 00 00 00 0f 23 02 14 ae 47 fc e3 07 0c f4 97 .....#...G......
//0070 e3 08 71 eb cf 61 00 64 00 6d 00 69 00 6e 00 69 ..q..a.d.m.i.n.i
//0080 00 73 00 74 00 72 00 61 00 74 00 65 00 75 00 72 .s.t.r.a.t.e.u.r
//0090 00 63 00 67 00 72 00 74 00 68 00 63 00 8b 09 e7 .c.g.r.t.h.c....
//00a0 80 37 80 86 73 59 aa 27 ad 72 63 d2 7f aa aa aa .7..sY.'.rc.....
//00b0 aa aa aa aa aa 8c c2 c6 d1 ee 99 be a1 70 f2 72 .............p.r
//00c0 5b c3 30 af 53 01 01 00 00 00 00 00 00 15 27 80 [.0.S.........'.
//00d0 39 4e bd d3 01 aa aa aa aa aa aa aa aa 00 00 00 9N..............
//00e0 00 02 00 10 00 53 00 41 00 2d 00 53 00 56 00 52 .....S.A.-.S.V.R
//00f0 00 30 00 31 00 01 00 10 00 53 00 41 00 2d 00 53 .0.1.....S.A.-.S
//0100 00 56 00 52 00 30 00 31 00 04 00 10 00 53 00 41 .V.R.0.1.....S.A
//0110 00 2d 00 53 00 56 00 52 00 30 00 31 00 03 00 10 .-.S.V.R.0.1....
//0120 00 53 00 41 00 2d 00 53 00 56 00 52 00 30 00 31 .S.A.-.S.V.R.0.1
//0130 00 07 00 08 00 15 27 80 39 4e bd d3 01 06 00 04 ......'.9N......
//0140 00 02 00 00 00 0a 00 10 00 00 00 00 00 00 00 00 ................
//0150 00 00 00 00 00 00 00 00 00 09 00 26 00 54 00 45 ...........&.T.E
//0160 00 52 00 4d 00 53 00 52 00 56 00 2f 00 31 00 30 .R.M.S.R.V./.1.0
//0170 00 2e 00 31 00 30 00 2e 00 34 00 37 00 2e 00 37 ...1.0...4.7...7
//0180 00 36 00 00 00 00 00 00 00 00 00 00 00 00 00 00 .6..............
//0190 00 00 00 a5 2e ba 9b 1d 1c ca b1 f5 33 c1 e5 53 ............3..S
//01a0 77 94 f0 a3 82 01 22 04 82 01 1e 01 00 00 00 c6 w.....".........
//01b0 bd 2a 5b af 7a 2e 45 00 00 00 00 90 c9 03 9d a2 .*[.z.E.........
//01c0 c0 33 f0 e9 2e 3d 56 32 bb 36 8c 28 30 ea e0 9c .3...=V2.6.(0...
//01d0 18 f6 29 9e 95 b1 30 89 46 88 ae 38 00 95 01 91 ..)...0.F..8....
//01e0 ff 37 38 ac 39 08 ee 67 be 95 60 7f 5a ae 2a 1b .78.9..g..`.Z.*.
//01f0 53 3b c0 dc 64 13 9c b7 82 c9 03 ae 60 0e 7b 8d S;..d.......`.{.
//0200 9f c6 24 46 b5 d1 05 df 82 92 06 ea 17 3f 80 9b ..$F.........?..
//0210 2c 47 53 ef f6 18 49 78 ff e9 60 39 21 69 8b f2 ,GS...Ix..`9!i..
//0220 06 fa be 98 f8 e6 7b 9a 72 71 ae 93 d3 dd aa 8a ......{.rq......
//0230 c5 2c e4 dc 5c 45 9a 64 2d 91 47 a4 f4 c5 6f cf .,..\E.d-.G...o.
//0240 ba 70 2c ac bf 49 fb 9d 9b 73 2b df 5b 71 00 a0 .p,..I...s+.[q..
//0250 2e 6a c3 43 4b fc fb ae 8f 2f c7 1d 1b 13 89 a1 .j.CK..../......
//0260 80 ae b3 bc d5 a9 f8 fc 48 a7 93 20 e3 41 a4 db ........H.. .A..
//0270 4c c2 40 9d e5 f6 1a a7 36 4c 73 ac 72 0b de c5 L.@.....6Ls.r...
//0280 04 0d 4d 90 2c 50 06 f1 31 7a 95 26 17 c4 f8 c8 ..M.,P..1z.&....
//0290 7a 12 1e 0a 47 b0 f4 2e 82 a7 9e 5f 5f 4c 37 f0 z...G......__L7.
//02a0 12 7b 16 53 cb 1d 19 d8 17 13 39 56 53 f9 e8 37 .{.S......9VS..7
//02b0 91 71 6b d9 9e 7e 25 7e 8c 18 0d 23 d7 9a 56 85 .qk..~%~...#..V.
//02c0 90 f2 88 9a 06 08 0d 7c 51                      .......|Q
//================= ================ =====================
//>............-- done credssp_send
//credssp_buffer_free
//............credssp_recv
//================= Receving Credssp =====================
//0000 30 82 01 2b a0 03 02 01 06 a3 82 01 22 04 82 01 0..+........"...
//0010 1e 01 00 00 00 2f e7 46 2e 1b 86 e0 eb 00 00 00 ...../.F........
//0020 00 42 0c b2 66 7d 24 e4 c2 6e 4d 98 e3 10 1c fe .B..f}$..nM.....
//0030 6e da f9 0b 61 dd a7 1b 08 0c a0 ef a3 6e dc ff n...a........n..
//0040 0c 1d 73 57 dc e7 21 7e d3 af 1c ab 3c 50 7c e6 ..sW..!~....<P|.
//0050 9e 86 f7 c4 e5 43 09 c2 57 a2 47 1a be 07 67 f5 .....C..W.G...g.
//0060 55 71 44 dc 02 74 89 d2 39 f3 2d 27 d1 48 eb 31 UqD..t..9.-'.H.1
//0070 ce 0e bc fa ab d2 5e 03 53 89 00 01 e2 7c cb b9 ......^.S....|..
//0080 5d 5a 0b b5 3d a5 97 3c 28 0f bd 23 ed 54 8f 2e ]Z..=..<(..#.T..
//0090 a9 0f f7 3f 5a ac 3c 5f bc 6a e8 f6 27 e9 18 ed ...?Z.<_.j..'...
//00a0 60 6b 11 1a d3 24 b1 76 a2 e7 b1 9b d1 cf 15 d6 `k...$.v........
//00b0 f6 b9 55 ab 97 cb 8d d9 87 e2 b3 6b 7a 37 df 5c ..U........kz7.\
//00c0 de 27 31 a4 6e 58 dd d7 a1 08 c6 66 32 2a c5 a5 .'1.nX.....f2*..
//00d0 21 46 c0 b9 62 b9 ae bb f0 e4 68 fe 17 d6 3e b8 !F..b.....h...>.
//00e0 e5 61 ba 43 0b a3 3c 56 55 ea e5 fc b9 49 b0 bd .a.C..<VU....I..
//00f0 7c 1c 91 2c fa 9c 09 e0 96 86 a6 e8 50 5d fe 88 |..,........P]..
//0100 ae 4e 8a 29 65 e7 05 f8 e3 fd e1 c7 29 a5 b7 bf .N.)e.......)...
//0110 15 c7 1b c2 bb 9b 32 53 1a 93 d4 d5 51 4d f7 eb ......2S....QM..
//0120 a2 bd fc 4a 01 4d b0 a0 80 4c 73 d0 99 b0 7a    ...J.M...Ls...z
//================= ================ =====================
//<............-- TSrequest
//............-- [1] negoTokens (NegoData)
//............-- [2] authInfo (OCTET STRING)
//............-- [3] pubKeyAuth (OCTET STRING)
//............-- done credssp_recv
//credssp_decrypt_public_key_echo
//Encrypted Data Buffer (length = 270)
//0000 42 0c b2 66 7d 24 e4 c2 6e 4d 98 e3 10 1c fe 6e B..f}$..nM.....n
//0010 da f9 0b 61 dd a7 1b 08 0c a0 ef a3 6e dc ff 0c ...a........n...
//0020 1d 73 57 dc e7 21 7e d3 af 1c ab 3c 50 7c e6 9e .sW..!~....<P|..
//0030 86 f7 c4 e5 43 09 c2 57 a2 47 1a be 07 67 f5 55 ....C..W.G...g.U
//0040 71 44 dc 02 74 89 d2 39 f3 2d 27 d1 48 eb 31 ce qD..t..9.-'.H.1.
//0050 0e bc fa ab d2 5e 03 53 89 00 01 e2 7c cb b9 5d .....^.S....|..]
//0060 5a 0b b5 3d a5 97 3c 28 0f bd 23 ed 54 8f 2e a9 Z..=..<(..#.T...
//0070 0f f7 3f 5a ac 3c 5f bc 6a e8 f6 27 e9 18 ed 60 ..?Z.<_.j..'...`
//0080 6b 11 1a d3 24 b1 76 a2 e7 b1 9b d1 cf 15 d6 f6 k...$.v.........
//0090 b9 55 ab 97 cb 8d d9 87 e2 b3 6b 7a 37 df 5c de .U........kz7.\.
//00a0 27 31 a4 6e 58 dd d7 a1 08 c6 66 32 2a c5 a5 21 '1.nX.....f2*..!
//00b0 46 c0 b9 62 b9 ae bb f0 e4 68 fe 17 d6 3e b8 e5 F..b.....h...>..
//00c0 61 ba 43 0b a3 3c 56 55 ea e5 fc b9 49 b0 bd 7c a.C..<VU....I..|
//00d0 1c 91 2c fa 9c 09 e0 96 86 a6 e8 50 5d fe 88 ae ..,........P]...
//00e0 4e 8a 29 65 e7 05 f8 e3 fd e1 c7 29 a5 b7 bf 15 N.)e.......)....
//00f0 c7 1b c2 bb 9b 32 53 1a 93 d4 d5 51 4d f7 eb a2 .....2S....QM...
//0100 bd fc 4a 01 4d b0 a0 80 4c 73 d0 99 b0 7a       ..J.M...Ls...z

//Data Buffer (length = 270)
//0000 31 82 01 0a 02 82 01 01 00 de fa 41 80 c6 5e 85 1..........A..^.
//0010 95 c7 5c cd ab f0 0a 26 2e e9 f6 84 82 27 ad fc ..\....&.....'..
//0020 56 a1 c7 7e 5c 9b 8c f2 e1 87 6f ab ed f2 b4 64 V..~\.....o....d
//0030 70 5f 31 a3 86 69 d6 c5 e9 af 5a 68 8d a2 4d 14 p_1..i....Zh..M.
//0040 69 ae d5 b1 25 9d ee f8 5e 59 8c 44 6b 8b b5 35 i...%...^Y.Dk..5
//0050 23 fe 87 e6 f5 eb 9d b2 24 dd 91 cf 93 ad 6e 1b #.......$.....n.
//0060 22 cb a4 9e 07 0c 7c 97 fa f5 b0 6d 5a 52 a7 c7 ".....|....mZR..
//0070 9a 0a d8 0a d5 10 9e 1c 9d f5 7f 11 46 9f 9f c5 ............F...
//0080 7b 36 9a c8 5b c2 c3 a9 90 62 7e fe 43 65 14 55 {6..[....b~.Ce.U
//0090 42 12 02 7c ce 40 23 70 ae b5 23 96 c2 8f 41 f9 B..|.@#p..#...A.
//00a0 3c ec 1a d7 b4 3c 4d 7d 08 fb 45 2b 27 ca 02 20 <....<M}..E+'.. 
//00b0 86 12 65 03 16 51 0a 6e b4 42 9d f9 1b 73 a4 d3 ..e..Q.n.B...s..
//00c0 9b 5c b0 e6 75 49 0c 88 c4 3c 06 35 2e cf 1d 2a .\..uI...<.5...*
//00d0 5a 95 ca b1 8e 54 92 15 a7 8f a2 c7 61 dd df 54 Z....T......a..T
//00e0 cc 0f 9d cb 20 2a 03 ea ac 0c 7f c4 00 a7 41 3e .... *........A>
//00f0 2d 16 22 ac 3d b1 cd 9e c9 6e 0d af 9b f7 de a7 -.".=....n......
//0100 2e 3b e1 80 f2 2c 48 df db 02 03 01 00 01       .;...,H.......

//credssp_buffer_free
//credssp_encrypt_ts_credentials
//credssp_encode_ts_credentials
//credssp_sizeof_ts_password_creds
//credssp_sizeof_ts_password_creds
//credssp_write_ts_credentials
//credssp_sizeof_ts_password_creds
//credssp_sizeof_ts_password_creds
//credssp_sizeof_ts_password_creds
//credssp_write_ts_password_creds
//credssp_sizeof_ts_password_creds
//Data Buffer (length = 81)
//0000 30 4f a0 03 02 01 01 a1 48 04 46 30 44 a0 02 04 0O......H.F0D...
//0010 00 a1 1e 04 1c 61 00 64 00 6d 00 69 00 6e 00 69 .....a.d.m.i.n.i
//0020 00 73 00 74 00 72 00 61 00 74 00 65 00 75 00 72 .s.t.r.a.t.e.u.r
//0030 00 a2 1e 04 1c 53 00 65 00 63 00 75 00 72 00 65 .....S.e.c.u.r.e
//0040 00 4c 00 69 00 6e 00 75 00 78 00 24 00 34 00 32 .L.i.n.u.x.$.4.2
//0050 00                                              .

//Encrypted Data Buffer (length = 81)
//0000 36 2a 83 63 57 75 ba 08 49 2d c4 61 16 38 fc 53 6*.cWu..I-.a.8.S
//0010 cd 03 72 93 17 58 34 1f d5 7b c3 0f 65 a3 e2 fc ..r..X4..{..e...
//0020 2a b2 81 4a b9 19 2b 26 d5 70 61 7d 7c af 4b 4b *..J..+&.pa}|.KK
//0030 9b 75 eb 7d ad 82 d1 44 8d d8 15 f9 cd 8d 7e 45 .u.}...D......~E
//0040 a0 73 9d b2 52 74 64 59 6e 7e 08 0a e4 96 88 1a .s..RtdYn~......
//0050 0d                                              .

//Signature (length = 16)
//0000 01 00 00 00 a5 33 15 44 49 be 3a 39 01 00 00 00 .....3.DI.:9....

//............credssp_send
//credssp_sizeof_auth_info
//................credssp_sizeof_ts_request
//............-- TSRequest
//............-- [0] version
//............-- [1] negoTokens (NegoData)
//............-- [2] authInfo (OCTET STRING)
//............-- [3] pubKeyAuth (OCTET STRING)
//================= Sending Credssp =====================
//0000 30 6a a0 03 02 01 02 a2 63 04 61 01 00 00 00 a5 0j......c.a.....
//0010 33 15 44 49 be 3a 39 01 00 00 00 36 2a 83 63 57 3.DI.:9....6*.cW
//0020 75 ba 08 49 2d c4 61 16 38 fc 53 cd 03 72 93 17 u..I-.a.8.S..r..
//0030 58 34 1f d5 7b c3 0f 65 a3 e2 fc 2a b2 81 4a b9 X4..{..e...*..J.
//0040 19 2b 26 d5 70 61 7d 7c af 4b 4b 9b 75 eb 7d ad .+&.pa}|.KK.u.}.
//0050 82 d1 44 8d d8 15 f9 cd 8d 7e 45 a0 73 9d b2 52 ..D......~E.s..R
//0060 74 64 59 6e 7e 08 0a e4 96 88 1a 0d             tdYn~.......
//================= ================ =====================
//>............-- done credssp_send
//credssp_buffer_free
//credssp_free

    const char client[] =
        // negotiate
/* 0000 */ "\x30\x37\xa0\x03\x02\x01\x02\xa1\x30\x30\x2e\x30\x2c\xa0\x2a\x04" //07......00.0,.*.
/* 0010 */ "\x28\x4e\x54\x4c\x4d\x53\x53\x50\x00\x01\x00\x00\x00\xb7\x82\x08" //(NTLMSSP........
/* 0020 */ "\xe2\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //.....(.......(..
/* 0030 */ "\x00\x00\x01\xb1\x1d\x00\x00\x00\x0f"                             //...(.....
        // authenticate
/* 0000 */ "\x30\x82\x01\x59\xa0\x03\x02\x01\x02\xa1\x82\x01\x2c\x30\x82\x01" //0..Y........,0..
/* 0010 */ "\x28\x30\x82\x01\x24\xa0\x82\x01\x20\x04\x82\x01\x1c\x4e\x54\x4c" //(0..$... ....NTL
/* 0020 */ "\x4d\x53\x53\x50\x00\x03\x00\x00\x00\x18\x00\x18\x00\x58\x00\x00" //MSSP.........X..
/* 0030 */ "\x00\x70\x00\x70\x00\x70\x00\x00\x00\x0e\x00\x0e\x00\xe0\x00\x00" //.p.p.p..........
/* 0040 */ "\x00\x0c\x00\x0c\x00\xee\x00\x00\x00\x12\x00\x12\x00\xfa\x00\x00" //................
/* 0050 */ "\x00\x10\x00\x10\x00\x0c\x01\x00\x00\xb7\xa2\x88\xe2\x05\x01\x28" //...............(
/* 0060 */ "\x0a\x00\x00\x00\x0f\x50\x89\x6d\x81\x4b\xb6\x53\x8b\x1d\x67\x92" //.....P.m.K.S..g.
/* 0070 */ "\x2e\xd5\x82\x95\x26\x34\xbe\xb5\x6e\xce\xec\x0a\x50\x2d\x29\x63" //....&4..n...P-)c
/* 0080 */ "\x7f\xcc\x5e\xe7\x18\xb8\x6c\xda\xa6\xf0\xf6\x30\x8d\xa6\x41\xcc" //..^...l....0..A.
/* 0090 */ "\x7a\x52\x8e\x7a\xb3\x06\x7d\x0b\xe0\x00\xd5\xf6\x13\x01\x01\x00" //zR.z..}.........
/* 00a0 */ "\x00\x00\x00\x00\x00\x67\x95\x0e\x5a\x4e\x56\x76\xd6\xb8\x6c\xda" //.....g..ZNVv..l.
/* 00b0 */ "\xa6\xf0\xf6\x30\x8d\x00\x00\x00\x00\x01\x00\x08\x00\x57\x00\x49" //...0.........W.I
/* 00c0 */ "\x00\x4e\x00\x37\x00\x02\x00\x08\x00\x57\x00\x49\x00\x4e\x00\x37" //.N.7.....W.I.N.7
/* 00d0 */ "\x00\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x04\x00\x08" //.....w.i.n.7....
/* 00e0 */ "\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x07\x00\x08\x00\x67\x95\x0e" //.w.i.n.7.....g..
/* 00f0 */ "\x5a\x4e\x56\x76\xd6\x00\x00\x00\x00\x00\x00\x00\x00\x49\x00\x74" //ZNVv.........I.t
/* 0100 */ "\x00\x68\x00\x61\x00\x71\x00\x75\x00\x65\x00\x55\x00\x6c\x00\x79" //.h.a.q.u.e.U.l.y
/* 0110 */ "\x00\x73\x00\x73\x00\x65\x00\x54\x00\xe9\x00\x6c\x00\xe9\x00\x6d" //.s.s.e.T...l...m
/* 0120 */ "\x00\x61\x00\x71\x00\x75\x00\x65\x00\x44\xbc\x4d\x7a\x13\x3f\x6b" //.a.q.u.e.D.Mz.?k
/* 0130 */ "\x81\xdb\x1d\x2b\x7b\xbf\x1e\x18\x0f\xa3\x22\x04\x20\x01\x00\x00" //...+{.....". ...
/* 0140 */ "\x00\x89\xe2\xda\x48\x17\x29\xb5\x08\x00\x00\x00\x00\x20\x59\x27" //....H.)...... Y'
/* 0150 */ "\x3f\x08\xd0\xc2\xe4\x75\x66\x10\x49\x7b\xbd\x8d\xf7"             //?....uf.I{...

//         // ts credentials (authinfo)
/* 0000 */ "\x30\x5c\xa0\x03\x02\x01\x02\xa2\x55\x04\x53\x01\x00\x00\x00\xaf" //0.......U.S.....
/* 0010 */ "\xad\x46\x2a\x6a\x9d\xf7\x88\x01\x00\x00\x00\xd5\x4f\xc8\xd0\xbd" //.F*j........O...
/* 0020 */ "\x89\x60\xe0\x71\x60\x31\x7a\xcc\xec\xc5\xbf\x23\x4b\xe5\xf9\xa5" //.`.q`1z....#K...
/* 0030 */ "\x8c\x21\x66\xa6\x78\xda\xd1\xbd\xef\xa4\xfd\x47\xa6\xf1\x56\xa5" //.!f.x......G..V.
/* 0040 */ "\xd9\x52\x72\x92\xfa\x41\xa5\xb4\x9d\x94\xfb\x0e\xe2\x61\xba\xfc" //.Rr..A.......a..
/* 0050 */ "\xd5\xf3\xa7\xb5\x33\xd5\x62\x8d\x93\x18\x54\x39\x8a\xe7"         //....3.b...T9..
        ;

    const char server[] =
        // challenge
/* 0000 */ "\x30\x81\x88\xa0\x03\x02\x01\x02\xa1\x81\x80\x30\x7e\x30\x7c\xa0" //0..........0~0|.
/* 0010 */ "\x7a\x04\x78\x4e\x54\x4c\x4d\x53\x53\x50\x00\x02\x00\x00\x00\x00" //z.xNTLMSSP......
/* 0020 */ "\x00\x00\x00\x38\x00\x00\x00\xb7\x82\x08\xe2\xb8\x6c\xda\xa6\xf0" //...8........l...
/* 0030 */ "\xf6\x30\x8d\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x40\x00\x38" //.0.........@.@.8
/* 0040 */ "\x00\x00\x00\x05\x01\x28\x0a\x00\x00\x00\x0f\x01\x00\x08\x00\x57" //.....(.........W
/* 0050 */ "\x00\x49\x00\x4e\x00\x37\x00\x02\x00\x08\x00\x57\x00\x49\x00\x4e" //.I.N.7.....W.I.N
/* 0060 */ "\x00\x37\x00\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x04" //.7.....w.i.n.7..
/* 0070 */ "\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x07\x00\x08\x00\x67" //...w.i.n.7.....g
/* 0080 */ "\x95\x0e\x5a\x4e\x56\x76\xd6\x00\x00\x00\x00"                     //..ZNVv.....

        // pubauthkey
/* 0000 */ "\x30\x29\xa0\x03\x02\x01\x02\xa3\x22\x04\x20\x01\x00\x00\x00\xa2" //0)......". .....
/* 0010 */ "\xe0\x5b\x50\x97\x8e\x99\x27\x00\x00\x00\x00\xdc\xa7\x0b\xfe\x37" //.[P...'........7
/* 0020 */ "\x45\x3d\x1b\x05\x15\xce\x56\x0a\x54\xa1\xf1"                     //E=....V.T..
        ;

}

