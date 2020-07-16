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

#include "mod/rdp/channels/virtual_channel_data_sender.hpp"
#include "mod/rdp/channels/rdpdr_asynchronous_task.hpp"
#include "utils/uninit_checked.hpp"
#include "acl/auth_api.hpp"
#include "mod/rdp/rdp_verbose.hpp"

#include <memory>

class BaseVirtualChannel
{
private:
    VirtualChannelDataSender* to_client_sender;
    VirtualChannelDataSender* to_server_sender;

protected:
    [[nodiscard]] VirtualChannelDataSender* to_client_sender_ptr() const noexcept
    {
        return this->to_client_sender;
    }

    [[nodiscard]] VirtualChannelDataSender* to_server_sender_ptr() const noexcept
    {
        return this->to_server_sender;
    }

    AuthApi & sesman;
    const RDPVerbose verbose;

public:
    // TODO: move that to BaseVirtualChannelConstructor
    struct Params
    {
        AuthApi & sesman;
        RDPVerbose verbose;

        explicit Params(AuthApi & sesman, RDPVerbose verbose)
          : sesman(sesman)
          , verbose(verbose)
        {}
    };

protected:
    BaseVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        const Params & params)
    : to_client_sender(to_client_sender_)
    , to_server_sender(to_server_sender_)
    , sesman(params.sesman)
    , verbose(params.verbose)
    {}

public:
    virtual ~BaseVirtualChannel() = default;

    void disable_to_client_sender() {
        this->to_client_sender = nullptr;
    }

protected:
    [[nodiscard]] inline bool has_valid_to_client_sender() const
    {
        return bool(this->to_client_sender);
    }

public:
    virtual void process_client_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data
    ) {
        (void)total_length;
        (void)flags;
        (void)chunk_data;
    }

    virtual void process_server_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task) = 0;

public:
    void send_message_to_client(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data)
    {
        if (this->to_client_sender)
        {
            (*this->to_client_sender)(total_length, flags, chunk_data);
        }
    }

    void send_message_to_server(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data)
    {
        if (this->to_server_sender)
        {
            (*this->to_server_sender)(total_length, flags, chunk_data);
        }
    }
};  // class BaseVirtualChannel

