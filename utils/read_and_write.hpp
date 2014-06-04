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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tans
 */

#ifndef REDEMPTION_PUBLIC_UTILS_READ_AND_WRITE_HPP
#define REDEMPTION_PUBLIC_UTILS_READ_AND_WRITE_HPP

#include <cerrno>
#include <cstddef>
#include <unistd.h>

namespace io {
namespace posix {

using std::size_t;

inline ssize_t read_all(int fd, char * data, size_t len) /*noexcept*/
{
    ssize_t ret = 0;
    size_t remaining_len = len;
    while (remaining_len) {
        ret = ::read(fd, data + (len - remaining_len), remaining_len);
        if (ret < 0){
            if (errno == EINTR){
                continue;
            }
            // Error should still be there next time we try to read
            if (remaining_len != len){
                return len - remaining_len;
            }
            return -1;
        }
        // We must exit loop or we will enter infinite loop
        if (ret == 0){
            break;
        }
        remaining_len -= ret;
    }
    return len - remaining_len;
}

inline ssize_t write_all(int fd, const char * data, size_t len) /*noexcept*/
{
    ssize_t ret = 0;
    size_t remaining_len = len;
    size_t total_sent = 0;
    while (remaining_len) {
        ret = ::write(fd, data + total_sent, remaining_len);
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

} //posix
} //io


#endif
