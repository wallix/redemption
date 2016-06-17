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

#ifndef REDEMPTION_TRANSPORT_BUFFER_FILE_BUF_HPP
#define REDEMPTION_TRANSPORT_BUFFER_FILE_BUF_HPP

#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils/log.hpp"
#include "utils/sugar/exchange.hpp"


namespace transbuf {
    class ofile_buf
    {
        int fd;
    public:
        ofile_buf() : fd(-1) {}
        ~ofile_buf()
        {
            this->close();
        }

        int open(const char * filename, mode_t mode)
        {
            this->close();
            this->fd = ::open(filename, O_WRONLY | O_CREAT, mode);
            return this->fd;
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

        ssize_t write(const void * data, size_t len)
        {
            size_t remaining_len = len;
            size_t total_sent = 0;
            while (remaining_len) {
                ssize_t ret = ::write(this->fd,
                    static_cast<const char*>(data) + total_sent, remaining_len);
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

        bool is_open() const noexcept
        { return -1 != this->fd; }

        off64_t seek(off64_t offset, int whence) const
        { return ::lseek64(this->fd, offset, whence); }

        int flush() const
        { return 0; }
    };


}

#endif
