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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */


#pragma once

#include <cstddef>
#include <cstring>

namespace transbuf {

    template<class Buf, std::size_t Size = (1024*4), std::size_t TempoMax = (Size/2)>
    class obuffering_buf
    : public Buf
    {
        static const std::size_t buffering_max_len = (Size > TempoMax ? TempoMax : Size);

        unsigned char buffer[Size];
        unsigned char * p;

        typedef long int long_int;

    public:
        obuffering_buf() noexcept
        : p(this->buffer)
        {}

        template<class T>
        explicit obuffering_buf(const T & params)
        : Buf(params)
        , p(this->buffer)
        {}

        ~obuffering_buf()
        {
            const std::size_t sz = this->p - this->buffer;
            if (sz) {
                Buf::write(this->buffer, sz);
            }
        }

        int close()
        {
            const std::size_t sz = this->p - this->buffer;
            long int res = sz ? Buf::write(this->buffer, sz) : 0;
            this->p = this->buffer;
            int rclose = Buf::close();
            return rclose ? rclose : (res < 0 ? res : (res != long_int(sz) ? -1 : 0));
        }

        long int write(const void * buffer, size_t len)
        {
            const std::size_t sz = this->p - this->buffer;
            if (sz + len > Size || len >= buffering_max_len) {
                if (sz) {
                    long int res = Buf::write(this->buffer, sz);
                    if (res != long_int(sz)) {
                        if (res > 0) {
                            this->p += res;
                        }
                        return res;
                    }
                }

                this->p = this->buffer;
                if (len >= buffering_max_len) {
                    return Buf::write(buffer, len);
                }
            }

            memcpy(this->p, buffer, len);
            this->p += len;

            return len;
        }

        int flush()
        {
            const long int sz = this->p - this->buffer;
            if (sz) {
                long int res = Buf::write(this->buffer, sz);
                if (sz != res) {
                    if (res < 0) {
                        return res;
                    }
                    this->p += res;
                    return -1;
                }
                this->p = this->buffer;
            }
            return 0;
        }
    };
}

