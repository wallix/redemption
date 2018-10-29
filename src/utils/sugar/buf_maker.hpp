/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#pragma once

#include <memory>
#include "utils/sugar/array_view.hpp"


template<std::size_t StaticLen, class T = uint8_t>
struct BufMaker
{
    array_view<T> dyn_array(std::size_t n) &
    {
        T * p = this->buf_;
        if (n > StaticLen) {
            this->dyn_buf_ = std::make_unique<T[]>(n);
            p = this->dyn_buf_.get();
        }
        return {p, n};
    }

    array_view<T> static_array() & noexcept
    {
        return this->buf_;
    }

    static constexpr std::size_t size() noexcept
    {
        return StaticLen;
    }

private:
    T buf_[StaticLen];
    std::unique_ptr<T[]> dyn_buf_;
};
