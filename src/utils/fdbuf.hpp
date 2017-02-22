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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan,
 *              Loïc Michaux
 */


#pragma once

#include "utils/sugar/exchange.hpp"

#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace io {
namespace posix {

class fdbuf
{
    int fd;

public:
    explicit fdbuf(int fd = -1) noexcept
    : fd(fd)
    {}

    fdbuf(fdbuf const &) = delete ;
    fdbuf&operator=(fdbuf const &) = delete ;

    fdbuf(fdbuf && other) noexcept
    : fd(other.fd)
    {
        other.fd = -1;
    }

    fdbuf& operator=(fdbuf && other) noexcept
    {
        this->fd = exchange(other.fd, -1);
        return *this;
    }

    ~fdbuf()
    {
        this->close();
    }

    //int get_fd() const
    //{
    //   return this->fd;
    //}

    int open(const char *pathname, int flags)
    {
        this->close();
        this->fd = ::open(pathname, flags);
        return fd;
    }

    int open(const char *pathname, int flags, mode_t mode)
    {
        this->close();
        this->fd = ::open(pathname, flags, mode);
        return fd;
    }

    int open(int fd)
    {
        this->close();
        this->fd = fd;
        return fd;
    }

    int close()
    {
        if (this->is_open()) {
            const int ret = ::close(this->fd);
            this->fd = -1;
            return ret;
        }
        return 0;
    }

    bool is_open() const noexcept
    {
        return -1 != this->fd;
    }

    explicit operator bool () const noexcept
    {
        return this->is_open();
    }

    ssize_t read(void * data, size_t len) const
    {
        return this->read_all(data, len);
    }

    ssize_t read_all(void * data, size_t len) const
    {
        size_t remaining_len = len;
        while (remaining_len) {
            ssize_t ret = ::read(this->fd, static_cast<char*>(data) + (len - remaining_len), remaining_len);
            if (ret < 0){
                if (errno == EINTR){
                    continue;
                }
                // Error should still be there next time we try to read
                if (remaining_len != len){
                    return len - remaining_len;
                }
                return ret;
            }
            // We must exit loop or we will enter infinite loop
            if (ret == 0){
                break;
            }
            remaining_len -= ret;
        }
        return len - remaining_len;
    }


    ssize_t write(const void * data, size_t len) const
    {
        return this->write_all(data, len);
    }

    ssize_t write_all(const void * data, size_t len) const
    {
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            ssize_t ret = ::write(this->fd, static_cast<const char*>(data) + total_sent, remaining_len);
            if (ret <= 0){
                if (errno == EINTR){
                    continue;
                }
                return -1;
            }
            remaining_len -= ret;
            total_sent += ret;
        }
        return total_sent;
    }

    off64_t seek(off64_t offset, int whence) const
    { return lseek64(this->fd, offset, whence); }

    void swap(fdbuf & other) noexcept
    {
        int const fd = this->fd;
        this->fd = other.fd;
        other.fd = fd;
    }

    int release() noexcept {
        int old_fd = this->fd;

        this->fd = -1;

        return old_fd;
    }
};

inline void swap(fdbuf & a, fdbuf & b) noexcept
{ a.swap(b); }

} //posix
} //io

