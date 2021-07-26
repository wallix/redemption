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
    Copyright (C) Wallix 2021
    Author(s): Florent Plard
*/

#pragma once

#include <memory>
#include <unordered_map>

#include "core/RDP/channels/rdpdr.hpp"
#include "core/front_api.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "scard/scard_pcsc_common.hpp"
#include "scard/scard_utils.hpp"
#include "utils/sugar/bytes_view.hpp"

class EventContainer;
class scard_rdp_channel_handler;
class VirtualChannelDataSender;


///////////////////////////////////////////////////////////////////////////////


///
class scard_rdp_channel_handler
{
public:
    ///
    virtual ~scard_rdp_channel_handler() = default;

    ///
    virtual void handle_rdp_channel_establishment() = 0;
    
    ///
    virtual void handle_rdp_channel_data(scard_wrapped_return data) = 0;
};

///
class scard_rdp_channel_sender : public VirtualChannelDataSender
{
public:
    ///
    explicit scard_rdp_channel_sender(FrontAPI &front,
        const CHANNELS::ChannelDef &channel_definition,
        bool verbose) noexcept;

    ///
    void operator()(uint32_t total_length, uint32_t flags,
        bytes_view chunk_data) override;

private:
    ///
    FrontAPI &_front;

    ///
    const CHANNELS::ChannelDef &_channel_definition;

    ///
    const bool _verbose;
};

///
class scard_rdp_channel final : public BaseVirtualChannel
{
public:
    ///
    scard_rdp_channel(
        EventContainer &events,
        VirtualChannelDataSender *sender,
        scard_rdp_channel_handler *handler_ptr,
        RDPVerbose verbosity);

    ///
    virtual ~scard_rdp_channel() = default;

    ///
    void establish();

    ///
    virtual void process_client_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override;

    ///
    virtual void process_server_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override;

    ///
    void send(scard_wrapped_call data);

private:
    /// Packet header.
    rdpdr::SharedHeader _packet_header;

    /// Client IDentifier sent by the client in the Client Announce Reply message.
    uint16_t _client_id;

    /// Pointer to the unique smartcard Device IDentifier sent by the client in the Device List Announce Request message.
    std::unique_ptr<uint32_t> _device_id_ptr;

    /// Output buffer length sent in the Device IO Request message.
    uint32_t _output_buffer_length;

    /// IO completion IDs.
    static_handle_table<uint32_t, 255, 0> _io_completion_ids;

    /// IO requests.
    std::unordered_map<
        uint32_t, uint32_t
    > _io_requests;

    ///
    RDPVerbose _verbosity;

    /// Handler for data coming from RDP client.
    scard_rdp_channel_handler *_handler_ptr;

    ///
    void announce();

    ///
    void process_client_announce_reply(InStream &chunk, uint32_t flags);

    ///
    void process_client_name_request(InStream &chunk, uint32_t flags);

    ///
    void process_client_core_capability_response(InStream &chunk, uint32_t flags);

    ///
    void process_client_device_list_announce_request(InStream &chunk, uint32_t flags);

    ///
    void send_server_core_capability_request();

    ///
    void send_server_client_id_confirm();

    ///
    void send_server_device_announce_response();

    ///
    void send_device_io_request(uint32_t io_control_code, bytes_view data);

    ///
    void process_device_control_response(InStream &chunk, uint32_t flags);
};