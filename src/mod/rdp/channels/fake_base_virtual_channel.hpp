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
    Author(s): Christophe Grosjean, Raphael Zhou, Clément Moroldo
*/


#pragma once

#include "mod/rdp/channels/base_channel.hpp"
#include "utils/stream.hpp"

#include <vector>


class FakeBaseVirtualChannel final : public BaseVirtualChannel
{
public:
    struct PDUData {
        uint8_t data[1600] = {0};
        size_t size = 0;
    };

    PDUData to_server_stream[10];
    int index_server = 0;
    PDUData to_client_stream[10];
    int index_client = 0;

    FakeBaseVirtualChannel(const Params & params)
      :  BaseVirtualChannel(nullptr,
                            nullptr,
                            params) {}

    inline void send_message_to_client(uint32_t ,
        uint32_t , const uint8_t* chunk_data, uint32_t chunk_data_length) override
    {
        if (this->index_client < 10) {
            this->to_client_stream[this->index_client].size = chunk_data_length;
            std::memcpy(to_client_stream[this->index_client].data, chunk_data, chunk_data_length);
            this->index_client++;
        }
    }

    inline void send_message_to_server(uint32_t ,
        uint32_t , const uint8_t* chunk_data, uint32_t chunk_data_length) override
    {
        if (this->index_server < 10) {
            to_server_stream[this->index_server].size = chunk_data_length;         //chunk.in_remain();
            std::memcpy(to_server_stream[this->index_server].data, chunk_data, chunk_data_length);
            this->index_server++;
        }
    }

    void process_server_message(uint32_t /*total_length*/,
        uint32_t /*flags*/, const uint8_t* /*chunk_data*/,
        uint32_t /*chunk_data_length*/,
        std::unique_ptr<AsynchronousTask> & /*out_asynchronous_task*/) override {}

    protected:
    const char* get_reporting_reason_exchanged_data_limit_reached() const
        override
    {
        return "CLIPBOARD_LIMIT";
    }


};  // class ClipboardVirtualChannel

