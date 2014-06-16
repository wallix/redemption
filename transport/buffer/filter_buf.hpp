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

#ifndef REDEMPTION_PUBLIC_TRANSPORT_BUFFER_FILTER_BUF_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_BUFFER_FILTER_BUF_HPP

#include <unistd.h>

namespace transbuf {

template<class Buf, class Filter>
class filter_buf
{
    Buf buf;
    Filter filter;

public:
    template<class BufParams, class FilterParams>
    filter_buf(const BufParams & buf_params, const FilterParams & filter_params)
    : buf(buf_params)
    , filter(filter_params)
    {}

    ~filter_buf() /*noexcept*/
    {
        if (this->filter.ready()) {
            this->filter.close(this->buf);
        }
    }

    int close() /*noexcept*/
    {
        const int res1 = this->filter.close(this->buf);
        const int res2 = this->buf.close();
        return res1 < 0 ? res1 : res2;
    }

    ssize_t write(const void * data, size_t len) /*noexcept*/
    { return this->filter.write(this->buf, data, len); }

    ssize_t read(void * data, size_t len) /*noexcept*/
    { return this->filter.read(this->buf, data, len); }

    bool ready() /*noexcept*/
    { return this->filter.ready(); }
};

}

#endif
