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

#ifndef REDEMPTION_PUBLIC_TRANSPORT_BUFFER_FILENAME_BUF_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_BUFFER_FILENAME_BUF_HPP

#include "fdbuf.hpp"
#include "error.hpp"

namespace transbuf {
    struct in_filename_buf
    : private io::posix::fdbuf
    {
        in_filename_buf(const char * filename)
        {
            if (this->open(filename, O_RDONLY) < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
        }

        using io::posix::fdbuf::read;
        using io::posix::fdbuf::close;
        using io::posix::fdbuf::is_open;
    };

    struct out_filename_buf
    : private io::posix::fdbuf
    {
        out_filename_buf(const char * filename)
        {
            if (this->open(filename, O_WRONLY|O_CREAT, S_IRUSR) < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
        }

        using io::posix::fdbuf::write;
        using io::posix::fdbuf::close;
        using io::posix::fdbuf::is_open;
    };
}

#endif
