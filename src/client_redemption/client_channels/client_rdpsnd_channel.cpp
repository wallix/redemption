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
   Copyright (C) Wallix 2018
   Author(s): Clément Moroldo

   Unit test to writing RDP orders to file and rereading them
*/


#include "client_redemption/client_channels/client_rdpsnd_channel.hpp"




// [MS-RDPEA]: Remote Desktop Protocol: Audio Output Virtual Channel Extension
//
//
//
// 1.3.2 Audio Redirection Protocol
//
// Remote Desktop Protocol: Audio Output Virtual Channel Extension is divided into three distinct sequences:
//
//     Initialization Sequence (section 1.3.2.1)
//
//     The connection is established and capabilities and settings are exchanged.
//
//     Data Transfer Sequences (section 1.3.2.2)
//
//     Audio data is transferred.
//
//     Audio Setting Transfer Sequences (section 1.3.2.3)
//
//     Changes to audio settings are transferred.
//
//
// 1.3.2.1 Initialization Sequence
//
// The initialization sequence has the following goals:
//
//     Establish the client and server protocol versions and capabilities.
//
//     Establish a list of audio formats common to both the client and the server. All audio data transmits in a format specified in this list.
//
//     Determine if UDP can be used to transmit audio data.
//
// Initially, the server sends a Server Audio Formats and Version PDU, specifying its protocol version and supported audio formats to the client. In response, the client sends a Client Audio Formats and Version protocol data unit (PDU). At this point, the server and client have each other's versions, each other's capabilities, and a synchronized list of supported audio formats.
//
// If both the client and the server are at least version 6, the client has to send a Quality Mode PDU immediately after sending the Client Audio Formats and Version PDU.
//
// If the client wants to accept data over UDP, the client advertises a port to be used for UDP traffic. Given the client's port, the server attempts to use UDP to send a Training PDU to the client over the port. The client in turn attempts to reply with its own Training Confirm PDU. The server then attempts to send a private key (using a Crypt Key PDU) to the client, using the audio virtual channels. This key will be used to encrypt some data sent over UDP. If all of the preceding steps succeed, the data transfer sequences are sent over UDP. If any of the preceding steps fail, the data transfer sequences are sent over virtual channels.
//
// +-----------+                                                 +-----------+
// |  Server   |                                                 |   Local   |
// |           |                                                 |           |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       +----------Server Audio Formats and Version PDU (VC)--------> |
//       |                                                             |
//       | <---------Client Audio Formats and Version (VC)-------------+
//       |                                                             |
//       | <-----------------Quality Mode PDU (VC)---------------------+
//       |                                                             |
//       +--------------------Training PDU (UDP)---------------------> |
//       |                                                             |
//       | <--------------Training Confirm PDU (UDP)-------------------+
//       |                                                             |
//       +--------------------Crypt Key PDU (VC)---------------------> |
//       |                                                             |
//
// Figure 1: Initialization sequence using UDP for data transfer
//
// If all data transfer sequences are to be sent over virtual channels, the server and client exchange a Training PDU and a Training Confirm PDU over virtual channels.
//
// +-----------+                                                 +-----------+
// |  Server   |                                                 |   Local   |
// |           |                                                 |           |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       +----------Server Audio Formats and Version PDU (VC)--------> |
//       |                                                             |
//       | <---------Client Audio Formats and Version (VC)-------------+
//       |                                                             |
//       | <-----------------Quality Mode PDU (VC)---------------------+
//       |                                                             |
//       +--------------------Training PDU (UDP)---------------------> |
//       |                                                             |
//       | <--------------Training Confirm PDU (UDP)-------------------+
//       |                                                             |
//
// Figure 2: Initialization sequence using virtual channels for data transfer
//
//
// 1.3.2.2 Data Transfer Sequences
//
// The data transfer sequences have the goal of transferring audio data from the server to the client. Two different protocols exist for the data transfer sequences: one protocol transfers over virtual channels, and another transfers over UDP.
//
// The data transfer sequence over virtual channels has a very simple protocol. If the client version or server version is less than 8, the server sends two consecutive packets of audio data: a WaveInfo PDU (section 2.2.3.3) and a Wave PDU (section 2.2.3.4). Upon consuming the audio data, the client sends back a Wave Confirm PDU (section 2.2.3.8) to the server to notify the server that it has consumed the audio data. Consuming the audio data means it was processed, canceled, or dropped by the client. See section 3.2.5.2.1.6 for details of how the wTimeStamp field of the Wave Confirm PDU is set.
//
// +-----------+                                                 +-----------+
// |  Server   |                                                 |   Local   |
// |           |                                                 |           |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       +----------------------WaveInfo PDU (VC)--------------------> |
//       |                                                             |
//       +------------------------Wave PDU (VC)----------------------> |
//       |                                                             |
//       | <------------------Wave Confirm PDU (VC)--------------------+
//       |                                                             |
//
// Data transfer sequence over virtual channels using WaveInfo PDU and Wave PDU
//
// Figure 3: Data transfer sequence over virtual channels using WaveInfo PDU and Wave PDU
//
// If the client and server versions are both at least 8, the server sends Wave2 PDU (section 2.2.3.10). On consuming the audio data, the client sends back a Wave Confirm PDU (section 2.2.3.8) to the server to notify the server that it has consumed the audio data.
//
// +-----------+                                                 +-----------+
// |  Server   |                                                 |   Local   |
// |           |                                                 |           |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       +-----------------------Wave2 PDU (VC)----------------------> |
//       |                                                             |
//       |                                                             |
//       | <------------------Wave Confirm PDU (VC)--------------------+
//       |                                                             |
//
// Figure 4: Data transfer sequence over virtual channels using Wave2 PDU
//
// The protocol for the data transfer sequence over UDP is a little more involved. Similar to the protocol over virtual channels, the server sends a chunk of audio data to the client. When the client finishes consuming the audio data, the client sends back a Wave Confirm PDU to the server. The difference with the protocol used over virtual channels is how the server sends the audio data.
//
// If either the client or server version is less than 5, the server sends audio data using a Wave Encrypt PDU (section 2.2.3.5). Upon consumption of the audio data, the client sends a Wave Confirm PDU to the server.
//
// +-----------+                                                 +-----------+
// |  Server   |                                                 |   Local   |
// |           |                                                 |           |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       +--------------------WaveEncrypt PDU (UDP)------------------> |
//       |                                                             |
//       |                                                             |
//       | <-----------------Wave Confirm PDU (UDP)--------------------+
//       |                                                             |
//
// Figure 5: Data transfer sequence over UDP
//
// If the client and server versions are both at least 5, another method can be used to send audio data over UDP. This method involves the server sending the audio data in successive PDUs. All PDUs (except for the final one) are UDP Wave PDUs (section 2.2.3.6). The final PDU is a UDP Wave Last PDU (section 2.2.3.7). Given these PDUs, the client reconstructs the audio data sample. Upon consumption of audio data, the client sends a Wave Confirm PDU to the server.
//
// +-----------+                                                 +-----------+
// |  Server   |                                                 |   Local   |
// |           |                                                 |           |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       +----------------------UDP Wave PDU (UDP)-------------------> |
//       |                                                             |
//       +----------------------UDP Wave PDU (UDP)-------------------> |
//       |                                                             |
//       |                             ...                             |
//       |                                                             |
//       +----------------------UDP Wave PDU (UDP)-------------------> |
//       |                                                             |
//       +-------------------UDP Wave Last PDU (UDP)-----------------> |
//       |                                                             |
//       | <------------------Wave Confirm PDU (UDP)-------------------+
//       |                                                             |
//
// Data transfer sequence over UDP when protocol version is at least 5
//
// Figure 6: Data transfer sequence over UDP when protocol version is at least 5
//
// During the initialization sequence (section 1.3.2.1), the server uses the Crypt Key PDU (section 2.2.2.4) to send a 32-byte private key over a virtual channel to the client. Some audio data is encrypted using this key.
//
// At the end of the audio data transfer, the server notifies the client by sending a Close PDU (section 2.2.3.9) over a virtual channel.
//
//
// 1.3.2.3 Audio Setting Transfer Sequences
//
// The audio setting transfer sequence has the goal of transferring audio setting changes from the server to the client. Two different settings can be redirected: Volume and Pitch. All audio setting transfer sequences are sent over virtual channels. The settings are redirected using the Volume PDU (section 2.2.4.1) and Pitch PDU (section 2.2.4.2), respectively.



ClientRDPSNDChannel::ClientRDPSNDChannel(RDPVerbose verbose, ClientChannelMod * callback, RDPSoundConfig & config)
    : verbose(verbose)
    , impl_sound(nullptr)
    , callback(callback)
    , channel_flags(CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST)
    , last_cBlockNo(0)
    , last_wTimeStamp(0)
    , dwFlags(config.dwFlags)
    , dwVolume(config.dwVolume)
    , dwPitch(config.dwPitch)
    , wDGramPort(config.wDGramPort)
    , wNumberOfFormats(config.wNumberOfFormats)
    , wVersion(config.wVersion)
    {}

void ClientRDPSNDChannel::receive(InStream & chunk) {
    if (this->wave_data_to_wait) {

        this->wave_data_to_wait -= chunk.in_remain();
        if (this->wave_data_to_wait < 0) {
            this->wave_data_to_wait = 0;
        }

        if (this->last_PDU_is_WaveInfo) {
            chunk.in_skip_bytes(4);
            this->last_PDU_is_WaveInfo = false;
        }

        if (this->impl_sound) {
            this->impl_sound->setData(chunk.get_current(), chunk.in_remain());
        }

        if (!(this->wave_data_to_wait)) {

            if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                LOG(LOG_INFO, "SERVER >> RDPEA: Wave PDU");
            }

            LOG(LOG_INFO, "SERVER >> RDPEA: Wave PDU 1");

            if (this->impl_sound) {
                LOG(LOG_INFO, "SERVER >> RDPEA: Wave PDU 2");
                uint8_t data[] = {'\0'};
                LOG(LOG_INFO, "SERVER >> RDPEA: Wave PDU 3");
                this->impl_sound->setData(data, 1);
                LOG(LOG_INFO, "SERVER >> RDPEA: Wave PDU 4");
                this->impl_sound->play();
                LOG(LOG_INFO, "SERVER >> RDPEA: Wave PDU 5");
//                 LOG(LOG_INFO, "ClientRDPSNDChannel::receive play!!!");
            }

            LOG(LOG_INFO, "SERVER >> RDPEA: Wave PDU 6");

            StaticOutStream<16> out_stream;
            rdpsnd::RDPSNDPDUHeader header(rdpsnd::SNDC_WAVECONFIRM, 4);
            header.emit(out_stream);
            LOG(LOG_INFO, "SERVER >> RDPEA: Wave PDU 7");
            rdpsnd::WaveConfirmPDU wc(this->last_wTimeStamp, this->last_cBlockNo);
            wc.emit(out_stream);

            InStream chunk_to_send(out_stream.get_bytes());

            LOG(LOG_INFO, "SERVER >> RDPEA: Wave PDU 8");

            this->callback->send_to_mod_channel( channel_names::rdpsnd
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , this->channel_flags
                                                );
            if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                LOG(LOG_INFO, "CLIENT >> RDPEA: Wave Confirm PDU");
            }
        }

    } else {
        rdpsnd::RDPSNDPDUHeader header;
        header.receive(chunk);

        switch (header.msgType) {

            case rdpsnd::SNDC_FORMATS:
                {
                if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                    LOG(LOG_INFO, "SERVER >> RDPEA: Server Audio Formats and Version PDU");
                }

                rdpsnd::ServerAudioFormatsandVersionHeader safsvh;
                safsvh.receive(chunk);

                StaticOutStream<1024> out_stream;

                rdpsnd::RDPSNDPDUHeader header_out(rdpsnd::SNDC_FORMATS, 38);
                header_out.emit(out_stream);

                rdpsnd::ClientAudioFormatsandVersionHeader cafvh( this->dwFlags
                                                                , this->dwVolume
                                                                , this->dwPitch
                                                                , this->wDGramPort
                                                                , this->wNumberOfFormats
                                                                , this->wVersion
                                                                );
                cafvh.emit(out_stream);

                for (uint16_t i = 0; i < safsvh.wNumberOfFormats; i++) {
                    rdpsnd::AudioFormat format;
                    format.receive(chunk);
//                             format.log();

                    if (format.wFormatTag == rdpsnd::WAVE_FORMAT_PCM) {
                        format.emit(out_stream);
                        if (this->impl_sound) {
                            this->impl_sound->n_sample_per_sec = format.nSamplesPerSec;
                            this->impl_sound->bit_per_sample = format.wBitsPerSample;
                            this->impl_sound->n_channels = format.nChannels;
                            this->impl_sound->n_block_align = format.nBlockAlign;
                            this->impl_sound->bit_per_sec = format.nSamplesPerSec * (format.wBitsPerSample/8) * format.nChannels;
                        } else {
                            //LOG(LOG_WARNING, "No Sound System module found");
                        }
                    }
                }

                InStream chunk_to_send(out_stream.get_bytes());

                this->callback->send_to_mod_channel( channel_names::rdpsnd
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , this->channel_flags
                                                );

                if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                    LOG(LOG_INFO, "CLIENT >> RDPEA: Client Audio Formats and Version PDU");
                }

                StaticOutStream<32> quality_stream;

                rdpsnd::RDPSNDPDUHeader header_quality(rdpsnd::SNDC_QUALITYMODE, 8);
                header_quality.emit(quality_stream);

                rdpsnd::QualityModePDU qm(rdpsnd::HIGH_QUALITY);
                qm.emit(quality_stream);

                InStream chunk_to_send2(quality_stream.get_bytes());

                this->callback->send_to_mod_channel( channel_names::rdpsnd
                                                , chunk_to_send2
                                                , quality_stream.get_offset()
                                                , this->channel_flags
                                                );

                if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                    LOG(LOG_INFO, "CLIENT >> RDPEA: Quality Mode PDU");
                }
                }
                break;

            case rdpsnd::SNDC_TRAINING:
                {
                if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                    LOG(LOG_INFO, "SERVER >> RDPEA: Training PDU");
                }

                rdpsnd::TrainingPDU train;
                train.receive(chunk);

                StaticOutStream<32> out_stream;

                rdpsnd::RDPSNDPDUHeader header_quality(rdpsnd::SNDC_TRAINING, 4);
                header_quality.emit(out_stream);

                rdpsnd::TrainingConfirmPDU train_conf(train.wTimeStamp, train.wPackSize);
                train_conf.emit(out_stream);

                InStream chunk_to_send(out_stream.get_bytes());

                this->callback->send_to_mod_channel( channel_names::rdpsnd
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , this->channel_flags
                                                );

                if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                    LOG(LOG_INFO, "CLIENT >> RDPEA: Training Confirm PDU");
                }
                }
                break;

            case rdpsnd::SNDC_WAVE:
                {
                if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                    LOG(LOG_INFO, "SERVER >> RDPEA: Wave Info PDU");
                }

                this->wave_data_to_wait = header.BodySize - 8;
                rdpsnd::WaveInfoPDU wi;
                wi.receive(chunk);
                this->last_cBlockNo = wi.cBlockNo;
                this->last_wTimeStamp = wi.wTimeStamp;

                if (this->impl_sound) {
                    this->impl_sound->init(header.BodySize - 12);
                    this->impl_sound->setData(wi.Data, 4);
                }
                this->last_PDU_is_WaveInfo = true;
                }
                break;

            case rdpsnd::SNDC_CLOSE:
                if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                    LOG(LOG_INFO, "SERVER >> RDPEA: Close PDU");
                }
                break;

            case rdpsnd::SNDC_SETVOLUME:
                if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                    LOG(LOG_INFO, "SERVER >> RDPEA: SNDC_SETVOLUME PDU");
                }
                {
                rdpsnd::VolumePDU v;
                v.receive(chunk);
                }
                break;

            case rdpsnd::SNDC_SETPITCH:
                if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                    LOG(LOG_INFO, "SERVER >> RDPEA: SNDC_SETPITCH PDU");
                }
                {
                rdpsnd::PitchPDU p;
                p.receive(chunk);
                }
                break;

//                     case rdpsnd::SNDC_CRYPTKEY:
//                         LOG(LOG_INFO, "SERVER >> RDPEA: SNDC_CRYPTKEY PDU");
//                         break;

//                     case rdpsnd::SNDC_WAVEENCRYPT:
//                         LOG(LOG_INFO, "SERVER >> RDPEA: SNDC_WAVEENCRYPT PDU");
//                         break;

            case rdpsnd::SNDC_QUALITYMODE:
                if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                    LOG(LOG_INFO, "SERVER >> RDPEA: SNDC_QUALITYMODE PDU");
                }
                {
                rdpsnd::QualityModePDU qm;
                qm.receive(chunk);
                }
                break;

            case rdpsnd::SNDC_WAVE2:
                if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                    LOG(LOG_INFO, "SERVER >> RDPEA: SNDC_WAVE2 PDU");
                }
                {
                this->wave_data_to_wait = header.BodySize - 12;
                rdpsnd::Wave2PDU w2;
                w2.receive(chunk);
                if (this->impl_sound) {
                    this->impl_sound->init(header.BodySize - 12);
                    this->impl_sound->setData(chunk.get_current(), chunk.in_remain());
                }

                this->last_PDU_is_WaveInfo = true;
                }
                break;


            default: LOG(LOG_WARNING, "SERVER >> RDPEA: Unknown message type: %x", header.msgType);
                break;
        }
    }
}

    void ClientRDPSNDChannel::set_api(ClientOutputSoundAPI * impl_sound) {
        this->impl_sound = impl_sound;
    }

