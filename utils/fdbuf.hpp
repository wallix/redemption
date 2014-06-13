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

#ifndef REDEMPTION_PUBLIC_UTILS_FDBUF_HPP
#define REDEMPTION_PUBLIC_UTILS_FDBUF_HPP

#include "read_and_write.hpp"

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
    fdbuf(int fd = -1) /*noexcept*/
    : fd(fd)
    {}

    //CPP_MOVE_CTOR(fdbuf) /*noexcept*/
    //: fd(CPP_MOVE_CTOR_ARG().fd)
    //{
    //  CPP_MOVE_CTOR_ARG().fd = -1;
    //}

    ~fdbuf() /*noexcept*/
    {
        this->close();
    }

    int get_fd() const
    {
       return this->fd;
    }

    int open(const char *pathname, int flags) /*noexcept*/
    {
        this->close();
        this->fd = ::open(pathname, flags);
        return fd;
    }

    int open(const char *pathname, int flags, mode_t mode) /*noexcept*/
    {
        this->close();
        this->fd = ::open(pathname, flags, mode);
        return fd;
    }

    int open(int fd) /*noexcept*/
    {
        this->close();
        this->fd = fd;
        return fd;
    }

    int close() /*noexcept*/
    {
        if (this->is_open()) {
            const int ret = ::close(this->fd);
            this->fd = -1;
            return ret;
        }
        return 0;
    }

    bool is_open() const /*noexcept*/
    {
        return -1 != this->fd;
    }

    ssize_t read(void * data, size_t len) const /*noexcept*/
    {
        return read_all(this->fd, data, len);
    }

    ssize_t write(const void * data, size_t len) const /*noexcept*/
    {
        return write_all(this->fd, data, len);
    }

    off_t seek(off_t offset, int whence) const /*noexcept*/
    { return lseek(this->fd, offset, whence); }

    void swap(fdbuf & other) /*noexcept*/
    {
        this->close();
        this->fd = other.fd;
        other.fd = -1;
    }

private:
    fdbuf(fdbuf const &) /*= delete */;
    fdbuf&operator=(fdbuf const &) /*= delete */;
};

void swap(fdbuf & a, fdbuf & b) /*noexcept*/
{ a.swap(b); }

} //posix
} //io

#endif
