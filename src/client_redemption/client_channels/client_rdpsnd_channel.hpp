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

#pragma once

#include "mod/rdp/rdp_verbose.hpp"
#include "utils/log.hpp"

#include "core/RDPEA/audio_output.hpp"

#include "client_redemption/mod_wrapper/client_channel_mod.hpp"
// #include "client_redemption/client_input_output_api/client_sound_api.hpp"
#include "client_redemption/client_input_output_api/rdp_sound_config.hpp"



#include <string>

class ClientOutputSoundAPI {

public:
    uint32_t n_sample_per_sec = 0;
    uint16_t bit_per_sample = 0;
    uint16_t n_channels = 0;
    uint16_t n_block_align = 0;
    uint32_t bit_per_sec = 0;

    virtual void init(size_t raw_total_size) = 0;
    virtual void setData(const uint8_t * data, size_t size) = 0;
    virtual void play() = 0;

    virtual ~ClientOutputSoundAPI() = default;

};


class ClientRDPSNDChannel {

    RDPVerbose verbose;
    ClientOutputSoundAPI * impl_sound;
    ClientChannelMod * callback;

    const uint32_t channel_flags;

    int wave_data_to_wait = 0;
    bool last_PDU_is_WaveInfo = false;

    uint16_t last_cBlockNo;
    uint16_t last_wTimeStamp;

    uint32_t dwFlags;
    uint32_t dwVolume;
    uint32_t dwPitch ;
    uint16_t wDGramPort;
    uint16_t wNumberOfFormats;
    uint16_t wVersion;


public:
    ClientRDPSNDChannel(RDPVerbose verbose, ClientChannelMod * callback, RDPSoundConfig & config);

    void receive(InStream & chunk);

    void set_api(ClientOutputSoundAPI * impl_sound);
};
