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
#include "core/RDP/channels/drdynvc.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "utils/log.hpp"
#include "core/stream_throw_helpers.hpp"

class DynamicChannelVirtualChannel final : public BaseVirtualChannel
{
public:
    struct Params : public BaseVirtualChannel::Params {
        using BaseVirtualChannel::Params::Params;
    };

    explicit DynamicChannelVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        const Params & params)
    : BaseVirtualChannel(to_client_sender_,
                         to_server_sender_,
                         params)
    {}

    void process_client_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override
    {
        LOG_IF(bool(this->verbose & RDPVerbose::drdynvc), LOG_INFO,
            "DynamicChannelVirtualChannel::process_client_message: "
                "total_length=%u flags=0x%08X chunk_data_length=%zu",
            total_length, flags, chunk_data.size());

        if (bool(this->verbose & RDPVerbose::drdynvc_dump)) {
            const bool send              = false;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, total_length, flags, chunk_data);
        }

        uint8_t Cmd = 0x00;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            InStream chunk(chunk_data);

            /* cbId(:2) + Sp(:2) + Cmd(:4) */
            ::check_throw(chunk, 1, "DynamicChannelVirtualChannel::process_client_message", ERR_RDP_DATA_TRUNCATED);

            Cmd = ((chunk.in_uint8() & 0xF0) >> 4);
        }

        InStream chunk(chunk_data);

        bool send_message_to_server = true;

        switch (Cmd)
        {
            // case drdynvc::CMD_CAPABILITIES:
            // break;

            case drdynvc::CMD_CREATE:
                if (bool(this->verbose & RDPVerbose::drdynvc) &&
                    (flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)))
                {
                    drdynvc::DVCCreateResponsePDU create_response;

                    create_response.receive(chunk);
                    create_response.log(LOG_INFO);
                }
            break;

            default:
//                assert(false);

                LOG_IF(bool(this->verbose & RDPVerbose::drdynvc), LOG_INFO,
                    "DynamicChannelVirtualChannel::process_client_message: "
                        "Delivering unprocessed messages %s(%u) to server.",
                    drdynvc::get_DCVC_Cmd_name(Cmd),
                    static_cast<unsigned>(Cmd));
            break;
        }   // switch (Cmd)

        if (send_message_to_server) {
            this->send_message_to_server(total_length, flags, chunk_data);
        }
    }   // process_client_message

    void process_server_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data,
        std::unique_ptr<AsynchronousTask> & /*out_asynchronous_task*/,
        SesmanInterface & sesman) override
    {
        LOG_IF(bool(this->verbose & RDPVerbose::drdynvc), LOG_INFO,
            "DynamicChannelVirtualChannel::process_server_message: "
                "total_length=%u flags=0x%08X chunk_data_length=%zu",
            total_length, flags, chunk_data.size());

        if (bool(this->verbose & RDPVerbose::drdynvc_dump)) {
            const bool send              = false;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, total_length, flags, chunk_data);
        }

        uint8_t Cmd = 0x00;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            InStream chunk(chunk_data);

            /* cbId(:2) + Sp(:2) + Cmd(:4) */
            ::check_throw(chunk, 1, "DynamicChannelVirtualChannel::process_server_message", ERR_RDP_DATA_TRUNCATED);

            Cmd = ((chunk.in_uint8() & 0xF0) >> 4);
        }

        InStream chunk(chunk_data);

        bool send_message_to_client = true;

        switch (Cmd)
        {
            case drdynvc::CMD_CREATE:
                if (bool(this->verbose & RDPVerbose::drdynvc) &&
                    (flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)))
                {
                    drdynvc::DVCCreateRequestPDU create_request;

                    create_request.receive(chunk);
                    create_request.log(LOG_INFO);
                }
            break;

            default:
//                assert(false);

                LOG_IF(bool(this->verbose & RDPVerbose::drdynvc), LOG_INFO,
                    "DynamicChannelVirtualChannel::process_server_message: "
                        "Delivering unprocessed messages %s(%u) to client.",
                    drdynvc::get_DCVC_Cmd_name(Cmd),
                    static_cast<unsigned>(Cmd));
            break;
        }   // switch (Cmd)

        if (send_message_to_client) {
            this->send_message_to_client(total_length, flags, chunk_data);
        }   // switch (this->server_message_type)
    }   // process_server_message
};
