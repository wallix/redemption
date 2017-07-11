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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define RED_TEST_MODULE TestNego
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "utils/log.hpp"
#include "core/RDP/nego.hpp"
#include "core/server_notifier_api.hpp"
#include "test_only/transport/test_transport.hpp"

#include "test_only/lcg_random.hpp"


RED_AUTO_TEST_CASE(TestNego)
{
    LOG(LOG_INFO, "============= Test Nego Client Side ===========");
    const char client[] =
// RDP Negotiation Request
/* 0000 */ "\x03\x00\x00\x2a\x25\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...*%......Cooki
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x74\x65\x73\x74" //e: mstshash=test
/* 0020 */ "\x0d\x0a\x01\x00\x08\x00\x03\x00\x00\x00"                         //..........
        // negotiate
/* 0000 */ "\x30\x37\xa0\x03\x02\x01\x02\xa1\x30\x30\x2e\x30\x2c\xa0\x2a\x04" //07......00.0,.*.
/* 0010 */ "\x28\x4e\x54\x4c\x4d\x53\x53\x50\x00\x01\x00\x00\x00\xb7\x82\x08" //(NTLMSSP........
/* 0020 */ "\xe2\x00\x00\x00\x00\x28\x00\x00\x00\x00\x00\x00\x00\x28\x00\x00" //.....(.......(..
/* 0030 */ "\x00\x05\x01\x28\x0a\x00\x00\x00\x0f"                             //...(.....
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

        // ts credentials (authinfo)
/* 0000 */ "\x30\x5c\xa0\x03\x02\x01\x02\xa2\x55\x04\x53\x01\x00\x00\x00\xaf" //0.......U.S.....
/* 0010 */ "\xad\x46\x2a\x6a\x9d\xf7\x88\x01\x00\x00\x00\xd5\x4f\xc8\xd0\xbd" //.F*j........O...
/* 0020 */ "\x89\x60\xe0\x71\x60\x31\x7a\xcc\xec\xc5\xbf\x23\x4b\xe5\xf9\xa5" //.`.q`1z....#K...
/* 0030 */ "\x8c\x21\x66\xa6\x78\xda\xd1\xbd\xef\xa4\xfd\x47\xa6\xf1\x56\xa5" //.!f.x......G..V.
/* 0040 */ "\xd9\x52\x72\x92\xfa\x41\xa5\xb4\x9d\x94\xfb\x0e\xe2\x61\xba\xfc" //.Rr..A.......a..
/* 0050 */ "\xd5\xf3\xa7\xb5\x33\xd5\x62\x8d\x93\x18\x54\x39\x8a\xe7"         //....3.b...T9..

        ;

    const char server[] =
// RDP Negotiation Response
/* 0000 */ "\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x00\x08\x00\x02" //................
/* 0010 */ "\x00\x00\x00"                                                     //...

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
    TestTransport logtrans(server, sizeof(server)-1, client, sizeof(client)-1);
    logtrans.set_public_key(reinterpret_cast<const uint8_t*>("1245789652325415"), 16);
    char user[] = "Ulysse";
    char domain[] = "Ithaque";
    char pass[] = "Pénélope\x00";
    char host[] = "Télémaque";
    LCGRandom rand(0);
    LCGTime timeobj;
    NullServerNotifier null_server_notifier;
    RdpNego nego(true, logtrans, "test", true, "127.0.0.1", false, rand, timeobj);
    nego.set_identity(user, domain, pass, host);

    nego.send_negotiation_request();
    nego.state = RdpNego::NEGO_STATE_NEGOCIATE;

    const bool server_cert_store = true;

    constexpr size_t pdu_full_packet = AUTOSIZE;
    uint8_t array[pdu_full_packet];
    uint8_t * end = array;
    X224::RecvFactory f(nego.trans, &end, pdu_full_packet);
    nego.recv_connection_confirm(
        server_cert_store,
        ServerCertCheck::always_succeed,
        null_server_notifier,
        "/tmp/certif",
        byte_array{array, end}
    );

    RED_CHECK_EQUAL(nego.state, RdpNego::NEGO_STATE_FINAL);
}

// RED_AUTO_TEST_CASE(TestNego2)
// {
//     LOG(LOG_INFO, "============= Test Nego Server Side ===========");
//     const char client[65000] =
// /* 0000 */ "\x03\x00\x00\x2a\x25\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...*%......Cooki
// /* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x74\x65\x73\x74" //e: mstshash=test
// /* 0020 */ "\x0d\x0a\x01\x00\x08\x00\x03\x00\x00\x00"                         //..........
//         ;
//     const char server[65000] =
// /* 0000 */ "\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x00\x08\x00\x02" //................
// /* 0010 */ "\x00\x00\x00"                                                     //...
//         ;
//     TestTransport logtrans(client, sizeof(client), server, sizeof(server));
//     RdpNego nego(true, &logtrans, "test", true, "127.0.0.1", false);
//     ClientInfo client_info(0, true);
//     nego.recv_resquest(nullptr, client_info, true, true);
// }


RED_AUTO_TEST_CASE(TestSetIdentity) {
    LogTransport null_transport;

    LCGRandom rand(0);
    LCGTime timeobj;
    RdpNego nego(true, null_transport, "test", true, "127.0.0.1", false, rand, timeobj);

    char pass[] = "Password\x00Pass\x00";

    nego.set_identity("Administrateur", "QA", pass, "Server");

    RED_CHECK_EQUAL(0, ::memcmp(pass, nego.password, sizeof(pass)));

    hexdump(pass, sizeof(pass));
}
