/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/gcc/userdata/sc_net.hpp"


RED_AUTO_TEST_CASE(Test_gcc_sc_net_nbpkt_0)
{
    auto expected =
        "\x03\x0c\x08\x00" // TS_UD_HEADER::type = SC_NET (0x0c03), length = 8 bytes
        "\xeb\x03"         // TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
        "\x00\x00"         // TS_UD_SC_NET::channelCount = 0
        ""_av
    ;

    {
        StaticOutStream<16> out_stream;
        GCC::UserData::SCNet sc_net {};
        sc_net.MCSChannelId = 1003;
        sc_net.channelCount = 0;
        sc_net.emit(out_stream);
        RED_CHECK(out_stream.get_produced_bytes() == expected);
    }

    {
        GCC::UserData::SCNet sc_net2 {};
        InStream in_stream(expected);

        RED_REQUIRE_NO_THROW(sc_net2.recv(in_stream));
        RED_CHECK_EQUAL(SC_NET, sc_net2.userDataType);
        RED_CHECK_EQUAL(1003, sc_net2.MCSChannelId);
        RED_CHECK_EQUAL(0, sc_net2.channelCount);
    }
}

RED_AUTO_TEST_CASE(Test_gcc_sc_net_nbpkt_1)
{
    auto expected =
        "\x03\x0c\x0c\x00" // TS_UD_HEADER::type = SC_NET (0x0c03), length = 12 bytes
        "\xeb\x03"         // TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
        "\x01\x00"         // TS_UD_SC_NET::channelCount = 1
        "\xec\x03"         // channel0 = 0x3ec = 1004 (rdpdr)
        "\x00\x00"         // padding
        ""_av
    ;

    {
        StaticOutStream<16> out_stream;
        GCC::UserData::SCNet sc_net {};
        sc_net.MCSChannelId = 1003;
        sc_net.channelCount = 1;
        sc_net.channelDefArray[0].id = 1004;
        sc_net.emit(out_stream);
        RED_CHECK(out_stream.get_produced_bytes() == expected);
    }

    {
        GCC::UserData::SCNet sc_net2 {};
        InStream in_stream(expected);

        RED_REQUIRE_NO_THROW(sc_net2.recv(in_stream));
        RED_CHECK_EQUAL(SC_NET, sc_net2.userDataType);
        RED_CHECK_EQUAL(1003, sc_net2.MCSChannelId);
        RED_CHECK_EQUAL(1, sc_net2.channelCount);
        RED_CHECK_EQUAL(1004, sc_net2.channelDefArray[0].id);
    }
}

RED_AUTO_TEST_CASE(Test_gcc_sc_net_nbpkt_2)
{
    auto expected =
        "\x03\x0c\x0c\x00" // TS_UD_HEADER::type = SC_NET (0x0c03), length = 12 bytes
        "\xeb\x03"         // TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
        "\x02\x00"         // TS_UD_SC_NET::channelCount = 2
        "\xec\x03"         // channel0 = 0x3ec = 1004 (rdpdr)
        "\xed\x03"         // channel1 = 0x3ed = 1005 (cliprdr)
        ""_av
    ;

    {
        StaticOutStream<16> out_stream;
        GCC::UserData::SCNet sc_net {};
        sc_net.MCSChannelId = 1003;
        sc_net.channelCount = 2;
        sc_net.channelDefArray[0].id = 1004;
        sc_net.channelDefArray[1].id = 1005;
        sc_net.emit(out_stream);
        RED_CHECK(out_stream.get_produced_bytes() == expected);
    }

    {
        GCC::UserData::SCNet sc_net2 {};
        InStream in_stream(expected);

        RED_REQUIRE_NO_THROW(sc_net2.recv(in_stream));
        RED_CHECK_EQUAL(SC_NET, sc_net2.userDataType);
        RED_CHECK_EQUAL(1003, sc_net2.MCSChannelId);
        RED_CHECK_EQUAL(2, sc_net2.channelCount);
        RED_CHECK_EQUAL(1004, sc_net2.channelDefArray[0].id);
        RED_CHECK_EQUAL(1005, sc_net2.channelDefArray[1].id);
    }
}

RED_AUTO_TEST_CASE(Test_gcc_sc_net_nbpkt_3)
{
    auto expected =
        "\x03\x0c\x10\x00" // TS_UD_HEADER::type = SC_NET (0x0c03), length = 16 bytes
        "\xeb\x03"         // TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
        "\x03\x00"         // TS_UD_SC_NET::channelCount = 3
        "\xec\x03"         // channel0 = 0x3ec = 1004 (rdpdr)
        "\xed\x03"         // channel1 = 0x3ed = 1005 (cliprdr)
        "\xee\x03"         // channel2 = 0x3ee = 1006 (rdpsnd)
        "\x00\x00"         // padding
        ""_av
    ;

    {
        StaticOutStream<16> out_stream;
        GCC::UserData::SCNet sc_net {};
        sc_net.MCSChannelId = 1003;
        sc_net.channelCount = 3;
        sc_net.channelDefArray[0].id = 1004;
        sc_net.channelDefArray[1].id = 1005;
        sc_net.channelDefArray[2].id = 1006;
        sc_net.emit(out_stream);
        RED_CHECK(out_stream.get_produced_bytes() == expected);
    }

    {
        GCC::UserData::SCNet sc_net2 {};
        InStream in_stream(expected);

        RED_REQUIRE_NO_THROW(sc_net2.recv(in_stream));
        RED_CHECK_EQUAL(SC_NET, sc_net2.userDataType);
        RED_CHECK_EQUAL(1003, sc_net2.MCSChannelId);
        RED_CHECK_EQUAL(3, sc_net2.channelCount);
        RED_CHECK_EQUAL(1004, sc_net2.channelDefArray[0].id);
        RED_CHECK_EQUAL(1005, sc_net2.channelDefArray[1].id);
        RED_CHECK_EQUAL(1006, sc_net2.channelDefArray[2].id);
    }
}

RED_AUTO_TEST_CASE(Test_gcc_sc_net_nbpkt_4)
{
    auto expected =
        "\x03\x0c\x10\x00" // TS_UD_HEADER::type = SC_NET (0x0c03), length = 16 bytes
        "\xeb\x03"         // TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
        "\x04\x00"         // TS_UD_SC_NET::channelCount = 4
        "\xec\x03"         // channel0 = 0x3ec = 1004 (rdpdr)
        "\xed\x03"         // channel1 = 0x3ed = 1005 (cliprdr)
        "\xee\x03"         // channel2 = 0x3ee = 1006 (rdpsnd)
        "\xef\x03"         // channel3 = 0x3ef = 1007 (?????)
        ""_av
    ;

    {
        StaticOutStream<16> out_stream;
        GCC::UserData::SCNet sc_net {};
        sc_net.MCSChannelId = 1003;
        sc_net.channelCount = 4;
        sc_net.channelDefArray[0].id = 1004;
        sc_net.channelDefArray[1].id = 1005;
        sc_net.channelDefArray[2].id = 1006;
        sc_net.channelDefArray[3].id = 1007;
        sc_net.emit(out_stream);
        RED_CHECK(out_stream.get_produced_bytes() == expected);
    }

    {
        GCC::UserData::SCNet sc_net2 {};
        InStream in_stream(expected);

        RED_REQUIRE_NO_THROW(sc_net2.recv(in_stream));
        RED_CHECK_EQUAL(SC_NET, sc_net2.userDataType);
        RED_CHECK_EQUAL(1003, sc_net2.MCSChannelId);
        RED_CHECK_EQUAL(4, sc_net2.channelCount);
        RED_CHECK_EQUAL(1004, sc_net2.channelDefArray[0].id);
        RED_CHECK_EQUAL(1005, sc_net2.channelDefArray[1].id);
        RED_CHECK_EQUAL(1006, sc_net2.channelDefArray[2].id);
        RED_CHECK_EQUAL(1007, sc_net2.channelDefArray[3].id);
    }
}

RED_AUTO_TEST_CASE(Test_gcc_sc_net_nbpkt_read_2_with_bad_padding)
{
    auto expected =
        "\x03\x0c\x0e\x00" // TS_UD_HEADER::type = SC_NET (0x0c03), length = 14 bytes
        "\xeb\x03"         // TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
        "\x02\x00"         // TS_UD_SC_NET::channelCount = 2
        "\xec\x03"         // channel0 = 0x3ec = 1004 (rdpdr)
        "\xed\x03"         // channel1 = 0x3ed = 1005 (cliprdr)
        "\x00\x00"         // padding (not standard, channel data is already aligned on 4)
        ""_av
    ;

    GCC::UserData::SCNet sc_net2 {};
    InStream in_stream(expected);

    RED_REQUIRE_NO_THROW(sc_net2.recv(in_stream));
    RED_CHECK_EQUAL(SC_NET, sc_net2.userDataType);
    RED_CHECK_EQUAL(1003, sc_net2.MCSChannelId);
    RED_CHECK_EQUAL(2, sc_net2.channelCount);
    RED_CHECK_EQUAL(1004, sc_net2.channelDefArray[0].id);
    RED_CHECK_EQUAL(1005, sc_net2.channelDefArray[1].id);
}

RED_AUTO_TEST_CASE(Test_gcc_sc_net_nbpkt_read_bad_pck)
{
    auto expected =
        "\x03\x0c\x0c\x00" // TS_UD_HEADER::type = SC_NET (0x0c03), length = 12 bytes
        "\xeb\x03"         // TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
        "\x03\x00"         // TS_UD_SC_NET::channelCount = 3
        "\xec\x03"         // channel0 = 0x3ec = 1004 (rdpdr)
        "\xed\x03"         // channel1 = 0x3ed = 1005 (cliprdr)
        // missing channel
        ""_av
    ;

    GCC::UserData::SCNet sc_net2 {};
    InStream in_stream(expected);

    RED_CHECK_THROW(sc_net2.recv(in_stream), Error);
}
