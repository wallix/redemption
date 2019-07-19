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
    Copyright (C) Wallix 2016
    Author(s): Clément Moroldo
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDPEA/audio_output.hpp"
#include "utils/sugar/cast.hpp"


RED_AUTO_TEST_CASE(RDPSNDPDUHeaderEmit)
{
    auto data = "\x07\x00\x26\x00"_av;

    StaticOutStream<8> stream;
    rdpsnd::RDPSNDPDUHeader(rdpsnd::SNDC_FORMATS, 38).emit(stream);

    RED_CHECK_MEM(stream.get_bytes(), data);

    InStream in_stream(stream.get_bytes());

    rdpsnd::RDPSNDPDUHeader ch;
    ch.receive(in_stream);

    RED_CHECK_EQUAL(ch.msgType, underlying_cast(rdpsnd::SNDC_FORMATS));
    RED_CHECK_EQUAL(ch.BodySize, 38);
}

RED_AUTO_TEST_CASE(ServerAudioFormatsandVersionHeaderEmit)
{
    auto data =
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x16\x00"
        "\xff\x06\x00\x00"_av;

    StaticOutStream<32> stream;
    rdpsnd::ServerAudioFormatsandVersionHeader(22, 0xff, 6).emit(stream);

    RED_CHECK_MEM(stream.get_bytes(), data);

    InStream in_stream(stream.get_bytes());

    rdpsnd::ServerAudioFormatsandVersionHeader ch;
    ch.receive(in_stream);

    RED_CHECK_EQUAL(ch.wNumberOfFormats, 22);
    RED_CHECK_EQUAL(ch.cLastBlockConfirmed, 0xff);
    RED_CHECK_EQUAL(ch.wVersion, 6);
}

RED_AUTO_TEST_CASE(AudioFormatEmit)
{
    auto data =
        "\x01\x00\x02\x00\x44\xac\x00\x00\x10\xb1\x02\x00\x04\x00\x10\x00"
        "\x00\x00"_av;

    StaticOutStream<32> stream;
    rdpsnd::AudioFormat(rdpsnd::WAVE_FORMAT_PCM, 2, 0x0000ac44, 0x0002b110, 4, 16, 0).emit(stream);

    RED_CHECK_MEM(stream.get_bytes(), data);

    InStream in_stream(stream.get_bytes());

    rdpsnd::AudioFormat ch;
    ch.receive(in_stream);

    RED_CHECK_EQUAL(ch.wFormatTag, underlying_cast(rdpsnd::WAVE_FORMAT_PCM));
    RED_CHECK_EQUAL(ch.nChannels, 2);
    RED_CHECK_EQUAL(ch.nSamplesPerSec, 0x0000ac44);
    RED_CHECK_EQUAL(ch.nAvgBytesPerSec, 0x0002b110);
    RED_CHECK_EQUAL(ch.nBlockAlign, 4);
    RED_CHECK_EQUAL(ch.wBitsPerSample, 16);
    RED_CHECK_EQUAL(ch.cbSize, 0);
}

RED_AUTO_TEST_CASE(ClientAudioFormatsandVersionHeaderEmit)
{
    auto data =
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
        "\x00\x06\x00\x00"_av;

    StaticOutStream<32> stream;
    rdpsnd::ClientAudioFormatsandVersionHeader(0x00000001, 0x00000000, 0x00000000, 0x0000, 1, 6).emit(stream);

    RED_CHECK_MEM(stream.get_bytes(), data);

    InStream in_stream(stream.get_bytes());

    rdpsnd::ClientAudioFormatsandVersionHeader ch;
    ch.receive(in_stream);

    RED_CHECK_EQUAL(ch.dwFlags, 0x00000001);
    RED_CHECK_EQUAL(ch.dwVolume, 0x00000000);
    RED_CHECK_EQUAL(ch.dwPitch, 0x00000000);
    RED_CHECK_EQUAL(ch.wDGramPort, 0x0000);
    RED_CHECK_EQUAL(ch.wNumberOfFormats, 1);
    RED_CHECK_EQUAL(ch.wVersion, 6);
}

RED_AUTO_TEST_CASE(QualityModePDUEmit)
{
    auto data = "\x01\x00\x00\x00"_av;

    StaticOutStream<32> stream;
    rdpsnd::QualityModePDU(rdpsnd::MEDIUM_QUALITY).emit(stream);

    RED_CHECK_MEM(stream.get_bytes(), data);

    InStream in_stream(stream.get_bytes());

    rdpsnd::QualityModePDU ch;
    ch.receive(in_stream);

    RED_CHECK_EQUAL(ch.wQualityMode, underlying_cast(rdpsnd::MEDIUM_QUALITY));
}

RED_AUTO_TEST_CASE(TrainingPDUEmit)
{
    auto data = "\x73\xfd\x00\x04"_av;

    StaticOutStream<32> stream;
    rdpsnd::TrainingPDU(0xfd73, 1024).emit(stream);

    RED_CHECK_MEM(stream.get_bytes(), data);

    InStream in_stream(stream.get_bytes());

    rdpsnd::TrainingPDU ch;
    ch.receive(in_stream);

    RED_CHECK_EQUAL(ch.wTimeStamp, 0xfd73);
    RED_CHECK_EQUAL(ch.wPackSize, 1024);
}


RED_AUTO_TEST_CASE(TrainingConfirmPDUEmit)
{
    auto data = "\x73\xfd\x00\x04"_av;

    StaticOutStream<32> stream;
    rdpsnd::TrainingConfirmPDU(0xfd73, 1024).emit(stream);

    RED_CHECK_MEM(stream.get_bytes(), data);

    InStream in_stream(stream.get_bytes());

    rdpsnd::TrainingConfirmPDU ch;
    ch.receive(in_stream);

    RED_CHECK_EQUAL(ch.wTimeStamp, 0xfd73);
    RED_CHECK_EQUAL(ch.wPackSize, 1024);
}

RED_AUTO_TEST_CASE(WaveInfoPDUEmit)
{
    auto data = "\xa7\x11\x00\x00\x00\x00\x00\x00\x01\x02\x03\x04"_av;

    StaticOutStream<32> stream;
    {
        rdpsnd::WaveInfoPDU ch(0x11a7, 0x0000, 0x00);
        ch.Data[0] = 1;
        ch.Data[1] = 2;
        ch.Data[2] = 3;
        ch.Data[3] = 4;
        ch.emit(stream);
    }

    RED_CHECK_MEM(stream.get_bytes(), data);

    InStream in_stream(data);

    rdpsnd::WaveInfoPDU ch;
    ch.receive(in_stream);

    RED_CHECK_EQUAL(ch.wTimeStamp, 0x11a7);
    RED_CHECK_EQUAL(ch.wFormatNo, 0x0000);
    RED_CHECK_EQUAL(ch.cBlockNo, 0x00);
    RED_CHECK_MEM_AA(ch.Data, data.from_at(8));
}

RED_AUTO_TEST_CASE(WaveConfirmPDUEmit)
{
    auto data = "\xa7\x11\x01\x00"_av;

    StaticOutStream<32> stream;
    rdpsnd::WaveConfirmPDU(0x11a7, 0x01).emit(stream);

    RED_CHECK_MEM(stream.get_bytes(), data);

    InStream in_stream(stream.get_bytes());

    rdpsnd::WaveConfirmPDU ch;
    ch.receive(in_stream);

    RED_CHECK_EQUAL(ch.wTimeStamp, 0x11a7);
    RED_CHECK_EQUAL(ch.cConfBlockNo, 0x01);
}
