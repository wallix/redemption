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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_PUBLIC_TRANSPORT_BUFFER_RWFD_BUF_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_BUFFER_RWFD_BUF_HPP

#include "read_and_write.hpp"

namespace transbuf
{
    class rwfd_buf
    {
        int fd_;

    public:
        rwfd_buf(int fd = -1) /*noexcept*/
        : fd_(fd)
        {}

        int open(int fd) /*noexcept*/
        { return this->fd_ = fd; }

        int init() /*noexcept*/
        { return 0; }

        int close() /*noexcept*/
        { return 0; }

        ssize_t read(void * data, size_t len) /*noexcept*/
        { return io::posix::read_all(this->fd_, data, len); }

        ssize_t write(const void * data, size_t len) /*noexcept*/
        { return io::posix::write_all(this->fd_, data, len); }

        bool is_open() const /*noexcept*/
        { return this->fd_ != -1; }
    };
}

#endif
