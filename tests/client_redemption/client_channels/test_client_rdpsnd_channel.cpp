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

#define RED_TEST_MODULE TestRDPSNDChannelchannel
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "client_redemption/client_channels/fake_client_mod.hpp"
#include "client_redemption/client_channels/client_rdpsnd_channel.hpp"



RED_AUTO_TEST_CASE(TestRDPSNDChannelInitialization)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeClientOutPutSound snd_io;
    RDPSoundConfig config;
    ClientRDPSNDChannel channel(to_verbose_flags(0x0), &callback, config);
    channel.set_api(&snd_io);


    StaticOutStream<512> out_ServerAudioFormatsandVersion;
    rdpsnd::RDPSNDPDUHeader header_ServerAudioFormatsandVersion(rdpsnd::SNDC_FORMATS, rdpsnd::ServerAudioFormatsandVersionHeader::size()+18);
    header_ServerAudioFormatsandVersion.emit(out_ServerAudioFormatsandVersion);

    rdpsnd::ServerAudioFormatsandVersionHeader safsvh(1, 0xff, 0x0006);
    safsvh.emit(out_ServerAudioFormatsandVersion);

    rdpsnd::AudioFormat af_to_send(rdpsnd::WAVE_FORMAT_PCM,
                           0x0002,
                           0x0000ac44,
                           0x0002b110,
                           0x0004,
                           0x0010,
                           0);

    af_to_send.emit(out_ServerAudioFormatsandVersion);

    InStream chunk_ServerAudioFormatsandVersion(out_ServerAudioFormatsandVersion.get_bytes());


    channel.receive(chunk_ServerAudioFormatsandVersion);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 2);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_REQUIRE(pdu_data);
    InStream stream_formats(pdu_data->data, pdu_data->size);
    rdpsnd::RDPSNDPDUHeader header_formats;
    header_formats.receive(stream_formats);
    RED_CHECK_EQUAL(header_formats.msgType, rdpsnd::SNDC_FORMATS);
    RED_CHECK_EQUAL(header_formats.BodySize, 38);

    rdpsnd::ClientAudioFormatsandVersionHeader safsvh_received;
    safsvh_received.receive(stream_formats);
    RED_CHECK_EQUAL(safsvh_received.dwFlags, 0x00000003);
    RED_CHECK_EQUAL(safsvh_received.dwVolume, 0x7fff7fff);
    RED_CHECK_EQUAL(safsvh_received.dwPitch, 0x00000000);
    RED_CHECK_EQUAL(safsvh_received.wDGramPort, 0x0000);
    RED_CHECK_EQUAL(safsvh_received.wNumberOfFormats, 1);
    RED_CHECK_EQUAL(safsvh_received.wVersion, 0x0006);

    rdpsnd::AudioFormat af_received;
    af_received.receive(stream_formats);
    RED_CHECK_EQUAL(af_received.wFormatTag, 0x0001);


    pdu_data = mod.stream();
    RED_REQUIRE(pdu_data);
    InStream stream_qualitymode(pdu_data->data, pdu_data->size);
    rdpsnd::RDPSNDPDUHeader header_qualitymode;
    header_qualitymode.receive(stream_qualitymode);
    RED_CHECK_EQUAL(header_qualitymode.msgType, rdpsnd::SNDC_QUALITYMODE);
    RED_CHECK_EQUAL(header_qualitymode.BodySize, 8);

    rdpsnd::QualityModePDU qm;
    qm.receive(stream_qualitymode);
    RED_CHECK_EQUAL(qm.wQualityMode, 0x0002);


    StaticOutStream<512> out_TrainingPDU;
    rdpsnd::RDPSNDPDUHeader header_TrainingPDU(rdpsnd::SNDC_TRAINING, 4);
    header_TrainingPDU.emit(out_TrainingPDU);
    rdpsnd::TrainingPDU train(0x954e, 0);
    train.emit(out_TrainingPDU);
    InStream chunk_TrainingPDU(out_TrainingPDU.get_bytes());

    channel.receive(chunk_TrainingPDU);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 3);

    pdu_data = mod.stream();
    RED_REQUIRE(pdu_data);
    InStream stream_clientTraining(pdu_data->data, pdu_data->size);
    rdpsnd::RDPSNDPDUHeader header_clientTraining;
    header_clientTraining.receive(stream_clientTraining);
    RED_CHECK_EQUAL(header_clientTraining.msgType, rdpsnd::SNDC_TRAINING);
    RED_CHECK_EQUAL(header_clientTraining.BodySize, 4);
    rdpsnd::TrainingConfirmPDU tc;
    tc.receive(stream_clientTraining);
    RED_CHECK_EQUAL(tc.wPackSize, 0);
}



RED_AUTO_TEST_CASE(TestRDPSNDChannelWave)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeClientOutPutSound snd_io;
    RDPSoundConfig config;
    ClientRDPSNDChannel channel(to_verbose_flags(0x0), &callback, config);
    channel.set_api(&snd_io);

    StaticOutStream<512> out_WaveInfoPDU;
    rdpsnd::RDPSNDPDUHeader header(rdpsnd::SNDC_WAVE, 12);
    header.emit(out_WaveInfoPDU);
    rdpsnd::WaveInfoPDU waveInfo(0x58ea, 0x0000, 0x00);
    waveInfo.emit(out_WaveInfoPDU);
    InStream chunk_WaveInfoPDU(out_WaveInfoPDU.get_bytes());

    channel.receive(chunk_WaveInfoPDU);

    StaticOutStream<512> out_WavePDU;
    out_WavePDU.out_uint32_be(0);
    InStream chunk_WavePDU(out_WavePDU.get_bytes());
    channel.receive(chunk_WavePDU);

    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_REQUIRE(pdu_data);
    InStream stream_waveconfirm(pdu_data->data, pdu_data->size);
    rdpsnd::RDPSNDPDUHeader header_waveConfirm;
    header_waveConfirm.receive(stream_waveconfirm);
    RED_CHECK_EQUAL(header_waveConfirm.msgType, rdpsnd::SNDC_WAVECONFIRM);
    RED_CHECK_EQUAL(header_waveConfirm.BodySize, 4);

    rdpsnd::WaveConfirmPDU wc;
    wc.receive(stream_waveconfirm);
    RED_CHECK_EQUAL(wc.wTimeStamp, 0x58ea);
    RED_CHECK_EQUAL(wc.cConfBlockNo, 0x00);
}


