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

#include "utils/asynchronous_task_manager.hpp"
#include "utils/virtual_channel_data_sender.hpp"
#include "mod/rdp/rdp_log.hpp"
#include "core/report_message_api.hpp"

typedef int_fast32_t data_size_type;

class BaseVirtualChannel
{
    VirtualChannelDataSender* to_client_sender;
    VirtualChannelDataSender* to_server_sender;

protected:
    ReportMessageApi & report_message;
    const RDPVerbose verbose;

private:
    const data_size_type exchanged_data_limit;
          data_size_type exchanged_data                        = 0;
          bool           exchanged_data_limit_reached_reported = false;

public:
    struct Params
    {
        ReportMessageApi & report_message;
        data_size_type  exchanged_data_limit;
        RDPVerbose verbose;

        Params(ReportMessageApi & report_message) : report_message(report_message) {}
    };

protected:
    BaseVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        const Params & params)
    : to_client_sender(to_client_sender_)
    , to_server_sender(to_server_sender_)
    , report_message(params.report_message)
    , verbose(params.verbose)
    , exchanged_data_limit(params.exchanged_data_limit)
    {}

public:
    virtual ~BaseVirtualChannel() = default;

    void disable_to_client_sender() {
        this->to_client_sender = nullptr;
    }

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
        uint32_t chunk_data_length
    ) {
        (void)total_length;
        (void)flags;
        (void)chunk_data;
        (void)chunk_data_length;
    }

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

public:
    inline void send_message_to_server(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length)
    {
        if (this->to_server_sender)
        {
            (*this->to_server_sender)(total_length, flags, chunk_data,
                chunk_data_length);
                LOG(LOG_INFO,  "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        }
    }

protected:
    inline void update_exchanged_data(uint32_t data_length)
    {
        this->exchanged_data += data_length;

        if (this->exchanged_data_limit
        && !this->exchanged_data_limit_reached_reported
        && (this->exchanged_data > this->exchanged_data_limit))
        {
            this->report_message.report(
                this->get_reporting_reason_exchanged_data_limit_reached(),
                "");

            this->exchanged_data_limit_reached_reported = true;
        }
    }
};  // class BaseVirtualChannel

