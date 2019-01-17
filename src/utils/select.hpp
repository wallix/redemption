/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   Network related utility functions

*/

#pragma once

#include <algorithm>

#include <sys/select.h>
#include "cxx/diagnostic.hpp"

#include "utils/invalid_socket.hpp"

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
inline void io_fd_zero(fd_set & rfds)
{
    FD_ZERO(&rfds);
}

inline void io_fd_set(int const fd, fd_set & rfds)
{
    FD_SET(fd, &rfds);
}

inline void io_fd_clr(int const fd, fd_set & rfds)
{
    FD_CLR(fd, &rfds);
}

inline int io_fd_isset(int const fd, fd_set const & rfds)
{
    return FD_ISSET(fd, &rfds);
}
REDEMPTION_DIAGNOSTIC_POP

inline unsigned prepare_fds(int const fd, unsigned max, fd_set & rfds)
{
    if (fd > INVALID_SOCKET) {
        io_fd_set(fd, rfds);
        max = std::max(static_cast<unsigned>(fd), max);
    }
    return max;
}
