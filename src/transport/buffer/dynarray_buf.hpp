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


#pragma once

#include <cstring>
#include <memory>

namespace transbuf {

    class dynarray_buf
    {
        std::unique_ptr<uint8_t[]> data;
        std::size_t len = 0;
        std::size_t current = 0;

    public:
        dynarray_buf() = default;

        int open(size_t len, const void * data = nullptr)
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

        int close() noexcept
        {
            this->data.reset();
            this->current = 0;
            this->len = 0;
            return 0;
        }

        long int read(void * buffer, size_t len)
        { return this->copy(buffer, this->data.get() + this->current, len); }

        long int write(const void * buffer, size_t len)
        { return this->copy(this->data.get() + this->current, buffer, len); }

        bool is_open() const noexcept
        { return this->data.get(); }

        int flush() const noexcept
        { return 0; }

    private:
        long int copy(void * dest, const void * src, size_t len)
        {
            const size_t rlen = std::min<size_t>(this->len - this->current, len);
            memcpy(dest, src, rlen);
            this->current += rlen;
            return rlen;
        }
    };

}

