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

#include "core/error.hpp"
#include "transport/in_file_transport.hpp"

#include <cerrno>


void InFileTransport::seek(int64_t offset, int whence)
{
    if (lseek64(this->file.fd(), offset, whence) == static_cast<off_t>(-1)) {
        throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
    }
}

Transport::Read InFileTransport::do_atomic_read(uint8_t * buffer, size_t len)
{
    size_t remaining_len = len;
    while (remaining_len) {
        ssize_t const res = ::read(this->file.fd(), buffer + (len - remaining_len), remaining_len);
        if (res <= 0){
            if (res == 0 && remaining_len == len){
                return Read::Eof;
            }
            if (res != 0 && errno == EINTR){
                continue;
            }
            throw Error(ERR_TRANSPORT_READ_FAILED, res);
        }
        remaining_len -= res;
    }
    if (remaining_len != 0){
        throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
    }
    return Read::Ok;
}

size_t InFileTransport::do_partial_read(uint8_t * buffer, size_t len)
{
    if (!len) {
        return 0;
    }

    ssize_t res;
    do {
        res = ::read(this->file.fd(), buffer, len);
    } while (res == 0 && errno == EINTR);

    if (res < 0) {
        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
    }

    return static_cast<size_t>(res);
}
