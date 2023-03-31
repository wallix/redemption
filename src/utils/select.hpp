/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include <sys/select.h>
#include "cxx/diagnostic.hpp"
#include "cxx/compiler_version.hpp"


REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
#if REDEMPTION_COMP_VERSION_NUMBER(13, 0, 0) <= REDEMPTION_COMP_CLANG_VERSION
  REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wreserved-identifier")
#endif

inline void io_fd_zero(fd_set & fds)
{
    FD_ZERO(&fds); /*NOLINT*/
}

inline void io_fd_set(int const fd, fd_set & fds)
{
    FD_SET(fd, &fds); /*NOLINT*/
}

inline void io_fd_clr(int const fd, fd_set & fds)
{
    FD_CLR(fd, &fds);
}

inline int io_fd_isset(int const fd, fd_set const & fds)
{
    return FD_ISSET(fd, &fds);
}

REDEMPTION_DIAGNOSTIC_POP()
