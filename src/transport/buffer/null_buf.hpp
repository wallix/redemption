/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_TRANSPORT_BUFFER_NULL_BUF_HPP
#define REDEMPTION_TRANSPORT_BUFFER_NULL_BUF_HPP

#include <cstddef>


namespace transbuf
{
    struct null_buf
    {
        int close() noexcept { return 0; }

        long int write(const void *, size_t len) noexcept { return len; }

        long int read(void *, size_t len) noexcept { return len; }

        int flush() const noexcept { return 0; }
    };
}

#endif
