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

#include "fdbuf.hpp"

namespace transbuf {

    class ifile_buf
    {
        io::posix::fdbuf fdbuf;

    public:
        int open(const char * filename)
        { return this->fdbuf.open(filename, O_RDONLY); }

        int open(const char * filename, mode_t /*mode*/)
        { return this->fdbuf.open(filename, O_RDONLY); }

        int close()
        { return this->fdbuf.close(); }

        bool is_open() const noexcept
        { return this->fdbuf.is_open(); }

        ssize_t read(void * data, size_t len)
        { return this->fdbuf.read(data, len); }

        off64_t seek(off64_t offset, int whence) const
        { return this->fdbuf.seek(offset, whence); }
    };

    class ofile_buf
    {
        io::posix::fdbuf fdbuf;

    public:
        int open(const char * filename, mode_t mode)
        { return this->fdbuf.open(filename, O_WRONLY | O_CREAT, mode); }

        int close()
        { return this->fdbuf.close(); }

        bool is_open() const noexcept
        { return this->fdbuf.is_open(); }

        ssize_t write(const void * data, size_t len)
        { return this->fdbuf.write(data, len); }

        off64_t seek(off64_t offset, int whence) const
        { return this->fdbuf.seek(offset, whence); }

        int flush() const
        { return 0; }
    };
}

#endif
