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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_TRANSPORT_BUFFER_SUM_BUF_HPP
#define REDEMPTION_TRANSPORT_BUFFER_SUM_BUF_HPP

#include <cstddef>
#include <algorithm>

#include "iter.hpp"


namespace transbuf
{
    template<class Buf>
    struct osum_buf
    : public Buf
    {
        osum_buf() noexcept
        {}

        template<class T>
        osum_buf(const T & params)
        : Buf(params)
        {}

        template<class... Ts>
        int open(Ts && ... args)
        {
            this->sum_ = 0;
            return Buf::open(args...);
        }

        long int write(const void * buffer, size_t len)
        {
            auto ret = Buf::write(buffer, len);
            auto ubuf = static_cast<unsigned char const *>(buffer);
            for (auto c : iter(ubuf, len)) {
                this->sum_ += c;
            }
            return ret;
        }

        uint32_t sum() const noexcept {
            return this->sum_;
        }

    private:
        uint32_t sum_ = 0;
    };


    template<class BufParams = void>
    struct sum_and_mini_sum_buf_param
    {
        size_t mini_sum_limit;
        BufParams buf_params;
    };

    template<>
    struct sum_and_mini_sum_buf_param<void>
    {
        size_t mini_sum_limit;
    };

    template<class Buf>
    struct osum_and_mini_sum_buf
    : public Buf
    {
        template<class BufParams>
        osum_and_mini_sum_buf(const sum_and_mini_sum_buf_param<BufParams> & params)
        : Buf(params.buf_params)
        , mini_sum_limit_(params.mini_sum_limit)
        {}

        osum_and_mini_sum_buf(const sum_and_mini_sum_buf_param<void> & params)
        : Buf()
        , mini_sum_limit_(params.mini_sum_limit)
        {}

        template<class... Ts>
        int open(Ts && ... args)
        {
            this->mini_sum_ = 0;
            this->after_sum_ = 0;
            this->mini_sum_consumed_ = 0;
            return Buf::open(args...);
        }

        long int write(const void * buffer, size_t len)
        {
            auto ret = Buf::write(buffer, len);

            auto ubuf = static_cast<unsigned char const *>(buffer);

            auto min_len = std::min(this->mini_sum_limit_ - this->mini_sum_consumed_, len);
            auto new_len = len - min_len;
            this->mini_sum_consumed_ += min_len;

            for (auto c : iter(ubuf, min_len)) {
                this->mini_sum_ += c;
            }

            for (auto c : iter(ubuf + min_len, new_len)) {
                this->after_sum_ += c;
            }

            return ret;
        }

        uint32_t sum() const noexcept {
            return this->after_sum_ + this->mini_sum_;
        }

        uint32_t mini_sum() const noexcept {
            return this->mini_sum_;
        }

    private:
        uint32_t mini_sum_ = 0;
        uint32_t after_sum_ = 0;
        size_t mini_sum_consumed_ = 0;
        size_t mini_sum_limit_;
    };
}

#endif
