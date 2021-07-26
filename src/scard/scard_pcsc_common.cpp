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
    Copyright (C) Wallix 2020
    Author(s): 
*/

#include <sys/socket.h>
#include <string.h>

#include <stdexcept>

#include "scard/scard_pcsc_common.hpp"


///////////////////////////////////////////////////////////////////////////////


const char * get_socket_error_message(int fd)
{
    int error = 0;
    socklen_t error_length;
    
    error_length = sizeof(error);
    auto ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &error_length);
    if (ret)
    {
        throw std::runtime_error("Failed to get socket error message.");
    }

    return strerror(error);
}