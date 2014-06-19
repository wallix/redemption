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

#ifndef REDEMPTION_TRANSPORT_BUFFER_DYNARRAY_BUF_HPP
#define REDEMPTION_TRANSPORT_BUFFER_DYNARRAY_BUF_HPP

#include "unique_ptr.hpp"
#include <cstring>

namespace transbuf {

    class dynarray_base
    {
        unique_ptr<uint8_t[]> data;
        std::size_t len;
        std::size_t current;

    public:
        dynarray_base() /*noexcept*/
        : len(0)
        , current(0)
        {}

        int open(size_t len, const char * data = 0) /*noexcept*/
        {
            this->data.reset(new(std::nothrow) uint8_t[len]);
            if (!this->data) {
                return -1;
            }
            if (data) {
                memcpy(this->data.get(), data, len);
            }
            this->len = len;
            this->current = 0;
            return 0;
        }

        int close() /*noexcept*/
        {
            this->data.reset();
            this->current = 0;
            this->len = 0;
            return 0;
        }

        int read(void * buffer, size_t len) /*noexcept*/
        { return this->copy(buffer, this->data.get() + this->current, len); }

        int write(const void * buffer, size_t len) /*noexcept*/
        { return this->copy(this->data.get() + this->current, buffer, len); }

        bool is_open() const /*noexcept*/
        { return this->data.get(); }

    private:
        int copy(void * dest, const void * src, size_t len)  /*noexcept*/
        {
            const size_t rlen = std::min<size_t>(this->len - this->current, len);
            memcpy(dest, src, rlen);
            this->current += rlen;
            return rlen;
        }
    };

}

#endif
