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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   T.124 Generic Conference Control (GCC) Unit Test
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestGCC
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
// #define LOGPRINT

#include "transport/test_transport.hpp"
#include "core/RDP/gcc.hpp"

BOOST_AUTO_TEST_CASE(Test_gcc_write_conference_create_request)
{
    const char gcc_user_data[] =
    "\x01\xc0\xd8\x00\x04\x00\x08\x00\x00\x05\x00\x04\x01\xCA\x03\xAA"
    "\x09\x04\x00\x00\xCE\x0E\x00\x00\x45\x00\x4c\x00\x54\x00\x4f\x00"
    "\x4e\x00\x53\x00\x2d\x00\x44\x00\x45\x00\x56\x00\x32\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"
    "\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x01\xCA\x01\x00\x00\x00\x00\x00\x18\x00\x07\x00"
    "\x01\x00\x36\x00\x39\x00\x37\x00\x31\x00\x32\x00\x2d\x00\x37\x00"
    "\x38\x00\x33\x00\x2d\x00\x30\x00\x33\x00\x35\x00\x37\x00\x39\x00"
    "\x37\x00\x34\x00\x2d\x00\x34\x00\x32\x00\x37\x00\x31\x00\x34\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x04\xC0\x0C\x00\x0D\x00\x00\x00"
    "\x00\x00\x00\x00\x02\xC0\x0C\x00\x1B\x00\x00\x00\x00\x00\x00\x00"
    "\x03\xC0\x2C\x00\x03\x00\x00\x00\x72\x64\x70\x64\x72\x00\x00\x00"
    "\x00\x00\x80\x80\x63\x6c\x69\x70\x72\x64\x72\x00\x00\x00\xA0\xC0"
    "\x72\x64\x70\x73\x6e\x64\x00\x00\x00\x00\x00\xc0"
    ;

    const char gcc_conference_create_request_expected[] =
    // conference_create_request_header
    "\x00\x05\x00\x14\x7C\x00\x01"
    "\x81\x2A\x00\x08\x00\x10\x00\x01\xC0"
    "\x00\x44\x75\x63\x61"
    "\x81\x1c" // User data length
    // header
    "\x01\xc0\xd8\x00\x04\x00\x08\x00\x00\x05\x00\x04\x01\xCA\x03\xAA"
    "\x09\x04\x00\x00\xCE\x0E\x00\x00\x45\x00\x4c\x00\x54\x00\x4f\x00"
    "\x4e\x00\x53\x00\x2d\x00\x44\x00\x45\x00\x56\x00\x32\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"
    "\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x01\xCA\x01\x00\x00\x00\x00\x00\x18\x00\x07\x00"
    "\x01\x00\x36\x00\x39\x00\x37\x00\x31\x00\x32\x00\x2d\x00\x37\x00"
    "\x38\x00\x33\x00\x2d\x00\x30\x00\x33\x00\x35\x00\x37\x00\x39\x00"
    "\x37\x00\x34\x00\x2d\x00\x34\x00\x32\x00\x37\x00\x31\x00\x34\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x04\xC0\x0C\x00\x0D\x00\x00\x00"
    "\x00\x00\x00\x00\x02\xC0\x0C\x00\x1B\x00\x00\x00\x00\x00\x00\x00"
    "\x03\xC0\x2C\x00\x03\x00\x00\x00\x72\x64\x70\x64\x72\x00\x00\x00"
    "\x00\x00\x80\x80\x63\x6c\x69\x70\x72\x64\x72\x00\x00\x00\xA0\xC0"
    "\x72\x64\x70\x73\x6e\x64\x00\x00\x00\x00\x00\xc0";


    TestTransport t(
        "", 0,
        gcc_conference_create_request_expected,
        sizeof(gcc_conference_create_request_expected) - sizeof(gcc_user_data),
        256);

    StaticOutStream<65536> stream;
    stream.out_copy_bytes(gcc_user_data, sizeof(gcc_user_data)-1); // -1 to ignore final 0

    StaticOutPerStream<65536> gcc_header;
    GCC::Create_Request_Send(gcc_header, stream.get_offset());
    t.send(gcc_header.get_data(), gcc_header.get_offset());

    constexpr std::size_t sz = sizeof(gcc_conference_create_request_expected)-1;  // -1 to ignore final 0
    uint8_t buf[sz];
    OutStream(buf).out_copy_bytes(gcc_conference_create_request_expected, sz);

    InStream in_stream(buf);
    BOOST_CHECK_NO_THROW(GCC::Create_Request_Recv{in_stream});
//    BOOST_CHECK(t.get_status());
}


BOOST_AUTO_TEST_CASE(Test_gcc_sc_core)
{
    const char expected[] =
        "\x01\x0c\x0c\x00" // TS_UD_HEADER::type = SC_CORE (0x0c01), length = 12 bytes
        "\x04\x00\x08\x00" // TS_UD_SC_CORE::version = 0x0080004
        "\x00\x00\x00\x00" // TS_UD_SC_CORE::clientRequestedProtocols = PROTOCOL_RDP
    ;

    uint8_t buf[12];
    GCC::UserData::SCCore sc_core;
    sc_core.length = 12;
    sc_core.version = 0x0080004;
    sc_core.clientRequestedProtocols = 0;
    OutStream out_stream(buf);
    sc_core.emit(out_stream);
    BOOST_CHECK_EQUAL(12, out_stream.get_offset());
    BOOST_CHECK(0 == memcmp(expected, out_stream.get_data(), 12));

    GCC::UserData::SCCore sc_core2;

    InStream in_stream(buf);
    sc_core2.recv(in_stream);
    BOOST_CHECK_EQUAL(SC_CORE, sc_core2.userDataType);
    BOOST_CHECK_EQUAL(12, sc_core2.length);
    BOOST_CHECK_EQUAL(0x0080004, sc_core2.version);
    BOOST_CHECK_EQUAL(0, sc_core2.clientRequestedProtocols);

    sc_core2.log("Server Received");
}

BOOST_AUTO_TEST_CASE(Test_gcc_sc_net)
{
    const char expected[] =
        "\x03\x0c\x10\x00" // TS_UD_HEADER::type = SC_NET (0x0c03), length = 16 bytes
        "\xeb\x03"         // TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
        "\x03\x00"         // TS_UD_SC_NET::channelCount = 3
        "\xec\x03"         // channel0 = 0x3ec = 1004 (rdpdr)
        "\xed\x03"         // channel1 = 0x3ed = 1005 (cliprdr)
        "\xee\x03"         // channel2 = 0x3ee = 1006 (rdpsnd)
        "\x00\x00"         // padding
    ;

    uint8_t buf[16];
    OutStream out_stream(buf);
    GCC::UserData::SCNet sc_net;
    sc_net.MCSChannelId = 1003;
    sc_net.channelCount = 3;
    sc_net.channelDefArray[0].id = 1004;
    sc_net.channelDefArray[1].id = 1005;
    sc_net.channelDefArray[2].id = 1006;
    sc_net.emit(out_stream);
    BOOST_CHECK_EQUAL(16, out_stream.get_offset());
    BOOST_CHECK(0 == memcmp(expected, out_stream.get_data(), 12));

    GCC::UserData::SCNet sc_net2;

    const bool bogus_sc_net_size = false;
    InStream in_stream(buf);
    BOOST_CHECK_NO_THROW(sc_net2.recv(in_stream, bogus_sc_net_size));
    BOOST_CHECK_EQUAL(SC_NET, sc_net2.userDataType);
    BOOST_CHECK_EQUAL(1003, sc_net2.MCSChannelId);
    BOOST_CHECK_EQUAL(3, sc_net2.channelCount);
    BOOST_CHECK_EQUAL(1004, sc_net2.channelDefArray[0].id);
    BOOST_CHECK_EQUAL(1005, sc_net2.channelDefArray[1].id);
    BOOST_CHECK_EQUAL(1006, sc_net2.channelDefArray[2].id);

    sc_net2.log("Server Received");
}


BOOST_AUTO_TEST_CASE(Test_gcc_user_data_cs_net)
{
    const char indata[] =
        "\x03\xc0"         // CS_NET
        "\x20\x00"         // 32 bytes user Data
        "\x02\x00\x00\x00" // ChannelCount
        "\x63\x6c\x69\x70\x72\x64\x72\x00" // "cliprdr"
        "\x00\x00\xa0\xc0" // = CHANNEL_OPTION_INITIALIZED
                           // | CHANNEL_OPTION_ENCRYPT_RDP
                           // | CHANNEL_OPTION_COMPRESS_RDP
                           // | CHANNEL_OPTION_SHOW_PROTOCOL
        "\x72\x64\x70\x64\x72\x00\x00\x00" // "rdpdr"
        "\x00\x00\x80\x80" // = CHANNEL_OPTION_INITIALIZED
                           // | CHANNEL_OPTION_COMPRESS_RDP
    ;

    constexpr std::size_t sz = sizeof(indata) - 1;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    InStream stream(buf, sz);
    GCC::UserData::CSNet cs_net;
    cs_net.recv(stream);
    BOOST_CHECK_EQUAL(CS_NET, cs_net.userDataType);
    BOOST_CHECK_EQUAL(32, cs_net.length);
    BOOST_CHECK_EQUAL(2, cs_net.channelCount);
    BOOST_CHECK_EQUAL('c', cs_net.channelDefArray[0].name[0]);
    BOOST_CHECK_EQUAL(0, memcmp("cliprdr\0", cs_net.channelDefArray[0].name, 8));
    BOOST_CHECK_EQUAL( GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                     | GCC::UserData::CSNet::CHANNEL_OPTION_ENCRYPT_RDP
                     | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP
                     | GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                     , cs_net.channelDefArray[0].options);
    BOOST_CHECK_EQUAL(0, memcmp("rdpdr\0\0\0", cs_net.channelDefArray[1].name, 8));
    BOOST_CHECK_EQUAL( GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                     | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP
                     , cs_net.channelDefArray[1].options);

    cs_net.log("Client Received");
}

BOOST_AUTO_TEST_CASE(Test_gcc_user_data_sc_sec1_ServerProprietaryCertificate)
{
    const char indata[] =
        /* 0000 */ "\x02\x0c\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    ;

    constexpr std::size_t sz = sizeof(indata) - 1;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    InStream stream(buf, sz);
    GCC::UserData::SCSecurity sc_sec1;
    sc_sec1.recv(stream);
    BOOST_CHECK_EQUAL(SC_SECURITY, sc_sec1.userDataType);
    BOOST_CHECK_EQUAL(sizeof(indata) - 1, sc_sec1.length);
    BOOST_CHECK_EQUAL(0, sc_sec1.encryptionMethod);
    BOOST_CHECK_EQUAL(0, sc_sec1.encryptionLevel);

    sc_sec1.log("Server Received");
}


BOOST_AUTO_TEST_CASE(Test_gcc_user_data_sc_sec1_no_crypt)
{
    const char indata[] =
        /* 0000 */ "\x02\x0c\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    ;

    constexpr std::size_t sz = sizeof(indata) - 1;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    InStream stream(buf, sz);
    GCC::UserData::SCSecurity sc_sec1;
    sc_sec1.recv(stream);
    BOOST_CHECK_EQUAL(SC_SECURITY, sc_sec1.userDataType);
    BOOST_CHECK_EQUAL(sizeof(indata) - 1, sc_sec1.length);
    BOOST_CHECK_EQUAL(0, sc_sec1.encryptionMethod);
    BOOST_CHECK_EQUAL(0, sc_sec1.encryptionLevel);

    sc_sec1.log("Server Received");
}


BOOST_AUTO_TEST_CASE(Test_gcc_user_data_sc_sec1_rdp5)
{
    const char indata[] =
    // SC_SECURITY tag=0c02 length=1410
    /* 0000 */ "\x02\x0c\x82\x05\x01\x00\x00\x00\x02\x00\x00\x00\x20\x00\x00\x00" //............ ...
    /* 0010 */ "\x4e\x05\x00\x00\x5e\x69\xf3\x27\x93\x2d\x98\x35\x0e\x09\x1f\xe6" //N...^i.'.-.5....
    /* 0020 */ "\xce\xea\xd9\x07\x58\x2f\x66\x6c\xd6\xa4\x32\x45\x1e\x61\x7a\xba" //....X/fl..2E.az.
    /* 0030 */ "\x95\x8c\xfd\x23\x02\x00\x00\x80\x02\x00\x00\x00\x83\x01\x00\x00" //...#............
    /* 0040 */ "\x30\x82\x01\x7f\x30\x82\x01\x2d\xa0\x03\x02\x01\x02\x02\x08\x01" //0...0..-........
    /* 0050 */ "\x9d\xf1\x14\x5e\x65\xc9\xf0\x30\x09\x06\x05\x2b\x0e\x03\x02\x1d" //...^e..0...+....
    /* 0060 */ "\x05\x00\x30\x3e\x31\x3c\x30\x19\x06\x03\x55\x04\x07\x1e\x12\x00" //..0>1<0...U.....
    /* 0070 */ "\x57\x00\x4f\x00\x52\x00\x4b\x00\x47\x00\x52\x00\x4f\x00\x55\x00" //W.O.R.K.G.R.O.U.
    /* 0080 */ "\x50\x30\x1f\x06\x03\x55\x04\x03\x1e\x18\x00\x57\x00\x49\x00\x4e" //P0...U.....W.I.N
    /* 0090 */ "\x00\x32\x00\x4b\x00\x30\x00\x2d\x00\x31\x00\x2d\x00\x53\x00\x52" //.2.K.0.-.1.-.S.R
    /* 00a0 */ "\x00\x56\x30\x1e\x17\x0d\x37\x30\x30\x33\x32\x32\x31\x30\x33\x31" //.V0...7003221031
    /* 00b0 */ "\x35\x36\x5a\x17\x0d\x34\x39\x30\x33\x32\x32\x31\x30\x33\x31\x35" //56Z..49032210315
    /* 00c0 */ "\x36\x5a\x30\x3e\x31\x3c\x30\x19\x06\x03\x55\x04\x07\x1e\x12\x00" //6Z0>1<0...U.....
    /* 00d0 */ "\x57\x00\x4f\x00\x52\x00\x4b\x00\x47\x00\x52\x00\x4f\x00\x55\x00" //W.O.R.K.G.R.O.U.
    /* 00e0 */ "\x50\x30\x1f\x06\x03\x55\x04\x03\x1e\x18\x00\x57\x00\x49\x00\x4e" //P0...U.....W.I.N
    /* 00f0 */ "\x00\x32\x00\x4b\x00\x30\x00\x2d\x00\x31\x00\x2d\x00\x53\x00\x52" //.2.K.0.-.1.-.S.R
    /* 0100 */ "\x00\x56\x30\x5c\x30\x0d\x06\x09\x2a\x86\x48\x86\xf7\x0d\x01\x01" //.V0\0...*.H.....
    /* 0110 */ "\x01\x05\x00\x03\x4b\x00\x30\x48\x02\x41\x00\xc4\x0b\xae\x24\x38" //....K.0H.A....$8
    /* 0120 */ "\x86\x8e\x5b\x61\x17\x5a\x60\x8a\xa1\xe1\xa3\xce\xcd\xf3\x8f\xea" //..[a.Z`.........
    /* 0130 */ "\x7f\x3f\xcc\xed\x60\xbd\x6f\x52\xb3\xcd\xab\xc5\x98\xc0\xcb\xa1" //.?..`.oR........
    /* 0140 */ "\x68\xcc\xfa\x32\xc0\x84\xc7\xd9\x0e\x11\x4c\x59\xc5\x15\x52\x25" //h..2......LY..R%
    /* 0150 */ "\x5a\xcc\xa4\xb6\xe7\x1c\xcc\x8f\x69\xb5\xe1\x02\x03\x01\x00\x01" //Z.......i.......
    /* 0160 */ "\xa3\x13\x30\x11\x30\x0f\x06\x03\x55\x1d\x13\x04\x08\x30\x06\x01" //..0.0...U....0..
    /* 0170 */ "\x01\xff\x02\x01\x00\x30\x09\x06\x05\x2b\x0e\x03\x02\x1d\x05\x00" //.....0...+......
    /* 0180 */ "\x03\x41\x00\x5e\x3f\x4e\xbd\x1a\x31\xc9\xbb\x58\xfc\x33\x9e\x61" //.A.^?N..1..X.3.a
    /* 0190 */ "\xe9\xf3\x1d\xe9\xcb\x98\x57\x02\x74\xce\x43\x10\x43\xf0\x8b\x6b" //......W.t.C.C..k
    /* 01a0 */ "\xb0\x59\xfd\x71\xa4\x66\xa1\x2c\x68\xbf\xa9\xdb\x54\x74\x97\x61" //.Y.q.f.,h...Tt.a
    /* 01b0 */ "\x26\xf0\x7a\x2c\x79\xcf\xef\x83\x6d\x65\x1a\x84\x08\x3c\xaa\xef" //&.z,y...me...<..
    /* 01c0 */ "\x08\x29\x56\xab\x03\x00\x00\x30\x82\x03\xa7\x30\x82\x03\x55\xa0" //.)V....0...0..U.
    /* 01d0 */ "\x03\x02\x01\x02\x02\x05\x01\x00\x00\x00\x01\x30\x09\x06\x05\x2b" //...........0...+
    /* 01e0 */ "\x0e\x03\x02\x1d\x05\x00\x30\x3e\x31\x3c\x30\x19\x06\x03\x55\x04" //......0>1<0...U.
    /* 01f0 */ "\x07\x1e\x12\x00\x57\x00\x4f\x00\x52\x00\x4b\x00\x47\x00\x52\x00" //....W.O.R.K.G.R.
    /* 0200 */ "\x4f\x00\x55\x00\x50\x30\x1f\x06\x03\x55\x04\x03\x1e\x18\x00\x57" //O.U.P0...U.....W
    /* 0210 */ "\x00\x49\x00\x4e\x00\x32\x00\x4b\x00\x30\x00\x2d\x00\x31\x00\x2d" //.I.N.2.K.0.-.1.-
    /* 0220 */ "\x00\x53\x00\x52\x00\x56\x30\x1e\x17\x0d\x37\x39\x31\x32\x33\x31" //.S.R.V0...791231
    /* 0230 */ "\x32\x33\x30\x30\x30\x30\x5a\x17\x0d\x33\x38\x30\x31\x31\x39\x30" //230000Z..3801190
    /* 0240 */ "\x33\x31\x34\x30\x37\x5a\x30\x81\xa2\x31\x81\x9f\x30\x27\x06\x03" //31407Z0..1..0'..
    /* 0250 */ "\x55\x04\x03\x1e\x20\x00\x6e\x00\x63\x00\x61\x00\x6c\x00\x72\x00" //U... .n.c.a.l.r.
    /* 0260 */ "\x70\x00\x63\x00\x3a\x00\x57\x00\x49\x00\x4e\x00\x32\x00\x4b\x00" //p.c.:.W.I.N.2.K.
    /* 0270 */ "\x30\x00\x2d\x00\x31\x30\x2f\x06\x03\x55\x04\x07\x1e\x28\x00\x6e" //0.-.10/..U...(.n
    /* 0280 */ "\x00\x63\x00\x61\x00\x6c\x00\x72\x00\x70\x00\x63\x00\x3a\x00\x57" //.c.a.l.r.p.c.:.W
    /* 0290 */ "\x00\x49\x00\x4e\x00\x32\x00\x4b\x00\x30\x00\x2d\x00\x31\x00\x2d" //.I.N.2.K.0.-.1.-
    /* 02a0 */ "\x00\x53\x00\x52\x00\x56\x30\x43\x06\x03\x55\x04\x05\x1e\x3c\x00" //.S.R.V0C..U...<.
    /* 02b0 */ "\x31\x00\x42\x00\x63\x00\x4b\x00\x65\x00\x54\x00\x53\x00\x44\x00" //1.B.c.K.e.T.S.D.
    /* 02c0 */ "\x74\x00\x74\x00\x56\x00\x62\x00\x64\x00\x53\x00\x6b\x00\x4a\x00" //t.t.V.b.d.S.k.J.
    /* 02d0 */ "\x41\x00\x6b\x00\x6a\x00\x48\x00\x4a\x00\x44\x00\x64\x00\x34\x00" //A.k.j.H.J.D.d.4.
    /* 02e0 */ "\x76\x00\x72\x00\x4d\x00\x3d\x00\x0d\x00\x0a\x30\x5c\x30\x0d\x06" //v.r.M.=....0\0..
    /* 02f0 */ "\x09\x2a\x86\x48\x86\xf7\x0d\x01\x01\x04\x05\x00\x03\x4b\x00\x30" //.*.H.........K.0
    /* 0300 */ "\x48\x02\x41\x00\xab\xd3\xfc\x7a\x40\xb5\x33\x01\xce\xd7\x00\x4a" //H.A....z@.3....J
    /* 0310 */ "\x09\x6a\x49\x19\xb7\xa2\x1b\xe9\x65\xe8\xbb\x6f\x90\x54\x77\x21" //.jI.....e..o.Tw!
    /* 0320 */ "\x00\x6a\xeb\x40\xb2\x86\xdb\xf5\x3b\x3b\x03\xc2\xeb\x4c\xce\x65" //.j.@....;;...L.e
    /* 0330 */ "\x1f\x6f\x33\xa9\x8a\x82\x11\x54\x1b\xbe\x38\xc6\x62\x87\xaf\xf3" //.o3....T..8.b...
    /* 0340 */ "\x5f\xc0\x04\x1f\x02\x03\x01\x00\x01\xa3\x82\x01\xd7\x30\x82\x01" //_............0..
    /* 0350 */ "\xd3\x30\x14\x06\x09\x2b\x06\x01\x04\x01\x82\x37\x12\x04\x01\x01" //.0...+.....7....
    /* 0360 */ "\xff\x04\x04\x01\x00\x05\x00\x30\x3c\x06\x09\x2b\x06\x01\x04\x01" //.......0<..+....
    /* 0370 */ "\x82\x37\x12\x02\x01\x01\xff\x04\x2c\x4d\x00\x69\x00\x63\x00\x72" //.7......,M.i.c.r
    /* 0380 */ "\x00\x6f\x00\x73\x00\x6f\x00\x66\x00\x74\x00\x20\x00\x43\x00\x6f" //.o.s.o.f.t. .C.o
    /* 0390 */ "\x00\x72\x00\x70\x00\x6f\x00\x72\x00\x61\x00\x74\x00\x69\x00\x6f" //.r.p.o.r.a.t.i.o
    /* 03a0 */ "\x00\x6e\x00\x00\x00\x30\x81\xcd\x06\x09\x2b\x06\x01\x04\x01\x82" //.n...0....+.....
    /* 03b0 */ "\x37\x12\x05\x01\x01\xff\x04\x81\xbc\x00\x30\x00\x00\x01\x00\x00" //7.........0.....
    /* 03c0 */ "\x00\x02\x00\x00\x00\x0c\x04\x00\x00\x1c\x00\x4a\x00\x66\x00\x4a" //...........J.f.J
    /* 03d0 */ "\x00\xb0\x00\x01\x00\x33\x00\x64\x00\x32\x00\x36\x00\x37\x00\x39" //.....3.d.2.6.7.9
    /* 03e0 */ "\x00\x35\x00\x34\x00\x2d\x00\x65\x00\x65\x00\x62\x00\x37\x00\x2d" //.5.4.-.e.e.b.7.-
    /* 03f0 */ "\x00\x31\x00\x31\x00\x64\x00\x31\x00\x2d\x00\x62\x00\x39\x00\x34" //.1.1.d.1.-.b.9.4
    /* 0400 */ "\x00\x65\x00\x2d\x00\x30\x00\x30\x00\x63\x00\x30\x00\x34\x00\x66" //.e.-.0.0.c.0.4.f
    /* 0410 */ "\x00\x61\x00\x33\x00\x30\x00\x38\x00\x30\x00\x64\x00\x00\x00\x33" //.a.3.0.8.0.d...3
    /* 0420 */ "\x00\x64\x00\x32\x00\x36\x00\x37\x00\x39\x00\x35\x00\x34\x00\x2d" //.d.2.6.7.9.5.4.-
    /* 0430 */ "\x00\x65\x00\x65\x00\x62\x00\x37\x00\x2d\x00\x31\x00\x31\x00\x64" //.e.e.b.7.-.1.1.d
    /* 0440 */ "\x00\x31\x00\x2d\x00\x62\x00\x39\x00\x34\x00\x65\x00\x2d\x00\x30" //.1.-.b.9.4.e.-.0
    /* 0450 */ "\x00\x30\x00\x63\x00\x30\x00\x34\x00\x66\x00\x61\x00\x33\x00\x30" //.0.c.0.4.f.a.3.0
    /* 0460 */ "\x00\x38\x00\x30\x00\x64\x00\x00\x00\x00\x00\x00\x10\x00\x80\xd4" //.8.0.d..........
    /* 0470 */ "\x00\x00\x00\x00\x00\x30\x7a\x06\x09\x2b\x06\x01\x04\x01\x82\x37" //.....0z..+.....7
    /* 0480 */ "\x12\x06\x01\x01\xff\x04\x6a\x00\x30\x00\x00\x00\x00\x1a\x00\x4a" //......j.0......J
    /* 0490 */ "\x00\x57\x00\x49\x00\x4e\x00\x32\x00\x4b\x00\x30\x00\x2d\x00\x31" //.W.I.N.2.K.0.-.1
    /* 04a0 */ "\x00\x2d\x00\x53\x00\x52\x00\x56\x00\x00\x00\x35\x00\x32\x00\x33" //.-.S.R.V...5.2.3
    /* 04b0 */ "\x00\x34\x00\x30\x00\x2d\x00\x30\x00\x30\x00\x36\x00\x2d\x00\x34" //.4.0.-.0.0.6.-.4
    /* 04c0 */ "\x00\x34\x00\x36\x00\x32\x00\x36\x00\x30\x00\x36\x00\x2d\x00\x38" //.4.6.2.6.0.6.-.8
    /* 04d0 */ "\x00\x30\x00\x32\x00\x30\x00\x31\x00\x00\x00\x57\x00\x4f\x00\x52" //.0.2.0.1...W.O.R
    /* 04e0 */ "\x00\x4b\x00\x47\x00\x52\x00\x4f\x00\x55\x00\x50\x00\x00\x00\x00" //.K.G.R.O.U.P....
    /* 04f0 */ "\x00\x30\x31\x06\x03\x55\x1d\x23\x01\x01\xff\x04\x27\x30\x25\xa1" //.01..U.#....'0%.
    /* 0500 */ "\x1c\xa4\x1a\x57\x00\x49\x00\x4e\x00\x32\x00\x4b\x00\x30\x00\x2d" //...W.I.N.2.K.0.-
    /* 0510 */ "\x00\x31\x00\x2d\x00\x53\x00\x52\x00\x56\x00\x00\x00\x82\x05\x01" //.1.-.S.R.V......
    /* 0520 */ "\x00\x00\x00\x01\x30\x09\x06\x05\x2b\x0e\x03\x02\x1d\x05\x00\x03" //....0...+.......
    /* 0530 */ "\x41\x00\x80\x34\x47\xa7\xe6\xe7\x84\xa2\x53\x48\xc9\x46\x4d\xd9" //A..4G.....SH.FM.
    /* 0540 */ "\xaf\xb2\x5d\x0b\x09\x1a\xb3\xea\xe5\xf5\x6d\xd1\xdb\x21\x59\xf4" //..].......m..!Y.
    /* 0550 */ "\xd1\xf4\xbb\xd4\x6a\x85\x86\x4f\xe3\xfc\xe5\xa2\x34\xcf\x04\xf8" //....j..O....4...
    /* 0560 */ "\xfc\x69\x0e\xb3\x1c\xd2\x4a\x02\x18\xea\x8e\x89\x71\x7f\x32\x52" //.i....J.....q.2R
    /* 0570 */ "\xb5\x6b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //.k..............
    /* 0580 */ "\x00\x00"                                                         //..
    ;

    constexpr std::size_t sz = sizeof(indata) - 1;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    InStream stream(buf, sz);
    GCC::UserData::SCSecurity sc_sec1;
    sc_sec1.recv(stream);
    BOOST_CHECK_EQUAL(SC_SECURITY, sc_sec1.userDataType);
    BOOST_CHECK_EQUAL(sizeof(indata) - 1, sc_sec1.length);
    BOOST_CHECK_EQUAL(1, sc_sec1.encryptionMethod);
    BOOST_CHECK_EQUAL(2, sc_sec1.encryptionLevel);
    BOOST_CHECK_EQUAL(32, sc_sec1.serverRandomLen);
    BOOST_CHECK_EQUAL(1358, sc_sec1.serverCertLen);
//    hexdump_c(sc_sec1.serverRandom, 32);
    BOOST_CHECK_EQUAL(0, memcmp(
                        "\x5e\x69\xf3\x27\x93\x2d\x98\x35\x0e\x09\x1f\xe6\xce\xea\xd9\x07"
                        "\x58\x2f\x66\x6c\xd6\xa4\x32\x45\x1e\x61\x7a\xba\x95\x8c\xfd\x23"
                     , sc_sec1.serverRandom, sc_sec1.serverRandomLen));
    BOOST_CHECK_EQUAL(static_cast<uint32_t>(GCC::UserData::SCSecurity::CERT_CHAIN_VERSION_2), sc_sec1.dwVersion);
    BOOST_CHECK_EQUAL(true, sc_sec1.temporary);

    sc_sec1.log("Server Received");

}

BOOST_AUTO_TEST_CASE(Test_gcc_user_data_sc_sec1_rdp4)
{
    const char indata[] =
        // SC_SECURITY tag=0c02 length=236
        /* 0000 */ "\x02\x0c\xec\x00\x01\x00\x00\x00\x01\x00\x00\x00\x20\x00\x00\x00" //............ ...
        /* 0010 */ "\xb8\x00\x00\x00\x73\xee\x92\x99\x02\x50\xfd\xe7\x89\xec\x2a\x83" //....s....P....*.
        /* 0020 */ "\xbd\xb4\xde\x56\xc4\x61\xb9\x5b\x05\x3d\xd9\xc6\x84\xe9\x83\x69" //...V.a.[.=.....i
        /* 0030 */ "\x25\xd4\x82\x3f\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00" //%..?............
        /* 0040 */ "\x06\x00\x5c\x00\x52\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00\x00" //..\.RSA1H.......
        /* 0050 */ "\x3f\x00\x00\x00\x01\x00\x01\x00\x67\xab\x0e\x6a\x9f\xd6\x2b\xa3" //?.......g..j..+.
        /* 0060 */ "\x32\x2f\x41\xd1\xce\xee\x61\xc3\x76\x0b\x26\x11\x70\x48\x8a\x8d" //2/A...a.v.&.pH..
        /* 0070 */ "\x23\x81\x95\xa0\x39\xf7\x5b\xaa\x3e\xf1\xed\xb8\xc4\xee\xce\x5f" //#...9.[.>......_
        /* 0080 */ "\x6a\xf5\x43\xce\x5f\x60\xca\x6c\x06\x75\xae\xc0\xd6\xa4\x0c\x92" //j.C._`.l.u......
        /* 0090 */ "\xa4\xc6\x75\xea\x64\xb2\x50\x5b\x00\x00\x00\x00\x00\x00\x00\x00" //..u.d.P[........
        /* 00a0 */ "\x08\x00\x48\x00\x6a\x41\xb1\x43\xcf\x47\x6f\xf1\xe6\xcc\xa1\x72" //..H.jA.C.Go....r
        /* 00b0 */ "\x97\xd9\xe1\x85\x15\xb3\xc2\x39\xa0\xa6\x26\x1a\xb6\x49\x01\xfa" //.......9..&..I..
        /* 00c0 */ "\xa6\xda\x60\xd7\x45\xf7\x2c\xee\xe4\x8e\x64\x2e\x37\x49\xf0\x4c" //..`.E.,...d.7I.L
        /* 00d0 */ "\x94\x6f\x08\xf5\x63\x4c\x56\x29\x55\x5a\x63\x41\x2c\x20\x65\x95" //.o..cLV)UZcA, e.
        /* 00e0 */ "\x99\xb1\x15\x7c\x00\x00\x00\x00\x00\x00\x00\x00"                 //...|........
    ;

    constexpr std::size_t sz = sizeof(indata) - 1;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    InStream stream(buf, sz);
    GCC::UserData::SCSecurity sc_sec1;
    sc_sec1.recv(stream);
    BOOST_CHECK_EQUAL(SC_SECURITY, sc_sec1.userDataType);
    BOOST_CHECK_EQUAL(sizeof(indata) - 1, sc_sec1.length);
    BOOST_CHECK_EQUAL(1, sc_sec1.encryptionMethod);
    BOOST_CHECK_EQUAL(1, sc_sec1.encryptionLevel);
    BOOST_CHECK_EQUAL(32, sc_sec1.serverRandomLen);
    BOOST_CHECK_EQUAL(184, sc_sec1.serverCertLen);
//    hexdump_c(sc_sec1.serverRandom, 32);
    BOOST_CHECK_EQUAL(0, memcmp(
                    "\x73\xee\x92\x99\x02\x50\xfd\xe7\x89\xec\x2a\x83\xbd\xb4\xde\x56"
                    "\xc4\x61\xb9\x5b\x05\x3d\xd9\xc6\x84\xe9\x83\x69\x25\xd4\x82\x3f"
                             , sc_sec1.serverRandom, sc_sec1.serverRandomLen));
    BOOST_CHECK_EQUAL(static_cast<uint32_t>(GCC::UserData::SCSecurity::CERT_CHAIN_VERSION_1), sc_sec1.dwVersion);
    BOOST_CHECK_EQUAL(false, sc_sec1.temporary);
    BOOST_CHECK_EQUAL(0x31415352, sc_sec1.proprietaryCertificate.RSAPK.magic); // magic is really ASCII string 'RSA1'

    sc_sec1.log("Server Received");
}

BOOST_AUTO_TEST_CASE(Test_gcc_user_data_cs_cluster)
{
    const char indata[] =
        "\x04\xc0"         // CS_CLUSTER
        "\x0c\x00"         // 12 bytes user Data
        "\x0d\x00\x00\x00" // TS_UD_CS_CLUSTER::Flags = 0x0d
        // 0x0d
        // = 0x03 << 2 | 0x01
        // = REDIRECTION_VERSION4 << 2 | REDIRECTION_SUPPORTED
        "\x00\x00\x00\x00" // TS_UD_CS_CLUSTER::RedirectedSessionID
    ;

    constexpr std::size_t sz = sizeof(indata) - 1;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    InStream stream(buf, sz);
    GCC::UserData::CSCluster cs_cluster;
    cs_cluster.recv(stream);
    BOOST_CHECK_EQUAL(CS_CLUSTER, cs_cluster.userDataType);
    BOOST_CHECK_EQUAL(12, cs_cluster.length);
    BOOST_CHECK_EQUAL(13, cs_cluster.flags);
    BOOST_CHECK_EQUAL(0, cs_cluster.redirectedSessionID);

    cs_cluster.log("Client Received");
}


BOOST_AUTO_TEST_CASE(Test_gcc_user_data_cs_core)
{
    const char indata[] =
        "\x01\xc0"         // TS_UD_HEADER::type = CS_CORE (0xc001)
        "\xd8\x00"         // length = 216 bytes
        "\x04\x00\x08\x00" // TS_UD_CS_CORE::version = 0x0008004
        "\x00\x05"         // TS_UD_CS_CORE::desktopWidth = 1280
        "\x00\x04"         // TS_UD_CS_CORE::desktopHeight = 1024
        "\x01\xca"         // TS_UD_CS_CORE::colorDepth = RNS_UD_COLOR_8BPP (0xca01)
        "\x03\xaa"         // TS_UD_CS_CORE::SASSequence
        "\x09\x04\x00\x00" // TS_UD_CS_CORE::keyboardLayout = 0x409 = 1033 = English (US)
        "\xce\x0e\x00\x00" // TS_UD_CS_CORE::clientBuild = 3790
                           // TS_UD_CS_CORE::clientName = ELTONS-TEST2
        "\x45\x00\x4c\x00\x54\x00\x4f\x00\x4e\x00\x53\x00\x2d\x00\x44\x00"
        "\x45\x00\x56\x00\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x04\x00\x00\x00" // TS_UD_CS_CORE::keyboardType
        "\x00\x00\x00\x00" // TS_UD_CS_CORE::keyboardSubtype
        "\x0c\x00\x00\x00" // TS_UD_CS_CORE::keyboardFunctionKey
                           // TS_UD_CS_CORE::imeFileName = ""
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x01\xca"         // TS_UD_CS_CORE::postBeta2ColorDepth = RNS_UD_COLOR_8BPP (0xca01)
        "\x01\x00"         // TS_UD_CS_CORE::clientProductId
        "\x00\x00\x00\x00" // TS_UD_CS_CORE::serialNumber
        "\x18\x00"         // TS_UD_CS_CORE::highColorDepth = 24 bpp
        "\x07\x00"         // TS_UD_CS_CORE::supportedColorDepths
                           //0x07 = 0x01 | 0x02 | 0x04 = RNS_UD_24BPP_SUPPORT | RNS_UD_16BPP_SUPPORT | RNS_UD_15BPP_SUPPORT
        "\x01\x00"         // TS_UD_CS_CORE::earlyCapabilityFlags
                           //0x01 = RNS_UD_CS_SUPPORT_ERRINFO_PDU
                           //TS_UD_CS_CORE::clientDigProductId = "69712-783-0357974-42714"
        "\x36\x00\x39\x00\x37\x00\x31\x00\x32\x00\x2d\x00\x37\x00\x38\x00"
        "\x33\x00\x2d\x00\x30\x00\x33\x00\x35\x00\x37\x00\x39\x00\x37\x00"
        "\x34\x00\x2d\x00\x34\x00\x32\x00\x37\x00\x31\x00\x34\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00"             // TS_UD_CS_CORE::connectionType = 0 (not used as RNS_UD_CS_VALID_CONNECTION_TYPE not set)
        "\x00"             // TS_UD_CS_CORE::pad1octet
        "\x00\x00\x00\x00" // TS_UD_CS_CORE::serverSelectedProtocol
    ;

    constexpr std::size_t sz = sizeof(indata) - 1;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    InStream stream(buf, sz);
    GCC::UserData::CSCore cs_core;
    cs_core.recv(stream);
    BOOST_CHECK_EQUAL(CS_CORE, cs_core.userDataType);
    BOOST_CHECK_EQUAL(216, cs_core.length);

    cs_core.log("Client Received");
}


// 02 c0 0c 00 -> TS_UD_HEADER::type = CS_SECURITY (0xc002), length = 12 bytes


BOOST_AUTO_TEST_CASE(Test_gcc_user_data_cs_security)
{
    const char indata[] =
        "\x02\xc0"         // CS_SECURITY
        "\x0c\x00"         // 12 bytes user Data

        "\x1b\x00\x00\x00" // TS_UD_CS_SEC::encryptionMethods
                           // 0x1b = 0x01 | 0x02 | 0x08 | 0x10
                           // = 40BIT_ENCRYPTION_FLAG
                           // | 128BIT_ENCRYPTION_FLAG
                           // | 56BIT_ENCRYPTION_FLAG
                           // | FIPS_ENCRYPTION_FLAG
        "\x00\x00\x00\x00" // TS_UD_CS_SEC::extEncryptionMethods
    ;

    constexpr auto sz = sizeof(indata) - 1u;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    GCC::UserData::CSSecurity cs_security;
    InStream stream(buf);
    cs_security.recv(stream);
    BOOST_CHECK_EQUAL(CS_SECURITY, cs_security.userDataType);
    BOOST_CHECK_EQUAL(12, cs_security.length);
    BOOST_CHECK_EQUAL(27, cs_security.encryptionMethods);
    BOOST_CHECK_EQUAL(0, cs_security.extEncryptionMethods);

    cs_security.log("Client Received");
}

BOOST_AUTO_TEST_CASE(Test_gcc_user_data_sc_sec1_lage_rsa_key_blob)
{
    const char indata[] =
        // SC_SECURITY tag=0c02 length=428
        /* 0000 */ "\x02\x0c\xac\x01\x02\x00\x00\x00\x02\x00\x00\x00\x20\x00\x00\x00" //............ ...
        /* 0010 */ "\x78\x01\x00\x00\xd0\x33\x1c\x1c\xd1\x2e\xc6\xe0\xd2\xcf\x8f\x64" //x....3.........d
        /* 0020 */ "\x15\x44\x44\xed\x5a\x56\x1b\xd5\x26\xb7\xce\x38\x9b\xe1\x76\xe4" //.DD.ZV..&..8..v.
        /* 0030 */ "\x3b\x35\x37\x9f\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00" //;57.............
        /* 0040 */ "\x06\x00\x1c\x01\x52\x53\x41\x31\x08\x01\x00\x00\x00\x08\x00\x00" //....RSA1........
        /* 0050 */ "\xff\x00\x00\x00\x01\x00\x01\x00\xa5\xa9\x3b\x58\xeb\x5b\x52\xfa" //..........;X.[R.
        /* 0060 */ "\x53\x91\x3f\x23\x8c\xeb\xbe\x0c\x9a\x80\x74\x12\xe7\x2e\x33\xa8" //S.?#......t...3.
        /* 0070 */ "\x66\xa6\x4c\x4b\xb3\xdb\xdc\xc9\xf1\x71\x99\xba\x1e\x59\x9d\xc4" //f.LK.....q...Y..
        /* 0080 */ "\x83\xa2\xea\x6d\xd6\x8d\xc6\xb2\xf3\xac\xe4\x10\xb6\xb8\x12\xae" //...m............
        /* 0090 */ "\x03\x5b\xfd\x9d\x91\x8f\x48\x08\x96\x07\x89\x3a\x51\x54\x76\x39" //.[....H....:QTv9
        /* 00a0 */ "\x73\x5f\x0a\x3d\x4e\xe1\x9a\x95\x88\x8a\xf8\x90\xf1\x1c\x86\xd6" //s_.=N...........
        /* 00b0 */ "\x48\xa6\x4d\xe3\x73\xf8\xf7\xa4\xd0\x79\x21\x64\x6a\xcc\x3a\x80" //H.M.s....y!dj.:.
        /* 00c0 */ "\x1a\x97\x99\x58\xad\x68\xc9\x64\x9d\x04\x35\x32\xf5\x65\x90\x75" //...X.h.d..52.e.u
        /* 00d0 */ "\x05\x19\xde\x20\x16\xbd\x8b\xa3\xf3\x32\x67\xc1\x0f\x32\xb0\xb9" //... .....2g..2..
        /* 00e0 */ "\x45\xdb\xdb\xbb\x83\xd9\xe8\xe7\x16\xcc\x47\x75\x0e\xba\xe1\xa2" //E.........Gu....
        /* 00f0 */ "\x74\x29\x6c\xdb\x2b\x68\x92\xc8\x46\x38\x99\x3c\x52\xf0\x82\xe3" //t)l.+h..F8.<R...
        /* 0100 */ "\xfa\xfc\x42\xdb\xa2\xc6\xda\xa2\xc3\xef\x1c\x27\x7b\x3d\x76\x31" //..B........'{=v1
        /* 0110 */ "\x87\x13\xe3\x86\x72\xa6\xfe\xb8\xf7\x62\x44\x19\x62\x8b\x25\x15" //....r....bD.b.%.
        /* 0120 */ "\x63\xfd\x9e\x49\xdd\x01\x31\x83\x42\xaf\x85\xb9\x27\x3c\x6f\x9f" //c..I..1.B...'<o.
        /* 0130 */ "\x23\x66\xbd\x28\x21\x74\x88\xe3\x3e\xf8\xca\xd5\x25\x20\x96\x2b" //#f.(!t..>...% .+
        /* 0140 */ "\xf7\xff\xf2\x55\x15\xf2\xb3\x31\xd5\xc5\x9d\xc0\xe0\xa9\x9f\xea" //...U...1........
        /* 0150 */ "\x41\x8a\xe5\xa4\xbe\x09\xc7\xaf\x00\x00\x00\x00\x00\x00\x00\x00" //A...............
        /* 0160 */ "\x08\x00\x48\x00\xff\xad\xca\x1a\x79\xd6\x10\x61\x0c\x65\xf0\x02" //..H.....y..a.e..
        /* 0170 */ "\xb1\x54\x9f\x8b\x4e\x29\x9f\x09\x6a\x6b\xb7\xfe\xbd\xf3\x8a\x81" //.T..N)..jk......
        /* 0180 */ "\x78\x3c\xae\x81\xf3\x46\x1d\x4a\x34\xa2\x03\x3b\x4d\xb5\x9d\xb6" //x<...F.J4..;M...
        /* 0190 */ "\xf3\x69\x95\x17\xd4\x0a\x67\x4f\x84\xf4\x11\xe3\xec\xe8\x93\xa1" //.i....gO........
        /* 01a0 */ "\xcb\x4c\x09\x25\x00\x00\x00\x00\x00\x00\x00\x00"             //.L.%........
    ;

    constexpr auto sz = sizeof(indata) - 1u;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    GCC::UserData::SCSecurity sc_sec1;
    InStream stream(buf);
    sc_sec1.recv(stream);
    BOOST_CHECK_EQUAL(SC_SECURITY, sc_sec1.userDataType);
    BOOST_CHECK_EQUAL(sizeof(indata) - 1, sc_sec1.length);
    BOOST_CHECK_EQUAL(2, sc_sec1.encryptionMethod);
    BOOST_CHECK_EQUAL(2, sc_sec1.encryptionLevel);
    BOOST_CHECK_EQUAL(32, sc_sec1.serverRandomLen);
    BOOST_CHECK_EQUAL(376, sc_sec1.serverCertLen);
    //hexdump_c(sc_sec1.serverRandom, 32);
    BOOST_CHECK_EQUAL(0, memcmp(
                    "\xd0\x33\x1c\x1c\xd1\x2e\xc6\xe0\xd2\xcf\x8f\x64\x15\x44\x44\xed"
                    "\x5a\x56\x1b\xd5\x26\xb7\xce\x38\x9b\xe1\x76\xe4\x3b\x35\x37\x9f"
                             , sc_sec1.serverRandom, sc_sec1.serverRandomLen));
    BOOST_CHECK_EQUAL(static_cast<uint32_t>(GCC::UserData::SCSecurity::CERT_CHAIN_VERSION_1), sc_sec1.dwVersion);
    BOOST_CHECK_EQUAL(false, sc_sec1.temporary);
    BOOST_CHECK_EQUAL(0x31415352, sc_sec1.proprietaryCertificate.RSAPK.magic); // magic is really ASCII string 'RSA1'

    sc_sec1.log("Server Received");

    OutStream out_stream(buf);
    sc_sec1.emit(out_stream);

    CheckTransport ct(indata, sizeof(indata) - 1);

    ct.send(out_stream.get_data(), out_stream.get_offset());
}

BOOST_AUTO_TEST_CASE(Test_gcc_user_data_cs_mcs_msgchannel)
{
    const char indata[] =
        "\x06\xc0"         // CS_MCS_MSGCHANNEL
        "\x08\x00"         // 8 bytes user Data

        "\x00\x00\x00\x00" // TS_UD_CS_MCS_MSGCHANNEL::flags
    ;

    constexpr auto sz = sizeof(indata) - 1u;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    GCC::UserData::CSMCSMsgChannel cs_mcs_msgchannel;
    InStream stream(buf);
    cs_mcs_msgchannel.recv(stream);
    BOOST_CHECK_EQUAL(CS_MCS_MSGCHANNEL, cs_mcs_msgchannel.userDataType);
    BOOST_CHECK_EQUAL(8, cs_mcs_msgchannel.length);
    BOOST_CHECK_EQUAL(0, cs_mcs_msgchannel.flags);

    cs_mcs_msgchannel.log("Client Received");
}

BOOST_AUTO_TEST_CASE(Test_gcc_user_data_cs_multitransport)
{
    const char indata[] =
        "\x0a\xc0"         // CS_MULTITRANSPORT
        "\x08\x00"         // 8 bytes user Data

        "\x05\x03\x00\x00" // TS_UD_CS_MULTITRANSPORT::flags
    ;

    constexpr auto sz = sizeof(indata) - 1u;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    GCC::UserData::CSMultiTransport cs_multitransport;
    InStream stream(buf);
    cs_multitransport.recv(stream);
    BOOST_CHECK_EQUAL(CS_MULTITRANSPORT, cs_multitransport.userDataType);
    BOOST_CHECK_EQUAL(8, cs_multitransport.length);
    BOOST_CHECK_EQUAL(GCC::UserData::CSMultiTransport::TRANSPORTTYPE_UDPFECR |
                      GCC::UserData::CSMultiTransport::TRANSPORTTYPE_UDPFECL |
                      GCC::UserData::CSMultiTransport::TRANSPORTTYPE_UDP_PREFERRED |
                      GCC::UserData::CSMultiTransport::SOFTSYNC_TCP_TO_UDP,
                      cs_multitransport.flags);

    cs_multitransport.log("Client Received");
}
