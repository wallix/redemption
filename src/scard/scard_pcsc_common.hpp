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

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "utils/sugar/bytes_view.hpp"


///////////////////////////////////////////////////////////////////////////////


///
struct scard_wrapped_call
{
    uint32_t                io_control_code;
    std::vector<uint8_t>    data;

    ///
    scard_wrapped_call(uint32_t io_control_code, bytes_view data)
        :
        io_control_code(io_control_code),
        data(data.begin(), data.end())
    {
    }
};

///
struct scard_wrapped_return
{
    uint32_t                io_control_code;
    std::vector<uint8_t>    data;

    ///
    scard_wrapped_return()
        : io_control_code(0)
    {
    }

    ///
    scard_wrapped_return(uint32_t io_control_code, bytes_view data)
        :
        io_control_code(io_control_code),
        data(data.begin(), data.end())
    {
    }
};

///
class scard_pcsc_exception : public std::runtime_error
{
public:
    ///
    scard_pcsc_exception(const char *message)
        : std::runtime_error(message)
    {
    }
};

///
const char * get_socket_error_message(int fd);