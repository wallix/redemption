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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>

#pragma once

/// \brief close a file descriptor automatically
struct local_fd
{
    local_fd(local_fd const &) = delete;
    local_fd & operator=(local_fd const &) = delete;

    explicit local_fd(int fd) noexcept : fd_(fd) {}

    local_fd(char const * pathname, int flags) noexcept
    : fd_(::open(pathname, flags))
    {}

    local_fd(char const * pathname, int flags, mode_t mode) noexcept
    : fd_(::open(pathname, flags, mode))
    {}

    local_fd(std::string const & pathname, int flags) noexcept
    : local_fd(pathname.c_str(), flags)
    {}

    local_fd(std::string const & pathname, int flags, mode_t mode) noexcept
    : local_fd(pathname.c_str(), flags, mode)
    {}

    ~local_fd() { if (this->is_open()) ::close(this->fd_); }

    explicit operator bool () const noexcept { return this->is_open(); }
    bool operator!() const noexcept { return !this->is_open(); }
    bool is_open() const noexcept { return this->fd_ >= 0; }

    int fd() const noexcept { return this->fd_; }

private:
    int fd_;
};
