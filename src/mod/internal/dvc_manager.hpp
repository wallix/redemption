/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2017
 *   Author(s): Christophe Grosjean, Raphael Zhou
 */

#pragma once

#include "core/channel_list.hpp"
#include "core/front_api.hpp"
#include "core/RDP/channels/drdynvc.hpp"

class DVCManager
{
    FrontAPI * front_ = nullptr;

    const CHANNELS::ChannelDef * channel_ = nullptr;

    bool verbose;

public:
    explicit DVCManager(bool verbose = false)
    : verbose(verbose)
    {}

    DVCManager(const DVCManager &) = delete;
    DVCManager & operator=(const DVCManager &) = delete;

    bool ready(FrontAPI & front)
    {
        if (this->verbose) {
            LOG(LOG_INFO, "DVCManager::ready");
        }

        this->front_ = &front;

        this->channel_ = front.get_channel_list().get_by_name(channel_names::drdynvc);

        if (this->channel_) {
            if (this->verbose) {
                LOG(LOG_INFO, "DVCManager::ready: Channel");
            }

            StaticOutStream<256> out_s;
            drdynvc::DVCCapabilitiesRequestPDU DVCCapsReqPDU;

            DVCCapsReqPDU.emit(out_s);

            const size_t length     = out_s.get_offset();
            const size_t chunk_size = length;
            this->front_->send_to_channel(*(this->channel_), out_s.get_data(), length, chunk_size,
                                          CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST/* | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL*/);

            return true;
        }

        return false;
    }

    void send_to_mod_drdynvc_channel(size_t length, InStream & chunk, uint32_t flags)
    {
        if (this->verbose) {
            LOG(LOG_INFO,
                "DVCManager::send_to_mod_drdynvc_channel: "
                    "total_length=%zu flags=0x%08X chunk_data_length=%zu",
                length, flags, chunk.get_capacity());
        }
    }
};
