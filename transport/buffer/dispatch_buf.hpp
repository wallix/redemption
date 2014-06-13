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

#ifndef REDEMPTION_PUBLIC_TRANSPORT_BUFFER_DISPATCH_BUF_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_BUFFER_DISPATCH_BUF_HPP

#include <unistd.h>

namespace transbuf {
    template<class Buf>
    struct dispatch_buf
    : protected Buf
    {
        Buf * rdbuf() const /*noexcept*/
        { return const_cast<Buf*>(static_cast<const Buf*>(this)); }

        bool buf_is_detachable() const /*noexcept*/
        { return false; }

        ssize_t read(void * data, size_t len) /*noexcept*/
        { return this->Buf::read(data, len); }

        ssize_t write(const void * data, size_t len) /*noexcept*/
        { return this->Buf::write(data, len); }

    protected:
        typedef dispatch_buf dispatch_type;
    };

    template<class Buf>
    class dispatch_buf<Buf*>
    {
        Buf * buf;

    public:
        ~dispatch_buf() /*noexcept*/
        { delete this->buf; }

        bool buf_is_detachable() const /*noexcept*/
        { return true; }

        ssize_t read(void * data, size_t len) /*noexcept*/
        { return this->buf->Buf::read(data, len); }

        ssize_t write(const void * data, size_t len) /*noexcept*/
        { return this->buf->Buf::write(data, len); }

        Buf * rdbuf(Buf * newbuf) /*noexcept*/
        {
            Buf * tmp = this->buf;
            this->buf = newbuf;
            return this->buf;
        }

        Buf * rdbuf() const /*noexcept*/
        { return this->buf; }

    protected:
        typedef dispatch_buf dispatch_type;
    };
}
#endif
