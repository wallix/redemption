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
   Copyright (C) Wallix 2010
   Author(s): Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them
*/

#define RED_TEST_MODULE TestRemoteAppChannelchannel
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "client_redemption/client_channels/fake_client_mod.hpp"
#include "client_redemption/client_channels/client_remoteapp_channel.hpp"



RED_AUTO_TEST_CASE(TestRemoteAppChannelInitialization)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod mod_channel;
    mod_channel.set_mod(&mod);
    ClientCallback callback(nullptr);
    callback.set_mod(&mod);
    FakeClientOutputGraphic graph_io(nullptr, nullptr);

    ClientRemoteAppChannel channel(RDPVerbose::rail, &callback, &mod_channel);
    channel.set_api(&graph_io);

    RDPRemoteAppConfig config;

    channel.set_configuration(800, 600, config);


    StaticOutStream<32> out_handshake;
    out_handshake.out_uint16_le(TS_RAIL_ORDER_HANDSHAKE);
    out_handshake.out_uint16_le(8);
    HandshakePDU handshake;
    handshake.buildNumber(1);
    handshake.emit(out_handshake);
    InStream chunk_handshake(out_handshake.get_bytes());
    channel.receive(chunk_handshake);

    StaticOutStream<32> out_sspu;
    out_sspu.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
    out_sspu.out_uint16_le(8);
    ServerSystemParametersUpdatePDU sspu;
    sspu.SystemParam(SPI_SETSCREENSAVEACTIVE);
    sspu.emit(out_sspu);
    InStream chunk_sspu(out_sspu.get_bytes());
    channel.receive(chunk_sspu);

    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 10);

    RED_CHECK_EQUAL(channel.build_number, 1);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_REQUIRE(pdu_data);
}


// RED_AUTO_TEST_CASE(TestRemoteAppChannel)
// {
//     FakeRDPChannelsMod mod;
//     ClientCallback callback(nullptr);
//     callback.set_mod(&mod);
//     FakeClientOutputGraphic graph_io;
//     FakeClientInputMouseKeyboard input_io;
//
//     ClientRemoteAppChannel channel(RDPVerbose::rail, &client, &graph_io, &input_io);
//
//     StaticOutStream<512> out_WaveInfoPDU;
//     rdpsnd::RDPSNDPDUHeader header(rdpsnd::SNDC_WAVE, 12);
//     header.emit(out_WaveInfoPDU);
//     rdpsnd::WaveInfoPDU waveInfo(0, 0, 0);
//     waveInfo.emit(out_WaveInfoPDU);
//     InStream chunk_WaveInfoPDU(out_WaveInfoPDU.get_bytes());
//
//     channel.receive(chunk_WaveInfoPDU);
//
//     StaticOutStream<512> out_WavePDU;
//     out_WavePDU.out_uint32_be(0);
//     InStream chunk_WavePDU(out_WavePDU.get_bytes());
//     channel.receive(chunk_WavePDU);
//
//     RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);
//
//     FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
//     RED_REQUIRE(pdu_data);
//     InStream stream_waveconfirm(pdu_data->data, pdu_data->size);
//     rdpsnd::RDPSNDPDUHeader header_waveConfirm;
//     header_waveConfirm.receive(stream_waveconfirm);
//     RED_CHECK_EQUAL(header_waveConfirm.msgType, rdpsnd::SNDC_WAVECONFIRM);
// }
