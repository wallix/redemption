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
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo, David Fort
*/

#pragma once

#include "utils/log.hpp"


#include "client_redemption/client_redemption_api.hpp"



class ClientOutputSoundAPI {

public:
    uint32_t n_sample_per_sec = 0;
    uint16_t bit_per_sample = 0;
    uint16_t n_channels = 0;
    uint16_t n_block_align = 0;
    uint32_t bit_per_sec = 0;

    std::string path;

    void set_path(const std::string & path) {
        this->path = path;
    }

    virtual void init(size_t raw_total_size) = 0;
    virtual void setData(const uint8_t * data, size_t size) = 0;
    virtual void play() = 0;

    virtual ~ClientOutputSoundAPI() = default;

};

