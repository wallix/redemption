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

#pragma once

#include "utils/log.hpp"
#include "core/RDPEA/audio_output.hpp"
#include "../client_input_output_api.hpp"



class ClientChannelRDPSNDManager {

    RDPVerbose verbose;

    ClientOutputSoundAPI * impl_sound;

    ClientRedemptionIOAPI * client;

    int wave_data_to_wait = 0;
    bool last_PDU_is_WaveInfo = 0;


public:
    ClientChannelRDPSNDManager(RDPVerbose verbose, ClientRedemptionIOAPI * client, ClientOutputSoundAPI * impl_sound)
      : verbose(verbose)
      , impl_sound(impl_sound)
      , client(client)
      {}

    void receive(InStream & chunk) {
        if (this->wave_data_to_wait) {
    //                 if (bool(this->verbose & RDPVerbose::rdpsnd)) {
    //                     LOG(LOG_INFO, "SERVER >> RDPEA: Wave PDU size = %zu",  chunk_size);
    //                 }
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
                //this->sound_qt->setData(uint8_t('\0'), 1);

                if (this->impl_sound) {
                    this->impl_sound->play();
                }

    //                     msgdump_c(false, false, out_stream.get_offset(), 0, out_stream.get_data(), out_stream.get_offset());
    //                     header_out.log();
    //                     wc.log();
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
    //                         header.log();
    //                         safsvh.log();

                    StaticOutStream<1024> out_stream;

                    rdpsnd::RDPSNDPDUHeader header_out(rdpsnd::SNDC_FORMATS, 38);
                    header_out.emit(out_stream);

                    rdpsnd::ClientAudioFormatsandVersionHeader cafvh( rdpsnd::TSSNDCAPS_ALIVE |
                                                                        rdpsnd::TSSNDCAPS_VOLUME
                                                                    , 0x7fff7fff
                                                                    , 0
                                                                    , 0
                                                                    , 1
                                                                    , 0x06
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
                                LOG(LOG_WARNING, "No Sound System module found");
                            }
                        }
                    }

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rdpsnd
                                                    , chunk_to_send
                                                    , out_stream.get_offset()
                                                    , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST
                                                    );

                    if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                        LOG(LOG_INFO, "CLIENT >> RDPEA: Client Audio Formats and Version PDU");
                    }
    //                         msgdump_c(false, false, out_stream.get_offset(), 0, out_stream.get_data(), out_stream.get_offset());
    //                         header_out.log();
    //                         cafvh.log();

                    StaticOutStream<32> quality_stream;

                    rdpsnd::RDPSNDPDUHeader header_quality(rdpsnd::SNDC_QUALITYMODE, 4);
                    header_quality.emit(quality_stream);

                    rdpsnd::QualityModePDU qm(rdpsnd::HIGH_QUALITY);
                    qm.emit(quality_stream);

                    InStream chunk_to_send2(quality_stream.get_data(), quality_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rdpsnd
                                                    , chunk_to_send2
                                                    , quality_stream.get_offset()
                                                    , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST
                                                    );

                    if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                        LOG(LOG_INFO, "CLIENT >> RDPEA: Quality Mode PDU");
                    }
    //                         msgdump_c(false, false, quality_stream.get_offset(), 0, quality_stream.get_data(), quality_stream.get_offset());
    //                         header_out.log();
    //                         qm.log();
                    }
                    break;

                case rdpsnd::SNDC_TRAINING:
                    {
                    if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                        LOG(LOG_INFO, "SERVER >> RDPEA: Training PDU");
                    }

                    rdpsnd::TrainingPDU train;
                    train.receive(chunk);
    //                         header.log();
    //                         train.log();

                    StaticOutStream<32> out_stream;

                    rdpsnd::RDPSNDPDUHeader header_quality(rdpsnd::SNDC_TRAINING, 8);
                    header_quality.emit(out_stream);

                    rdpsnd::TrainingConfirmPDU train_conf(train.wTimeStamp, train.wPackSize);
                    train_conf.emit(out_stream);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rdpsnd
                                                    , chunk_to_send
                                                    , out_stream.get_offset()
                                                    , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST
                                                    );

                    if (bool(this->verbose & RDPVerbose::rdpsnd)) {
                        LOG(LOG_INFO, "CLIENT >> RDPEA: Training Confirm PDU");
                    }
    //                         msgdump_c(false, false, out_stream.get_offset(), 0, out_stream.get_data(), out_stream.get_offset());
    //                         header_quality.log();
    //                         train_conf.log();
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
};