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

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

#include <cstdint>
#include <vector>

#include "utils/sugar/unique_fd.hpp"
#include "utils/stream.hpp"


///////////////////////////////////////////////////////////////////////////////



class scard_pcsc_client
{
public:
    ///
    scard_pcsc_client(const char *socket_file_path,
        std::size_t buffer_size_hint);

    ///
    std::size_t send(uint32_t io_control_code, bytes_view data);

    ///
    bytes_view receive();

private:
    ///
    void connect();
    
    ///
    unique_fd _socket;

    ///
    struct sockaddr_un _socket_address;

    ///
    std::vector<uint8_t> _buffer;
};