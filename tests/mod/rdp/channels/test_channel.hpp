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
        bytes_view chunk_data
    ) override
    {
        StaticOutStream<4*4> stream;
        stream.out_uint32_le(dest);
        stream.out_uint32_le(total_length);
        stream.out_uint32_le(flags);
        stream.out_uint32_le(chunk_data.size());
        this->transport.send(stream.get_produced_bytes());
        this->transport.send(chunk_data);
    }
};

using TestToClientSender = TestSender<0>;
using TestToServerSender = TestSender<1>;

namespace tu
{

inline bool test_channel(Transport& t, BaseVirtualChannel& virtual_channel)
{
    uint8_t  virtual_channel_data[CHANNELS::CHANNEL_CHUNK_LENGTH + 8];
    InStream virtual_channel_stream(virtual_channel_data);

    while (true) {
        t.recv_boom(virtual_channel_data,
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

        auto chunk_data = t.recv_boom(virtual_channel_data, chunk_data_length);

        //hexdump_c(chunk_data, virtual_channel_stream.in_remain());

        if (!dest)  // Client
        {
            virtual_channel.process_client_message(total_length, flags, chunk_data);
        }
        else
        {
            std::unique_ptr<AsynchronousTask> out_asynchronous_task;

            virtual_channel.process_server_message(
                total_length, flags, chunk_data, out_asynchronous_task);

            if (out_asynchronous_task) {
                return false;
            }
        }

        virtual_channel_stream.rewind();
    }

    return true;
}

#define CHECK_CHANNEL(t, virtual_channel, sesman) RED_CHECK(tu::test_channel(t, virtual_channel));

} // namespace tu
