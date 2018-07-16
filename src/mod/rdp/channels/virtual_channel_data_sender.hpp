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
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "utils/log.hpp"
#include "utils/hexdump.hpp"

namespace
{
    void msgdump_impl(
        bool from_or_to_client,
        uint32_t total_length, uint32_t flags,
        const uint8_t* chunk_data, uint32_t chunk_data_length)
    {
        const uint32_t dest = (from_or_to_client
                            ? 0  // Client
                            : 1  // Server
                            );
        hexdump_c(reinterpret_cast<const uint8_t*>(&dest),
            sizeof(dest));
        hexdump_c(reinterpret_cast<uint8_t*>(&total_length),
            sizeof(total_length));
        hexdump_c(reinterpret_cast<uint8_t*>(&flags), sizeof(flags));
        hexdump_c(reinterpret_cast<uint8_t*>(&chunk_data_length),
            sizeof(chunk_data_length));
        hexdump_c(chunk_data, chunk_data_length);
    }
} // namespace

inline static void msgdump_c(bool send, bool from_or_to_client,
    uint32_t total_length, uint32_t flags, const uint8_t* chunk_data,
    uint32_t chunk_data_length)
{
    if (send) {
        LOG(LOG_INFO, "Sending on channel (%u) n bytes", chunk_data_length);
    }
    else {
        LOG(LOG_INFO, "Recv done on channel (%u) n bytes", chunk_data_length);
    }
    msgdump_impl(from_or_to_client, total_length, flags, chunk_data, chunk_data_length);
    if (send) {
        LOG(LOG_INFO, "Sent dumped on channel (%u) n bytes", chunk_data_length);
    }
    else {
        LOG(LOG_INFO, "Dump done on channel (%u) n bytes", chunk_data_length);
    }
}

inline static void msgdump_d(bool send, bool from_or_to_client,
    uint32_t total_length, uint32_t flags, const uint8_t* chunk_data,
    uint32_t chunk_data_length)
{
    if (send) {
        LOG(LOG_INFO, "Sending on channel (%u) n bytes", total_length);
    }
    else {
        LOG(LOG_INFO, "Recv done on channel (%u) n bytes", total_length);
    }
    msgdump_impl(from_or_to_client, total_length, flags, chunk_data, chunk_data_length);
    if (total_length > chunk_data_length) {
        LOG(LOG_INFO, "/* ---- */ \"...                                                             \" //................");
    }
    if (send) {
        LOG(LOG_INFO, "Sent dumped on channel (%u) n bytes", total_length);
    }
    else {
        LOG(LOG_INFO, "Dump done on channel (%u) n bytes", total_length);
    }
}

class VirtualChannelDataSender
{
public:
    virtual ~VirtualChannelDataSender() = default;

    virtual VirtualChannelDataSender& SynchronousSender() {
        return *this;
    }

    virtual void operator()(uint32_t total_length, uint32_t flags,
        const uint8_t * chunk_data, uint32_t chunk_data_length) = 0;
};

