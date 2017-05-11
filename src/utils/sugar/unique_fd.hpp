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

#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>

#include "utils/sugar/exchange.hpp"

/// \brief close a file descriptor automatically
struct unique_fd
{
    unique_fd(unique_fd const &) = delete;
    unique_fd & operator=(unique_fd const &) = delete;

    explicit unique_fd(int fd) noexcept : fd_(fd) {}

    unique_fd(unique_fd && other)
    : fd_(exchange(other.fd_, -1))
    {}

    unique_fd & operator=(unique_fd && other)
    {
        unique_fd tmp(std::move(other));
        tmp.swap(*this);
        return *this;
    }

    unique_fd(char const * pathname, int flags) noexcept
    : fd_(::open(pathname, flags))
    {}

    unique_fd(char const * pathname, int flags, mode_t mode) noexcept
    : fd_(::open(pathname, flags, mode))
    {}

    unique_fd(std::string const & pathname, int flags) noexcept
    : unique_fd(pathname.c_str(), flags)
    {}

    unique_fd(std::string const & pathname, int flags, mode_t mode) noexcept
    : unique_fd(pathname.c_str(), flags, mode)
    {}

    ~unique_fd()
    {
        this->reset();
    }

    explicit operator bool () const noexcept
    {
        return this->is_open();
    }

    bool operator!() const noexcept
    {
        return !this->is_open();
    }

    bool is_open() const noexcept
    {
        return this->fd_ >= 0;
    }

    int fd() const noexcept
    {
        return this->fd_;
    }

    int release() noexcept
    {
        return exchange(this->fd_, -1);
    }

    bool close()
    {
        if (this->is_open()) {
            int res = ::close(this->fd_);
            this->fd_ = -1;
            return res != -1;
        }
        return false;
    }

    void reset(int fd = -1) noexcept
    {
        if (this->is_open()) {
            ::close(this->fd_);
        }
        this->fd_ = fd;
    }

    void swap(unique_fd& other) noexcept
    {
        using std::swap;
        swap(fd_, other.fd_);
    }

private:
    int fd_;
};

inline void swap(unique_fd& a, unique_fd& b) noexcept
{
    a.swap(b);
}

inline unique_fd invalid_fd() noexcept
{
    return unique_fd{-1};
}
