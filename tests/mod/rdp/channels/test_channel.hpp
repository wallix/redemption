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

#include "core/channel_list.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp"
#include "transport/transport.hpp"
#include "utils/stream.hpp"


template<uint32_t dest>
class TestSender : public VirtualChannelDataSender
{
    Transport& transport;

public:
    TestSender(Transport& transport)
        : transport(transport)
    {}

    void operator() (
        uint32_t total_length, uint32_t flags,
        const uint8_t* chunk_data, uint32_t chunk_data_length
    ) override
    {
        uint8_t tmp[sizeof(dest)+sizeof(total_length)+sizeof(flags)+sizeof(chunk_data_length)];
        ::out_bytes_be(tmp, sizeof(dest), dest);
        ::out_bytes_be(tmp + sizeof(dest), sizeof(total_length), total_length);
        ::out_bytes_be(tmp + sizeof(dest) + sizeof(total_length), sizeof(flags), flags);
        ::out_bytes_be(tmp + sizeof(dest) + sizeof(total_length) + sizeof(flags), sizeof(chunk_data_length), chunk_data_length);
        this->transport.send(tmp,sizeof(tmp));
        this->transport.send(chunk_data, chunk_data_length);
    }
};

using TestToClientSender = TestSender<0>;
using TestToServerSender = TestSender<1>;

bool test(Transport& t, BaseVirtualChannel& virtual_channel);

bool test(Transport& t, BaseVirtualChannel& virtual_channel)
{
    uint8_t  virtual_channel_data[CHANNELS::CHANNEL_CHUNK_LENGTH + 8];
    InStream virtual_channel_stream(virtual_channel_data);

    while (true) {
        auto end = virtual_channel_data;
        t.recv_boom(end,
                16    // dest(4) + total_length(4) + flags(4) +
                        //     chunk_length(4)
            );

        const uint32_t dest              =
            virtual_channel_stream.in_uint32_le();
        const uint32_t total_length      =
            virtual_channel_stream.in_uint32_le();
        const uint32_t flags             =
            virtual_channel_stream.in_uint32_le();
        const uint32_t chunk_data_length =
            virtual_channel_stream.in_uint32_le();

        //std::cout << "dest=" << dest <<
        //    ", total_length=" << total_length <<
        //    ", flags=" <<  flags <<
        //    ", chunk_data_length=" << chunk_data_length <<
        //    std::endl;

        end = virtual_channel_data;
        uint8_t * chunk_data = end;

        t.recv_boom(end, chunk_data_length);

        //hexdump_c(chunk_data, virtual_channel_stream.in_remain());

        if (!dest)  // Client
        {
            virtual_channel.process_client_message(
                total_length, flags, chunk_data, chunk_data_length);
        }
        else
        {
            std::unique_ptr<AsynchronousTask> out_asynchronous_task;

            virtual_channel.process_server_message(
                total_length, flags, chunk_data, chunk_data_length,
                out_asynchronous_task);

            if (out_asynchronous_task) {
                return false;
            }
        }

        virtual_channel_stream.rewind();
    }

    return true;
}

#define CHECK_CHANNEL(t, virtual_channel) RED_CHECK(test(t, virtual_channel));
