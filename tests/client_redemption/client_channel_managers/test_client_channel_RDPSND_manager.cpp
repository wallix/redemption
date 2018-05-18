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

#define RED_TEST_MODULE TestRDPSNDChannelManager
#include "system/redemption_unit_tests.hpp"


#include "client_redemption/client_channel_managers/fake_client_mod.hpp"
#include "client_redemption/client_channel_managers/client_channel_RDPSND_manager.hpp"



RED_AUTO_TEST_CASE(TestRDPSNDChannelInitialization)
{
    FakeClient client;
    FakeClientOutPutSound snd_io;
    ClientChannelRDPSNDManager manager(to_verbose_flags(0x0), &client, &snd_io);



    StaticOutStream<512> out_ServerAudioFormatsandVersion;
    rdpsnd::RDPSNDPDUHeader header_ServerAudioFormatsandVersion(rdpsnd::SNDC_FORMATS, rdpsnd::ServerAudioFormatsandVersionHeader::size());
    header_ServerAudioFormatsandVersion.emit(out_ServerAudioFormatsandVersion);
    rdpsnd::ServerAudioFormatsandVersionHeader safsvh(0, 0, 0);
    safsvh.emit(out_ServerAudioFormatsandVersion);
    InStream chunk_ServerAudioFormatsandVersion(out_ServerAudioFormatsandVersion.get_data(), out_ServerAudioFormatsandVersion.get_offset());

    manager.receive(chunk_ServerAudioFormatsandVersion);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 2);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_formats(pdu_data->data, pdu_data->size);
    rdpsnd::RDPSNDPDUHeader header_formats;
    header_formats.receive(stream_formats);
    RED_CHECK_EQUAL(header_formats.msgType, rdpsnd::SNDC_FORMATS);

    pdu_data = client.stream();
    InStream stream_qualitymode(pdu_data->data, pdu_data->size);
    rdpsnd::RDPSNDPDUHeader header_qualitymode;
    header_qualitymode.receive(stream_qualitymode);
    RED_CHECK_EQUAL(header_qualitymode.msgType, rdpsnd::SNDC_QUALITYMODE);



    StaticOutStream<512> out_TrainingPDU;
    rdpsnd::RDPSNDPDUHeader header_TrainingPDU(rdpsnd::SNDC_TRAINING, 4);
    header_TrainingPDU.emit(out_TrainingPDU);
    rdpsnd::TrainingPDU train(0, 0);
    train.emit(out_TrainingPDU);
    InStream chunk_TrainingPDU(out_TrainingPDU.get_data(), out_TrainingPDU.get_offset());

    manager.receive(chunk_TrainingPDU);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 3);

    pdu_data = client.stream();
    InStream stream_clientTraining(pdu_data->data, pdu_data->size);
    rdpsnd::RDPSNDPDUHeader header_clientTraining;
    header_clientTraining.receive(stream_clientTraining);
    RED_CHECK_EQUAL(header_clientTraining.msgType, rdpsnd::SNDC_TRAINING);
}



RED_AUTO_TEST_CASE(TestRDPSNDChannelWave)
{
    FakeClient client;
    FakeClientOutPutSound snd_io;
    ClientChannelRDPSNDManager manager(to_verbose_flags(0x0), &client, &snd_io);

    

    StaticOutStream<512> out_WaveInfoPDU;
    rdpsnd::RDPSNDPDUHeader header(rdpsnd::SNDC_WAVE, 12);
    header.emit(out_WaveInfoPDU);
    rdpsnd::WaveInfoPDU waveInfo(0, 0, 0);
    waveInfo.emit(out_WaveInfoPDU);
    InStream chunk_WaveInfoPDU(out_WaveInfoPDU.get_data(), out_WaveInfoPDU.get_offset());

    manager.receive(chunk_WaveInfoPDU);

    StaticOutStream<512> out_WavePDU;
    out_WavePDU.out_uint32_be(0);
    InStream chunk_WavePDU(out_WavePDU.get_data(), out_WavePDU.get_offset());
    manager.receive(chunk_WavePDU);

    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_waveconfirm(pdu_data->data, pdu_data->size);
    rdpsnd::RDPSNDPDUHeader header_waveConfirm;
    header_waveConfirm.receive(stream_waveconfirm);
    RED_CHECK_EQUAL(header_waveConfirm.msgType, rdpsnd::SNDC_WAVECONFIRM);
}


