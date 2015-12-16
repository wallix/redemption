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

#ifndef REDEMPTION_MOD_RDP_CHANNELS_BASECHANNEL_HPP
#define REDEMPTION_MOD_RDP_CHANNELS_BASECHANNEL_HPP

#include "asynchronous_task_manager.hpp"
#include "virtual_channel_data_sender.hpp"

#define MODRDP_LOGLEVEL_CLIPRDR      0x04000000
#define MODRDP_LOGLEVEL_RDPDR        0x08000000

#define MODRDP_LOGLEVEL_CLIPRDR_DUMP 0x40000000
#define MODRDP_LOGLEVEL_RDPDR_DUMP   0x80000000

typedef int_fast32_t data_size_type;

class BaseVirtualChannel
{
    VirtualChannelDataSender* to_client_sender;
    VirtualChannelDataSender* to_server_sender;

          auth_api*      authentifier;
    const data_size_type exchanged_data_limit;
          data_size_type exchanged_data                        = 0;
          bool           exchanged_data_limit_reached_reported = false;

protected:
    const uint32_t verbose;

public:
    struct Params
    {
        auth_api*       authentifier;
        data_size_type  exchanged_data_limit;
        uint32_t        verbose;
    };

protected:
    BaseVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        const Params & params)
    : to_client_sender(to_client_sender_)
    , to_server_sender(to_server_sender_)
    , authentifier(params.authentifier)
    , exchanged_data_limit(params.exchanged_data_limit)
    , verbose(params.verbose) {}

public:
    virtual ~BaseVirtualChannel() = default;

    virtual const char * get_reporting_reason_exchanged_data_limit_reached()
        const = 0;

protected:
    inline bool has_valid_to_client_sender() const
    {
        return bool(this->to_client_sender);
    }

public:
    virtual void process_client_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length) = 0;

    virtual void process_server_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task) = 0;

protected:
    inline void send_message_to_client(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length)
    {
        if (this->to_client_sender)
        {
            (*this->to_client_sender)(total_length, flags, chunk_data,
                chunk_data_length);
        }
    }

    inline void send_message_to_server(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length)
    {
        if (this->to_server_sender)
        {
            (*this->to_server_sender)(total_length, flags, chunk_data,
                chunk_data_length);
        }
    }

    inline void update_exchanged_data(uint32_t data_length)
    {
        this->exchanged_data += data_length;

        if (this->exchanged_data_limit &&
            this->authentifier &&
            !this->exchanged_data_limit_reached_reported &&
            (this->exchanged_data > this->exchanged_data_limit))
        {
            this->authentifier->report(
                this->get_reporting_reason_exchanged_data_limit_reached(),
                "");

            this->exchanged_data_limit_reached_reported = true;
        }
    }
};  // class BaseVirtualChannel

#endif  // #ifndef REDEMPTION_MOD_RDP_CHANNELS_BASECHANNEL_HPP
