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
    Copyright (C) Wallix 2015
    Author(s): Clément Moroldo
*/


#pragma once

#include "core/error.hpp"
#include "utils/stream.hpp"
#include "core/channel_list.hpp"


namespace rdpsnd {



// 2.2.1 RDPSND PDU Header (SNDPROLOG)
//
// The RDPSND PDU header is present in many audio PDUs. It is used to identify the
// PDU type, specify the length of the PDU, and convey message flags.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |    msgType    |     bPad      |            BodySize           |
// +---------------+---------------+-------------------------------+

// msgType (1 byte): An 8-bit unsigned integer that specifies the type of audio PDU
// that follows the BodySize field.

//  +-----------------------------+--------------------------------------------+
//  | Value                       | Meaning                                    |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_CLOSE                  | Close PDU                                  |
//  | 0x01                        |                                            |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_WAVE                   | WaveInfo PDU                               |
//  | 0x02                        |                                            |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_SETVOLUME              | Volume PDU                                 |
//  | 0x03                        |                                            |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_SETPITCH               | Pitch PDU                                  |
//  | 0x04                        |                                            |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_WAVECONFIRM            | Wave Confirm PDU                           |
//  | 0x05                        |                                            |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_TRAINING               | Training PDU or Training Confirm PDU       |
//  | 0x06                        |                                            |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_FORMATS                | Server Audio Formats and Version PDU or    |
//  | 0x07                        | Client Audio Formats and Version PDU       |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_CRYPTKEY               | Crypt Key PDU                              |
//  | 0x08                        |                                            |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_WAVEENCRYPT            | Wave Encrypt PDU                           |
//  | 0x09                        |                                            |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_UDPWAVE                | UDP Wave PDU                               |
//  | 0x0a                        |                                            |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_UDPWAVELAST            | UDP Wave Last PDU                          |
//  | 0x0b                        |                                            |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_QUALITYMODE            | Quality Mode PDU                           |
//  | 0x0c                        |                                            |
//  +-----------------------------+--------------------------------------------+
//  | SNDC_WAVE2                  | Wave2 PDU                                  |
//  | 0x0d                        |                                            |
//  +-----------------------------+--------------------------------------------+

// bPad (1 byte): An 8-bit unsigned integer. Unused. The value in this field is arbitrary
// and MUST be ignored on receipt.

// BodySize (2 bytes): A 16-bit unsigned integer. If msgType is not set to 0x02 (SNDC_WAVE),
// then this field specifies the size, in bytes, of the data that follows the RDPSND PDU header.
// If msgType is set to 0x02 (SNDC_WAVE), then the representation of BodySize is explained in
// the Header field in section 2.2.3.3.

enum : uint8_t {
    SNDC_CLOSE       = 0x01,
    SNDC_WAVE        = 0x02,
    SNDC_SETVOLUME   = 0x03,
    SNDC_SETPITCH    = 0x04,
    SNDC_WAVECONFIRM = 0x05,
    SNDC_TRAINING    = 0x06,
    SNDC_FORMATS     = 0x07,
    SNDC_CRYPTKEY    = 0x08,
    SNDC_WAVEENCRYPT = 0x09,
    SNDC_UDPWAVE     = 0x0a,
    SNDC_UDPWAVELAST = 0x0b,
    SNDC_QUALITYMODE = 0x0c,
    SNDC_WAVE2       = 0x0d
};

inline static const char * get_messageType_name(uint8_t messageType) {
    switch (messageType) {
        case SNDC_CLOSE:       return "SNDC_CLOSE";
        case SNDC_WAVE:        return "SNDC_WAVE";
        case SNDC_SETVOLUME:   return "SNDC_SETVOLUME";
        case SNDC_SETPITCH:    return "SNDC_SETPITCH";
        case SNDC_WAVECONFIRM: return "SNDC_WAVECONFIRM";
        case SNDC_TRAINING:    return "SNDC_TRAINING";
        case SNDC_FORMATS:     return "SNDC_FORMATS";
        case SNDC_CRYPTKEY:    return "SNDC_CRYPTKEY";
        case SNDC_WAVEENCRYPT: return "SNDC_WAVEENCRYPT";
        case SNDC_UDPWAVE:     return "SNDC_UDPWAVE";
        case SNDC_UDPWAVELAST: return "SNDC_UDPWAVELAST";
        case SNDC_QUALITYMODE: return "SNDC_QUALITYMODE";
        case SNDC_WAVE2:       return "SNDC_WAVE2";
    }

    return "<unknown>";
}

struct RDPSNDPDUHeader {

    uint8_t msgType;
    uint8_t bPad;
    size_t  BodySize;

    RDPSNDPDUHeader() = default;

    RDPSNDPDUHeader(uint8_t msgType, size_t BodySize)
      : msgType(msgType)
      , bPad(0)
      , BodySize(BodySize) {}

    void emit(OutStream & stream) {
        stream.out_uint8(this->msgType);
        stream.out_uint8(this->bPad);
        stream.out_uint16_le(this->BodySize);
    }

    void receive(InStream & stream) {
        const unsigned expected = 4;    // msgType(1) + bPad(1) + BodySize(2)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated RDPSNDPDUHeader (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }
        this->msgType  = stream.in_uint8();
        this->bPad     = stream.in_uint8();
        this->BodySize = stream.in_uint16_le();
//         const unsigned expected2 = this->BodySize;    // BodySize
//         if (!stream.in_check_rem(expected2)) {
//             LOG(LOG_ERR,
//                 "Truncated RDPSNDPDUHeader BodySize (0): expected=%u remains=%zu",
//                 expected, stream.in_remain());
//             throw Error(ERR_FSCC_DATA_TRUNCATED);
//         }
    }

    void log() {
        LOG(LOG_INFO, "     RDPSND PDU Header:");
        LOG(LOG_INFO, "          * msgType  = 0x%02x (1 byte): %s", this->msgType, get_messageType_name(this->msgType));
        LOG(LOG_INFO, "          * bPad - (1 byte) NOT USED");
        LOG(LOG_INFO, "          * BodySize = %zu (2 bytes)", this->BodySize);
    }

};




// 2.2.2.1 Server Audio Formats and Version PDU (SERVER_AUDIO_VERSION_AND_FORMATS)
//
// The Server Audio Formats and Version PDU is a PDU used by the server to send version
// information and a list of supported audio formats to the client. This PDU MUST be sent
// using virtual channels.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Header                           |
// +---------------------------------------------------------------+
// |                             dwFlags                           |
// +---------------------------------------------------------------+
// |                             dwVolume                          |
// +---------------------------------------------------------------+
// |                             dwPitch                           |
// +-------------------------------+-------------------------------+
// |          wDGramPort           |        wNumberOfFormats       |
// +---------------+---------------+---------------+---------------+
// |cLastBlockConf |            wVersion           |     bPad      |
// +---------------+-------------------------------+---------------+
// |                       sndFormats (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Header (4 bytes): A RDPSND PDU Header (section 2.2.1). The msgType field of the RDPSND PDU
//                   Header MUST be set to SNDC_FORMATS (0x07).
//
// dwFlags (4 bytes): A 32-bit unsigned integer. This field is unused. The value is arbitrary
//                    and MUST be ignored on receipt.
//
// dwVolume (4 bytes): A 32-bit unsigned integer. This field is unused. The value is arbitrary
//                     and MUST be ignored on receipt.
//
// dwPitch (4 bytes): A 32-bit unsigned integer. This field is unused. The value is arbitrary
//                    and MUST be ignored on receipt.
//
// wDGramPort (2 bytes): A 16-bit unsigned integer. This field is unused. The value is arbitrary
//                       and MUST be ignored on receipt.
//
// wNumberOfFormats (2 bytes): A 16-bit unsigned integer. Number of AUDIO_FORMAT structures
//                             contained in the sndFormats array.
//
// cLastBlockConfirmed (1 byte): An 8-bit unsigned integer specifying the initial value for
//                               the cBlockNo counter used by the WaveInfo PDU, Wave2 PDU, Wave
//                               Encrypt PDU, UDP Wave PDU, and UDP Wave Last PDU. The value sent
//                               by the server is arbitrary. See section 3.3.5.2.1.1 for more
//                               information about the cBlockNo counter.
//
// wVersion (2 bytes): A 16-bit unsigned integer that contains the version of the protocol
//                     supported by the server.<3>
//
// bPad (1 byte): An 8-bit unsigned integer. This field is unused. The value is arbitrary and MUST
//                be ignored on receipt.
//
// sndFormats (variable): A variable-sized array of audio formats supported by the server, each
//                        conforming in structure to the AUDIO_FORMAT structure. The number of
//                        formats in the array is wNumberOfFormats.

struct ServerAudioFormatsandVersionHeader{

    uint16_t wNumberOfFormats;
    uint8_t  cLastBlockConfirmed;
    uint16_t wVersion;

    ServerAudioFormatsandVersionHeader() = default;

    ServerAudioFormatsandVersionHeader(uint16_t wNumberOfFormats,
                                       uint8_t  cLastBlockConfirmed,
                                       uint16_t wVersion)

        : wNumberOfFormats(wNumberOfFormats)
        , cLastBlockConfirmed(cLastBlockConfirmed)
        , wVersion(wVersion) {}

    void emit(OutStream & stream) {
        stream.out_clear_bytes(14);
        stream.out_uint16_le(this->wNumberOfFormats);
        stream.out_uint8(this->cLastBlockConfirmed);
        stream.out_uint16_le(this->wVersion);
        stream.out_clear_bytes(1);
    }

    void receive(InStream & stream) {
        const unsigned expected = 20;           // dwFlags(4) + dwVolume(4) + dwPitch(4) + wDGramPort(2) +
                                                // wNumberOfFormats(2) + cLastBlockConfirmed(1) +
                                                // wVersion(2) + bPad(1)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated ServerAudioFormatsandVersionHeader (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }
        stream.in_skip_bytes(14);
        this->wNumberOfFormats = stream.in_uint16_le();
        this->cLastBlockConfirmed = stream.in_uint8();
        this->wVersion = stream.in_uint16_le();
        stream.in_skip_bytes(1);
    }

    void log() {
        LOG(LOG_INFO, "     Server Audio Formats and Version Header:");
        LOG(LOG_INFO, "          * dwFlags    - (4 bytes) NOT USED");
        LOG(LOG_INFO, "          * dwVolume   - (4 bytes) NOT USED");
        LOG(LOG_INFO, "          * dwPitch    - (4 bytes) NOT USED");
        LOG(LOG_INFO, "          * wDGramPort - (2 bytes) NOT USED");
        LOG(LOG_INFO, "          * wNumberOfFormats    = %d (2 bytes)", int(this->wNumberOfFormats));
        LOG(LOG_INFO, "          * cLastBlockConfirmed = 0x%02x (1 byte)", this->cLastBlockConfirmed);
        LOG(LOG_INFO, "          * wVersion            = 0x%04x (2 bytes)", this->wVersion);
        LOG(LOG_INFO, "          * bPad       - (1 byte)  NOT USED");
    }

    static size_t size() {
        return 24;  //  4 + 4 + 4 + 4 + 2 + 2 + 2 + 1 + 2 + 1
    }
};



// 2.2.2.1.1 Audio Format (AUDIO_FORMAT)

// The AUDIO_FORMAT structure is used to describe a supported audio format.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          wFormatTag           |           nChannels           |
// +---------------+---------------+---------------+---------------+
// |                        nSamplesPerSec                         |
// +---------------------------------------------------------------+
// |                        nAvgBytesPerSec                        |
// +---------------------------------------------------------------+
// |          nBlockAlign          |          wBitsPerSample       |
// +---------------+---------------+---------------+---------------+
// |            cbSize             |         data (variable)       |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// wFormatTag (2 bytes): An unsigned 16-bit integer identifying the compression
// format of the audio format. It MUST be set to a WAVE form Registration Number
// listed in [RFC2361]. At a minimum, clients and servers MUST support WAVE_FORMAT_PCM (0x0001).
// All compression formats supported on specific Windows versions along with corresponding
// wFormatTag field values are specified by the product behavior note in the data field
// description of this section.
//
// nChannels (2 bytes): An unsigned 16-bit integer that specifies the number of channels in the
// audio format. The number of channels is part of the audio format and is not determined by the
// Remote Desktop Protocol: Audio Output Virtual Channel Extension protocol.
//
// nSamplesPerSec (4 bytes): An unsigned 32-bit integer that specifies the number of audio
// samples per second in the audio format.
//
// nAvgBytesPerSec (4 bytes): An unsigned 32-bit integer that specifies the average
// number of bytes the audio format uses to encode one second of audio data.
//
// nBlockAlign (2 bytes): An unsigned 16-bit integer that specifies the minimum atomic
// unit of data needed to process audio of this format. See [MSDN-AUDIOFORMAT] for more
// information about block alignment semantics.
//
// wBitsPerSample (2 bytes): An unsigned 16-bit integer that specifies the number of bits
// needed to represent a sample.
//
// cbSize (2 bytes): An unsigned 16-bit integer specifying the size of the data field.
//
// data (variable): Extra data specific to the audio format.<4> See [MSDN-AUDIOFORMAT] for
// additional details about extra format information. The size of data, in bytes, is cbSize.




// <4> Section 2.2.2.1.1: For more information about registering format information, see [RFC2361].
//
// For more information about Pulse-Code Modulation (PCM), see [G711].
//
// For more information about Global System for Mobile communications (GSM), see [ETSI-GSM].
//
// The following tables show codecs and associated format tags that are supported by default
// on the different versions of Windows, which support audio redirection. Unless otherwise
// specified, information about these codecs can be found in [RFC2361].
//
// The following codecs are supported by default on Windows XP and Windows Server 2003:

//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Codec name                 | Format tag                              | Exceptions                            |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | DSP Group, Inc. TrueSpeech | WAVE_FORMAT_DSPGROUP_TRUESPEECH         |                                       |
//  |                            | 0x0022                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | ISO/MPEG Layer 3           | WAVE_FORMAT_MPEGLAYER3                  |                                       |
//  | [ISO/IEC-11172-3]          | 0x0055                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Voxware, Inc. AC10         | WAVE_FORMAT_VOXWARE_AC10                | Not supported on Windows Server 2003  |
//  |                            | 0x0071                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Voxware, Inc. AC16         | WAVE_FORMAT_VOXWARE_AC16                | Not supported on Windows Server 2003  |
//  |                            | 0x0072                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Voxware, Inc. AC20         | WAVE_FORMAT_VOXWARE_AC20                |                                       |
//  |                            | 0x0073                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Voxware, Inc. AC8          | WAVE_FORMAT_VOXWARE_AC8                 | Not supported on Windows Server 2003  |
//  |                            | 0x0070                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+

// The following codecs are supported by default on Windows:

//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Codec name                 | Format tag                              | Exceptions                            |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Microsoft PCM              | WAVE_FORMAT_PCM                         |                                       |
//  |                            | 0x0001                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Microsoft Adaptive PCM     | WAVE_FORMAT_ADPCM                       |                                       |
//  |                            | 0x0002                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Microsoft ALAW             | WAVE_FORMAT_ALAW                        |                                       |
//  |                            | 0x0006                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Microsoft G.723            | WAVE_FORMAT_MSG723                      |                                       |
//  |                            | 0x0042                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Microsoft GSM              | WAVE_FORMAT_GSM610                      |                                       |
//  |                            | 0x0031                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Microsoft MULAW            | WAVE_FORMAT_MULAW                       |                                       |
//  |                            | 0x0007                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+
//  | Microsoft AAC              | WAVE_FORMAT_AAC                         | Not supported on Windows Server 2003  |
//  |                            | 0xa106                                  |                                       |
//  +----------------------------+-----------------------------------------+---------------------------------------+

enum : uint16_t {
    // The following codecs are supported by default on Windows:
    WAVE_FORMAT_PCM    = 0x0001,
    WAVE_FORMAT_ADPCM  = 0x0002,
    WAVE_FORMAT_ALAW   = 0x0006,
    WAVE_FORMAT_MSG723 = 0x0042,
    WAVE_FORMAT_GSM610 = 0x0031,
    WAVE_FORMAT_MULAW  = 0x0007,
    WAVE_FORMAT_AAC    = 0xa106,

    // The following codecs are NOT supported by default on Windows:
    WAVE_FORMAT_DSPGROUP_TRUESPEECH = 0x0022,
    WAVE_FORMAT_MPEGLAYER3          = 0x0055,
    WAVE_FORMAT_VOXWARE_AC10        = 0x0071,
    WAVE_FORMAT_VOXWARE_AC16        = 0x0072,
    WAVE_FORMAT_VOXWARE_AC20        = 0x0073,
    WAVE_FORMAT_VOXWARE_AC8         = 0x0070
};

inline static const char * get_wFormatTag_name(uint16_t wFormatTag) {
    switch (wFormatTag) {
        case WAVE_FORMAT_PCM:    return "WAVE_FORMAT_PCM";
        case WAVE_FORMAT_ADPCM:  return "WAVE_FORMAT_ADPCM";
        case WAVE_FORMAT_ALAW:   return "WAVE_FORMAT_ALAW";
        case WAVE_FORMAT_MSG723: return "WAVE_FORMAT_MSG723";
        case WAVE_FORMAT_GSM610: return "WAVE_FORMAT_GSM610";
        case WAVE_FORMAT_MULAW:  return "WAVE_FORMAT_MULAW";
        case WAVE_FORMAT_AAC:    return "WAVE_FORMAT_AAC";

        case WAVE_FORMAT_DSPGROUP_TRUESPEECH: return "WAVE_FORMAT_DSPGROUP_TRUESPEECH";
        case WAVE_FORMAT_MPEGLAYER3:          return "WAVE_FORMAT_MPEGLAYER3";
        case WAVE_FORMAT_VOXWARE_AC10:        return "WAVE_FORMAT_VOXWARE_AC10";
        case WAVE_FORMAT_VOXWARE_AC16:        return "WAVE_FORMAT_VOXWARE_AC16";
        case WAVE_FORMAT_VOXWARE_AC20:        return "WAVE_FORMAT_VOXWARE_AC20";
        case WAVE_FORMAT_VOXWARE_AC8:         return "WAVE_FORMAT_VOXWARE_AC8";
    }

    return "<unknown>";
}

struct AudioFormat {

    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    uint16_t cbSize;

    AudioFormat() = default;

    AudioFormat(uint16_t wFormatTag,
                uint16_t nChannels,
                uint32_t nSamplesPerSec,
                uint32_t nAvgBytesPerSec,
                uint16_t nBlockAlign,
                uint16_t wBitsPerSample,
                uint16_t cbSize)
        : wFormatTag(wFormatTag)
        , nChannels(nChannels)
        , nSamplesPerSec(nSamplesPerSec)
        , nAvgBytesPerSec(nAvgBytesPerSec)
        , nBlockAlign(nBlockAlign)
        , wBitsPerSample(wBitsPerSample)
        , cbSize(cbSize) {}

    void emit(OutStream & stream) {
        stream.out_uint16_le(this->wFormatTag);
        stream.out_uint16_le(this->nChannels);
        stream.out_uint32_le(this->nSamplesPerSec);
        stream.out_uint32_le(this->nAvgBytesPerSec);
        stream.out_uint16_le(this->nBlockAlign);
        stream.out_uint16_le(this->wBitsPerSample);
        stream.out_uint16_le(this->cbSize);
    }

    void receive(InStream & stream) {
        const unsigned expected = 18;   // wFormatTag(2) + nChannels(2) + nSamplesPerSec(4) +
                                        // nAvgBytesPerSec(4) + nBlockAlign(2) + wBitsPerSample(2) +
                                        // cbSize(2)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated AudioFormat (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }

        this->wFormatTag = stream.in_uint16_le();
        this->nChannels = stream.in_uint16_le();
        this->nSamplesPerSec = stream.in_uint32_le();
        this->nAvgBytesPerSec = stream.in_uint32_le();
        this->nBlockAlign = stream.in_uint16_le();
        this->wBitsPerSample = stream.in_uint16_le();
        this->cbSize = stream.in_uint16_le();

        stream.in_skip_bytes(this->cbSize);
    }

    void log() {
        LOG(LOG_INFO, "     Audio Format:");
        LOG(LOG_INFO, "          * wFormatTag      = 0x%04x (2 bytes): %s", this->wFormatTag, get_wFormatTag_name(this->wFormatTag));
        LOG(LOG_INFO, "          * nChannels       = 0x%04x (2 bytes)", this->nChannels);
        LOG(LOG_INFO, "          * nSamplesPerSec  = 0x%08x (4 bytes)", this->nSamplesPerSec);
        LOG(LOG_INFO, "          * nAvgBytesPerSec = 0x%08x (4 bytes)", this->nAvgBytesPerSec);
        LOG(LOG_INFO, "          * nBlockAlign     = 0x%04x (2 bytes)", this->nBlockAlign);
        LOG(LOG_INFO, "          * wBitsPerSample  = 0x%04x (2 bytes)", this->wBitsPerSample);
        LOG(LOG_INFO, "          * cbSize          = %zu (2 bytes)", std::size_t(this->cbSize));
    }
};



// 2.2.2.2 Client Audio Formats and Version PDU (CLIENT_AUDIO_VERSION_AND_FORMATS)
//
// The Client Audio Formats and Version PDU is a PDU that is used to send version information,
// capabilities, and a list of supported audio formats from the client to the server.<5> After
// the server sends its version and a list of supported audio formats to the client, the
// client sends back a Client Audio Formats and Version PDU to the server containing its
// version and a list of formats that both the client and server support. This PDU MUST
// be sent by using virtual channels.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Header                           |
// +---------------------------------------------------------------+
// |                             dwFlags                           |
// +---------------------------------------------------------------+
// |                             dwVolume                          |
// +---------------------------------------------------------------+
// |                             dwPitch                           |
// +-------------------------------+-------------------------------+
// |          wDGramPort           |        wNumberOfFormats       |
// +---------------+---------------+---------------+---------------+
// |cLastBlockConf |            wVersion           |     bPad      |
// +---------------+-------------------------------+---------------+
// |                       sndFormats (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPSND PDU header (section 2.2.1). The msgType field of the RDPSND
// PDU header MUST be set to SNDC_FORMATS (0x07).
//
// dwFlags (4 bytes): A 32-bit unsigned integer that specifies the general capability flags.
// The dwFlags field MUST be one or more of the following flags, combined with a bitwise OR operator.

//  +-----------------------------+------------------------------------------------------+
//  | Value                       | Meaning                                              |
//  +-----------------------------+------------------------------------------------------+
//  | TSSNDCAPS_ALIVE             | The client is capable of consuming audio data. This  |
//  | 0x00000001                  | flag MUST be set for audio data to be transferred.   |
//  +-----------------------------+------------------------------------------------------+
//  | TSSNDCAPS_VOLUME            | The client is capable of applying a volume change to |
//  | 0x00000002                  | all the audio data that is received.                 |
//  +-----------------------------+------------------------------------------------------+
//  | TSSNDCAPS_PITCH             | The client is capable of applying a pitch change to  |
//  | 0x00000004                  | all the audio data that is received.                 |
//  +-----------------------------+------------------------------------------------------+

// dwVolume (4 bytes): A 32-bit unsigned integer. If the TSSNDCAPS_VOLUME flag is not set in
// the dwFlags field, the dwVolume field MUST be ignored. If the TSSNDCAPS_VOLUME flag is set
// in the dwFlags field, the dwVolume field specifies the initial volume of the audio stream.
// The low-order word contains the left-channel volume setting, and the high-order word
// contains the right-channel setting. A value of 0xFFFF represents full volume, and a
// value of 0x0000 is silence.
//
// This value is to be interpreted logarithmically. This means that the perceived increase
// in volume is the same when increasing the volume level from 0x5000 to 0x6000 as it is
// from 0x4000 to 0x5000.
//
// dwPitch (4 bytes): A 32-bit unsigned integer. If the TSSNDCAPS_PITCH flag is not set in the
// dwFlags field, the dwPitch field MUST be ignored. If the TSSNDCAPS_PITCH flag is set in the
// dwFlags field, the dwPitch field specifies the initial pitch of the audio stream. The pitch
// is specified as a fixed-point value. The high-order word contains the signed integer part
// of the number, and the low-order word contains the fractional part. A value of 0x8000 in
// the low-order word represents one-half, and 0x4000 represents one-quarter. For example,
// the value 0x00010000 specifies a multiplier of 1.0 (no pitch change), and a value of
// 0x000F8000 specifies a multiplier of 15.5.
//
// wDGramPort (2 bytes): A 16-bit unsigned integer that, if set to a nonzero value, specifies
// the client port that the server MUST use to send data over UDP. A zero value means UDP is
// not supported. This field MUST be specified by using big-endian byte ordering.
//
// wNumberOfFormats (2 bytes): A 16-bit unsigned integer that specifies the number of
// AUDIO_FORMAT structures that are contained in an sndFormats array.
//
// cLastBlockConfirmed (1 byte): An 8-bit unsigned integer. This field is unused. The value
// is arbitrary and MUST be ignored on receipt.
//
// wVersion (2 bytes): A 16-bit unsigned integer that specifies the version of the protocol
// that is supported by the client.<6>
//
// bPad (1 byte): An 8-bit unsigned integer. This field is unused. The value is arbitrary
// and MUST be ignored on receipt.
//
// sndFormats (variable): A variable-sized array of audio formats that are supported by
// the client and the server, each conforming in structure to the AUDIO_FORMAT. Each audio
// format MUST also appear in the Server Audio Formats and Version PDU list of audio formats
// just sent by the server. The number of formats in the array is wNumberOfFormats.

enum : uint32_t {
    TSSNDCAPS_ALIVE  = 0x00000001,
    TSSNDCAPS_VOLUME = 0x00000002,
    TSSNDCAPS_PITCH  = 0x00000003
};

struct ClientAudioFormatsandVersionHeader{

    uint32_t dwFlags;
    uint32_t dwVolume;
    uint32_t dwPitch;
    uint16_t wDGramPort;
    uint16_t wNumberOfFormats;
    uint16_t wVersion;

    ClientAudioFormatsandVersionHeader() = default;

    ClientAudioFormatsandVersionHeader(uint32_t dwFlags,
                                       uint32_t dwVolume,
                                       uint32_t dwPitch,
                                       uint16_t wDGramPort,
                                       uint16_t wNumberOfFormats,
                                       uint16_t wVersion)
        : dwFlags(dwFlags)
        , dwVolume(dwVolume)
        , dwPitch(dwPitch)
        , wDGramPort(wDGramPort)
        , wNumberOfFormats(wNumberOfFormats)
        , wVersion(wVersion) {}


    void emit(OutStream & stream) {
        stream.out_uint32_le(this->dwFlags);
        stream.out_uint32_le(this->dwVolume);
        stream.out_uint32_le(this->dwPitch);
        stream.out_uint16_le(this->wDGramPort);
        stream.out_uint16_le(this->wNumberOfFormats);
        stream.out_clear_bytes(1);
        stream.out_uint16_le(this->wVersion);
        stream.out_clear_bytes(1);
    }

    void receive(InStream & stream) {
        const unsigned expected = 20;           // dwFlags(4) + dwVolume(4) + dwPitch(4) + wDGramPort(2) +
                                                // wNumberOfFormats(2) + cLastBlockConfirmed(1) +
                                                // wVersion(2) + bPad(1)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated ClientAudioFormatsandVersionHeader (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }
        this->dwFlags = stream.in_uint32_le();
        this->dwVolume = stream.in_uint32_le();
        this->dwPitch = stream.in_uint32_le();
        this->wDGramPort = stream.in_uint16_le();
        this->wNumberOfFormats = stream.in_uint16_le();
        stream.in_skip_bytes(1);
        this->wVersion = stream.in_uint16_le();
        stream.in_skip_bytes(1);
    }

    void log() {
        LOG(LOG_INFO, "     Client Audio Formats and Version Header:");
        LOG(LOG_INFO, "          * dwFlags             = 0x%08x (4 bytes)", this->dwFlags);
        LOG(LOG_INFO, "          * dwVolume            = 0x%08x (4 bytes)", this->dwVolume);
        LOG(LOG_INFO, "          * dwPitch             = 0x%08x (4 bytes)", this->dwPitch);
        LOG(LOG_INFO, "          * wDGramPort          = 0x%04x (2 bytes)", this->wDGramPort);
        LOG(LOG_INFO, "          * wNumberOfFormats    = %d (2 bytes)", int(this->wNumberOfFormats));
        LOG(LOG_INFO, "          * cLastBlockConfirmed - (1 byte) NOT USED");
        LOG(LOG_INFO, "          * wVersion            = 0x%04x (2 bytes)", this->wVersion);
        LOG(LOG_INFO, "          * bPad                - (1 byte)  NOT USED");
    }

};



// 2.2.2.3 Quality Mode PDU
//
// The Quality Mode PDU is a PDU used by the client to select one of three quality modes.
// If both the client and server are at least version 6, the client MUST send a Quality Mode
// PDU immediately after sending the audio formats. This packet is only used when the client
// and server versions are both at least 6.<7> This PDU MUST be sent using virtual channels.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Header                           |
// +-------------------------------+-------------------------------+
// |         wQualityMode          |           Reserved            |
// +-------------------------------+-------------------------------+

// Header (4 bytes): An RDPSND PDU Header (section 2.2.1). The msgType field of the RDPSND PDU
// Header MUST be set to SNDC_QUALITYMODE (0x0C).
//
// wQualityMode (2 bytes): A 16-bit unsigned integer. This field specifies the quality setting
// the client has requested. The definition of these three modes is implementation-dependent, but
// SHOULD use the following guidelines.

//  +-----------------------------+------------------------------------------------------+
//  | Value                       | Meaning                                              |
//  +-----------------------------+------------------------------------------------------+
//  | DYNAMIC_QUALITY             | The server dynamically adjusts the audio format to   |
//  | 0x0000                      | best match the bandwidth and latency characteristics |
//  |                             | of the network.                                      |
//  +-----------------------------+------------------------------------------------------+
//  | MEDIUM_QUALITY              | The server chooses an audio format from the list of  |
//  | 0x0001                      | formats the client supports that gives moderate audio|
//  |                             |  quality and requires a moderate amount of bandwidth.|
//  +-----------------------------+------------------------------------------------------+
//  | HIGH_QUALITY                | The server chooses the audio format that provides the|
//  | 0x0002                      | best quality audio without regard to the bandwidth   |
//  |                             | requirements for that format.                        |
//  +-----------------------------+------------------------------------------------------+

// Reserved (2 bytes): A 16-bit unsigned integer. This field is unused. The value is arbitrary
// and MUST be ignored on receipt.

enum : uint16_t {
    DYNAMIC_QUALITY = 0x0000,
    MEDIUM_QUALITY  = 0x0001,
    HIGH_QUALITY    = 0x0002
};

static inline const char * get_wQualityMode_name(uint16_t wQualityMode) {
    switch (wQualityMode) {
        case DYNAMIC_QUALITY: return "DYNAMIC_QUALITY";
        case MEDIUM_QUALITY: return "MEDIUM_QUALITY";
        case HIGH_QUALITY: return "HIGH_QUALITY";
    }

    return "unknow";
}

struct QualityModePDU {

    uint16_t wQualityMode;

    QualityModePDU() = default;

    explicit QualityModePDU(int16_t wQualityMode)
      : wQualityMode(wQualityMode) {}

    void emit(OutStream & stream) {
        stream.out_uint16_le(this->wQualityMode);
        stream.out_clear_bytes(2);
    }

    void receive(InStream & stream) {
        const unsigned expected = 4;           // wQualityMode(2) + Reserved(2)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated QualityModePDU (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }
        this->wQualityMode = stream.in_uint16_le();
        stream.in_skip_bytes(2);
    }

    void log() {
        LOG(LOG_INFO, "     Quality Mode PDU:");
        LOG(LOG_INFO, "          * wQualityMode = 0x%04x (2 bytes): %s", this->wQualityMode, get_wQualityMode_name(this->wQualityMode));
        LOG(LOG_INFO, "          * Reserved - (2 bytes)  NOT USED");
    }

};



// 2.2.3.1 Training PDU (SNDTRAINING)
//
// The Training PDU is a PDU used by the server to request that the client send it a
// Training Confirm PDU. In response, the client MUST immediately send a Training Confirm
// PDU to the server. The server uses the sending and receiving of these packets for
// diagnostic purposes. This PDU can be sent using virtual channels or UDP.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Header                           |
// +-------------------------------+-------------------------------+
// |         wTimeStamp            |           wPackSize           |
// +-------------------------------+-------------------------------+
// |                         data (variable)                       |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPSND PDU Header (section 2.2.1). The msgType field of the
// RDPSND PDU Header MUST be set to SNDC_TRAINING (0x06).
//
// wTimeStamp (2 bytes): A 16-bit unsigned integer. In the Training PDU this value is arbitrary.
//
// wPackSize (2 bytes): A 16-bit unsigned integer. If the size of data is nonzero,
// then this field specifies the size, in bytes, of the entire PDU. If the size of
// data is 0, then wPackSize MUST be 0.
//
// data (variable): Unused. The value in this field is arbitrary and MUST be ignored on receipt.

struct TrainingPDU {

    uint16_t wTimeStamp;
    uint16_t wPackSize;

    TrainingPDU() = default;

    TrainingPDU( uint16_t wTimeStamp
               , uint16_t wPackSize)
        : wTimeStamp(wTimeStamp)
        , wPackSize(wPackSize) {}

    void emit(OutStream & stream) {
        stream.out_uint16_le(this->wTimeStamp);
        stream.out_uint16_le(this->wPackSize);
    }

    void receive(InStream & stream) {
        const unsigned expected = 4;           // wTimeStamp(2) + wPackSize(2)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated TrainingPDU (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }
        this->wTimeStamp = stream.in_uint16_le();
        this->wPackSize = stream.in_uint16_le();
       // stream.in_skip_bytes(this->wPackSize);
    }

    void log() {
        LOG(LOG_INFO, "     Training PDU:");
        LOG(LOG_INFO, "          * wTimeStamp = 0x%04x (2 bytes)", this->wTimeStamp);
        LOG(LOG_INFO, "          * wPackSize  = %zu (2 bytes)", std::size_t(this->wPackSize));
        LOG(LOG_INFO, "          * Data - (%zu bytes)  NOT USED", std::size_t(this->wPackSize));
    }
};



// 2.2.3.2 Training Confirm PDU (SNDTRAININGCONFIRM)
//
// The Training Confirm PDU is a PDU sent by the client to confirm the reception
// of a Training PDU. This PDU MUST be sent using virtual channels or UDP.
// The server MAY use data from this PDU to calculate how fast the network can
// transmit data, as described in section 3.3.5.1.1.5.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Header                           |
// +-------------------------------+-------------------------------+
// |         wTimeStamp            |           wPackSize           |
// +-------------------------------+-------------------------------+

// Header (4 bytes): An RDPSND PDU Header (section 2.2.1). The msgType field of the
// RDPSND PDU Header MUST be set to SNDC_TRAINING (0x06).
//
// wTimeStamp (2 bytes): A 16-bit unsigned integer. In the Training PDU this value
// is arbitrary.
//
// wPackSize (2 bytes): A 16-bit unsigned integer. If the size of data is nonzero, then
// this field specifies the size, in bytes, of the entire PDU. If the size of data is 0,
// then wPackSize MUST be 0.
//
// data (variable): Unused. The value in this field is arbitrary and MUST be ignored
// on receipt.

struct TrainingConfirmPDU {

    uint16_t wTimeStamp;
    uint16_t wPackSize;

    TrainingConfirmPDU() = default;

    TrainingConfirmPDU( uint16_t wTimeStamp
               , uint16_t wPackSize)
        : wTimeStamp(wTimeStamp)
        , wPackSize(wPackSize) {}

    void emit(OutStream & stream) {
        stream.out_uint16_le(this->wTimeStamp);
        stream.out_uint16_le(this->wPackSize);
    }

    void receive(InStream & stream) {
        const unsigned expected = 4;           // wTimeStamp(2) + wPackSize(2)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated TrainingConfirmPDU (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }
        this->wTimeStamp = stream.in_uint16_le();
        this->wPackSize = stream.in_uint16_le();
       // stream.in_skip_bytes(this->wPackSize);
    }

    void log() {
        LOG(LOG_INFO, "     Training Confirm PDU:");
        LOG(LOG_INFO, "          * wTimeStamp = 0x%04x (2 bytes)", this->wTimeStamp);
        LOG(LOG_INFO, "          * wPackSize  = %zu (2 bytes)", std::size_t(this->wPackSize));
    }
};



// 2.2.3.3 WaveInfo PDU (SNDWAVINFO)

// The WaveInfo PDU is the first of two consecutive PDUs used to transmit audio
// data over virtual channels. This packet contains information about the audio data
// along with the first 4 bytes of the audio data itself. This PDU MUST be sent
// using static virtual channels.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Header                           |
// +-------------------------------+-------------------------------+
// |          wTimeStamp           |           wFormatNo           |
// +---------------+---------------+-------------------------------+
// |cLastBlockConf |                     bPad                      |
// +---------------+-----------------------------------------------+
// |                             Data                              |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPSND PDU Header (section 2.2.1). The msgType field of
// the RDPSND PDU Header MUST be set to SNDC_WAVE (0x02). The BodySize field of the
// RDPSND PDU Header is the size of the WaveInfo PDU plus the size of the data field
// of the Wave PDU that immediately follows this packet minus the size of the Header.
//
// wTimeStamp (2 bytes): A 16-bit unsigned integer representing the time stamp of
// the audio data. It SHOULD be set to a time that represents when this PDU is built.<8>
//
// wFormatNo (2 bytes): A 16-bit unsigned integer that represents an index into the
// list of audio formats exchanged between the client and server during the initialization
// phase, as described in section 3.1.1.2. The format located at that index is the
// format of the audio data in this PDU and the Wave PDU that immediately follows
// this packet.
//
// cBlockNo (1 byte): An 8-bit unsigned integer specifying the block ID of the audio
// data. When the client notifies the server that it has consumed the audio data, it
// sends a Wave Confirm PDU (section 2.2.3.8) containing this field in its
// cConfirmedBlockNo field.
//
// bPad (3 bytes): A 24-bit unsigned integer. This field is unused. The value is
// arbitrary and MUST be ignored on receipt.
//
// Data (4 bytes): The first four bytes of the audio data. The rest of the audio data
// arrives in the next PDU, which MUST be a Wave PDU. The audio data MUST be in the audio
// format from the list of formats exchanged during the Initialization Sequence (section 2.2.2);
// this list is found at the index specified in the wFormatNo field.

struct WaveInfoPDU {

    uint16_t wTimeStamp;
    uint16_t wFormatNo;
    uint8_t  cBlockNo;
    uint8_t  Data[4];

    WaveInfoPDU() = default;

    WaveInfoPDU( uint16_t wTimeStamp
               , uint16_t wFormatNo
               , uint8_t  cBlockNo)
        : wTimeStamp(wTimeStamp)
        , wFormatNo(wFormatNo)
        , cBlockNo(cBlockNo) {}

    void emit(OutStream & stream) {
        stream.out_uint16_le(this->wTimeStamp);
        stream.out_uint16_le(this->wFormatNo);
        stream.out_uint8(this->cBlockNo);
        stream.out_clear_bytes(3);
        stream.out_copy_bytes(this->Data, 4);
    }

    void receive(InStream & stream) {
        const unsigned expected = 12;   // wTimeStamp(2) + wFormatNo(2) + cBlockNo(1) + bPad(3) + Data (4)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated WaveInfoPDU (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }
        this->wTimeStamp = stream.in_uint16_le();
        this->wFormatNo = stream.in_uint16_le();
        this->cBlockNo = stream.in_uint8();
        stream.in_skip_bytes(3);
        stream.in_copy_bytes(this->Data, 4);
    }

    void log() {
        LOG(LOG_INFO, "     Wave Info PDU:");
        LOG(LOG_INFO, "          * wTimeStamp = 0x%04x (2 bytes)", this->wTimeStamp);
        LOG(LOG_INFO, "          * wFormatNo  = 0x%04x (2 bytes)", this->wFormatNo);
        LOG(LOG_INFO, "          * cBlockNo   = 0x%02x (1 byte)", this->cBlockNo);
        LOG(LOG_INFO, "          * bPad - (3 bytes)  NOT USED");
        LOG(LOG_INFO, "          * Data       = \"0x%02x0x%02x0x%02x0x%02x\" (4 byte)",
                        this->Data[0], this->Data[1], this->Data[2], this->Data[3]);
    }
};



// 2.2.3.8 Wave Confirm PDU (SNDWAV_CONFIRM)
//
// The Wave Confirm PDU is a PDU that MUST be sent by the client to the server
// immediately after the following two events occur:
//
// An audio data sample is received from the server, whether using a WaveInfo
// PDU and Wave PDU, a Wave2 PDU, a Wave Encrypt PDU, or several UDP Wave PDUs
// followed by a UDP Wave Last PDU.
//
//     The audio data sample is emitted to completion by the client.
//
// This PDU can be sent using static virtual channels or UDP.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Header                           |
// +-------------------------------+---------------+---------------+
// |          wTimeStamp           |  cConfBlockNo |      bPad     |
// +---------------+---------------+---------------+---------------+

// Header (4 bytes): An RDPSND PDU Header (section 2.2.1). The msgType field of the
// RDPSND PDU Header MUST be set to SNDC_WAVECONFIRM (0x05).
//
// wTimeStamp (2 bytes): A 16-bit unsigned integer. See section 3.2.5.2.1.6 for
// details of how this field is set.
//
// cConfBlockNo (1 byte): Confirmation Block Number. An 8-bit unsigned
// integer that MUST be the same as the cBlockNo field of the
// UDP Wave Last PDU (section 2.2.3.7), the Wave Encrypt PDU (section
// 2.2.3.5) or the WaveInfo PDU (section 2.2.3.3) just received from the server.
//
// bPad (1 byte): An unsigned 8-bit integer. This field is unused. The value is
// arbitrary and MUST be ignored on receipt.

struct WaveConfirmPDU {

    uint16_t wTimeStamp;
    uint8_t  cConfBlockNo;

    WaveConfirmPDU() = default;

    WaveConfirmPDU( uint16_t wTimeStamp
                  , uint8_t  cConfBlockNo)
        : wTimeStamp(wTimeStamp)
        , cConfBlockNo(cConfBlockNo) {}

    void emit(OutStream & stream) {
        stream.out_uint16_le(this->wTimeStamp);
        stream.out_uint8(this->cConfBlockNo);
        stream.out_clear_bytes(1);
    }

    void receive(InStream & stream) {
        const unsigned expected = 4;   // wTimeStamp(2) + cBlockNo(1) + bPad(1)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated WaveConfirmPDU (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }
        this->wTimeStamp = stream.in_uint16_le();
        this->cConfBlockNo = stream.in_uint8();
        stream.in_skip_bytes(1);
    }

    void log() {
        LOG(LOG_INFO, "     Wave Confirm PDU:");
        LOG(LOG_INFO, "          * wTimeStamp   = 0x%04x (2 bytes)", this->wTimeStamp);
        LOG(LOG_INFO, "          * cConfBlockNo = 0x%02x (1 byte)", this->cConfBlockNo);
        LOG(LOG_INFO, "          * bPad - (1 byte) NOT USED");
    }
};



// 2.2.3.10 Wave2 PDU (SNDWAVE2)
//
// The Wave2 PDU is used to transmit audio data over virtual channels.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +-------------------------------+-------------------------------+
// |          wTimeStamp           |           wFormatNo           |
// +---------------+---------------+-------------------------------+
// |    cBlockNo   |                     bPad                      |
// +---------------+-----------------------------------------------+
// |                        dwAudioTimeStamp                       |
// +---------------------------------------------------------------+
// |                         Data (variable)                       |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPSND PDU Header (section 2.2.1). The msgType field
// of the RDPSND PDU Header MUST be set to SNDC_WAVE2 (0x0D). The BodySize
// field of the RDPSND PDU Header is the size of this PDU minus the size of
// the header.
//
// wTimeStamp (2 bytes): A 16-bit unsigned integer representing the time stamp
// of the audio data. It SHOULD<10> be set to a time that represents when this PDU is built.
//
// wFormatNo (2 bytes): A 16-bit unsigned integer that represents an index into
// the list of audio formats exchanged between the client and server during the
// initialization phase, as described in section 3.1.1.2. The format located at
// that index is the format of the audio data in this PDU and the Wave PDU that
// immediately follows this packet.
//
// cBlockNo (1 byte): An 8-bit unsigned integer specifying the block ID of the
// audio data. When the client notifies the server that it has consumed the audio
// data, it sends a Wave Confirm PDU (section 2.2.3.8) containing this field in
// its cConfirmedBlockNo field.
//
// bPad (3 bytes): A 24-bit unsigned integer. This field is unused. The value
// is arbitrary and MUST be ignored on receipt.
//
// dwAudioTimeStamp (4 bytes): A 32-bit unsigned integer representing the timestamp
// when the server gets audio data from the audio source. The timestamp is the number
// of milliseconds that have elapsed since the system was started. This timestamp
// SHOULD be used to sync the audio stream with a video stream remoted using the
// Remote Desktop Protocol: Video Optimized Remoting Virtual Channel Extension (see
// the hnsTimestampOffset and hnsTimestamp fields as specified in [MS-RDPEVOR]
// sections 2.2.1.2 and 2.2.1.6, respectively).
//
// Data (variable): Audio data. The format of the audio data MUST be the format
// specified in the list of formats exchanged during the initialization sequence and
// found at the index specified in the wFormatNo field.

struct Wave2PDU {

    uint16_t wTimeStamp;
    uint16_t wFormatNo;
    uint8_t  cBlockNo;
    uint32_t dwAudioTimeStamp;


    Wave2PDU() = default;

    Wave2PDU( uint16_t wTimeStamp
               , uint16_t wFormatNo
               , uint8_t  cBlockNo
               , uint32_t dwAudioTimeStamp)
        : wTimeStamp(wTimeStamp)
        , wFormatNo(wFormatNo)
        , cBlockNo(cBlockNo)
        , dwAudioTimeStamp(dwAudioTimeStamp){}

    void emit(OutStream & stream) {
        stream.out_uint16_le(this->wTimeStamp);
        stream.out_uint16_le(this->wFormatNo);
        stream.out_uint8(this->cBlockNo);
        stream.out_clear_bytes(3);
        stream.out_uint16_le(this->dwAudioTimeStamp);
    }

    void receive(InStream & stream) {
        const unsigned expected = 8;   // wTimeStamp(2) + wFormatNo(2) + cBlockNo(1) + bPad(3)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated WaveInfoPDU (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }
        this->wTimeStamp = stream.in_uint16_le();
        this->wFormatNo = stream.in_uint16_le();
        this->cBlockNo = stream.in_uint8();
        stream.in_skip_bytes(3);
        this->dwAudioTimeStamp = stream.in_uint32_le();
    }

    void log() {
        LOG(LOG_INFO, "     Wave 2 PDU:");
        LOG(LOG_INFO, "          * wTimeStamp       = 0x%04x (2 bytes)", this->wTimeStamp);
        LOG(LOG_INFO, "          * wFormatNo        = 0x%04x (2 bytes)", this->wFormatNo);
        LOG(LOG_INFO, "          * cBlockNo         = 0x%02x (1 byte)", this->cBlockNo);
        LOG(LOG_INFO, "          * bPad - (3 bytes)  NOT USED");
        LOG(LOG_INFO, "          * dwAudioTimeStamp = 0x%08x (4 bytes)", this->dwAudioTimeStamp);
        LOG(LOG_INFO, "          * Data       ");
    }
};



// 2.2.4.1 Volume PDU (SNDVOL)
//
// The Volume PDU is a PDU sent from the server to the client to specify the volume to
// be set on the audio stream. For this packet to be sent, the client MUST have set the
// flag TSSNDCAPS_VOLUME (0x0000002) in the Client Audio Formats and Version PDU
// (section 2.2.2.2) that is sent during the initialization sequence described in
// section 2.2.2.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +---------------------------------------------------------------+
// |                             Volume                            |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPSND PDU Header (section 2.2.1). The msgType field of the RDPSND
// PDU Header MUST be set to SNDC_VOLUME (0x03).
//
// Volume (4 bytes): A 32-bit unsigned integer specifying the volume to be set on the audio
// stream. See the dwVolume field in section 2.2.2.2 for semantics of the data in this field.

struct VolumePDU {

    uint32_t Volume = 0;


    explicit VolumePDU() = default;

    explicit VolumePDU(uint32_t Volume)
        : Volume(Volume){}

    void emit(OutStream & stream) {
        stream.out_uint16_le(this->Volume);
    }

    void receive(InStream & stream) {
        const unsigned expected = 4;   // Volume(4)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Volume PDU (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }
        this->Volume = stream.in_uint32_le();
    }

    void log() {
        LOG(LOG_INFO, "    Volume PDU:");
        LOG(LOG_INFO, "          * Volume = 0x%08x (4 bytes)", this->Volume);
    }
};



// 2.2.4.2 Pitch PDU (SNDPITCH)
//
// The Pitch PDU is a PDU sent from the server to the client to specify the pitch to be set on
// the audio stream. For this packet to be sent, the client MUST have set the flag TSSNDCAPS_PITCH
// (0x0000004) in the Client Audio Formats and Version PDU (section 2.2.2.2) that is sent during
// the initialization sequence specified in section 2.2.2.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +---------------------------------------------------------------+
// |                              Pitch                            |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPSND PDU Header (section 2.2.1). The msgType field of the RDPSND PDU
// Header MUST be set to SNDC_PITCH (0x04).
//
// Pitch (4 bytes): A 32-bit unsigned integer. Although the server can send this PDU, the client
// MUST ignore it.

struct PitchPDU {

    uint32_t Pitch = 0;


    explicit PitchPDU() = default;

    explicit PitchPDU(uint32_t Pitch)
        : Pitch(Pitch){}

    void emit(OutStream & stream) {
        stream.out_uint16_le(this->Pitch);
    }

    void receive(InStream & stream) {
        const unsigned expected = 4;   // Pitch(4)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Pitch PDU (0): expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_FSCC_DATA_TRUNCATED);
        }
        this->Pitch = stream.in_uint32_le();
    }

    void log() {
        LOG(LOG_INFO, "    Pitch PDU:");
        LOG(LOG_INFO, "          * Pitch = 0x%08x (4 bytes)", this->Pitch);
    }
};



static inline void streamLogClient(InStream & stream, int flag) {
    if ( flag & CHANNELS::CHANNEL_FLAG_FIRST) {
        LOG(LOG_INFO, "received from Client:");
        RDPSNDPDUHeader header;
        header.receive(stream);
        header.log();

        switch (header.msgType) {

            case SNDC_FORMATS:
            {
                ClientAudioFormatsandVersionHeader safsvh;
                safsvh.receive(stream);
                safsvh.log();
                for (uint16_t i = 0; i < safsvh.wNumberOfFormats; i++) {
                    rdpsnd::AudioFormat format;
                    format.receive(stream);
                    format.log();
                }
            }
                break;

            case SNDC_TRAINING:
            {
                TrainingConfirmPDU train;
                train.receive(stream);
                train.log();
            }
                break;

            case SNDC_QUALITYMODE:
            {
                QualityModePDU qm;
                qm.receive(stream);
                qm.log();
            }
                break;

            case SNDC_WAVECONFIRM:
            {
                WaveConfirmPDU wc;
                wc.receive(stream);
                wc.log();
            }
                break;

            default: LOG(LOG_WARNING, "Client RDPSND Unknown PDU with length = %zu", header.BodySize);
                break;
        }
    }
}



static inline void streamLogServer(InStream & stream, int flag) {

    if ( flag & CHANNELS::CHANNEL_FLAG_FIRST) {
        LOG(LOG_INFO, "received from Server:");
        RDPSNDPDUHeader header;
        header.receive(stream);
        header.log();

        switch (header.msgType) {

            case SNDC_FORMATS:
            {
                ServerAudioFormatsandVersionHeader safsvh;
                safsvh.receive(stream);
                safsvh.log();
                for (uint16_t i = 0; i < safsvh.wNumberOfFormats; i++) {
                    rdpsnd::AudioFormat format;
                    format.receive(stream);
                    format.log();
                }
            }
                break;

            case SNDC_TRAINING:
            {
                TrainingPDU train;
                train.receive(stream);
                train.log();
            }
                break;

            case SNDC_WAVE:
            {
                WaveInfoPDU wi;
                wi.receive(stream);
                wi.log();
            }
                break;

            case SNDC_CLOSE:
            {
                LOG(LOG_INFO, "Server >> RDPSDN SNDC_CLOSE");
            }
                break;

            case SNDC_SETVOLUME:
            {
                VolumePDU v;
                v.receive(stream);
                v.log();
            }
                break;

            case SNDC_SETPITCH:
            {
                PitchPDU p;
                p.receive(stream);
                p.log();
            }
                break;

            case SNDC_QUALITYMODE:
            {
                QualityModePDU qm;
                qm.receive(stream);
                qm.log();
            }
                break;

            case SNDC_WAVE2:
            {
                Wave2PDU w2;
                w2.receive(stream);
                w2.log();
            }
                break;

            default:
                if (header.BodySize ==  0 && header.msgType ==  0) {
                    LOG(LOG_INFO, "Server sndc wave");
                } else {
                    LOG(LOG_WARNING, "Server RDPSND Unknown PDU with length = %zu", header.BodySize);
                }
                break;
        }
    }
}


}  // namespace rdpsnd
