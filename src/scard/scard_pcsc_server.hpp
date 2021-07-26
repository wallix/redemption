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

#include <atomic>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>

#include "utils/sugar/unique_fd.hpp"
#include "scard/scard_pcsc_common.hpp"


///////////////////////////////////////////////////////////////////////////////


///
class scard_pcsc_server_handler
{
public:
    ///
    virtual ~scard_pcsc_server_handler() = default;
    
    ///
    virtual void handle_pcsc_server_start() = 0;

    ///
    virtual void handle_pcsc_client_connection(int socket) = 0;

    ///
    virtual void handle_pcsc_server_data(scard_wrapped_call wrapped_call) = 0;

    ///
    virtual void handle_pcsc_client_disconnection(int socket) = 0;

    ///
    virtual void handle_pcsc_server_stop() = 0;
};

///
class scard_pcsc_server
{
public:
    ///
    scard_pcsc_server(const char *socket_path,
        scard_pcsc_server_handler *handler_ptr = nullptr);

    ///
    ~scard_pcsc_server();

    ///
    void disconnect();
    
    ///
    void serve();

    ///
    void post(scard_wrapped_return wrapped_return);

private:
    ///
    scard_wrapped_call receive(int fd, uint8_t *buffer, std::size_t buffer_size);

    ///
    void send(int fd, scard_wrapped_return &&wrapped_return);

    ///
    std::string _socket_path;

    ///
    unique_fd _socket;

    ///
    scard_pcsc_server_handler *_handler_ptr;

    ///
    std::atomic<bool> _state;

    ///
    std::deque<scard_wrapped_return> _queue;

    ///
    std::mutex _queue_mutex;
};