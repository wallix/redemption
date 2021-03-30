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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include <cstdint>
#include <cassert>

namespace detail
{
    template<class UInt>
    struct BitsetStreamIterator
    {
        BitsetStreamIterator(UInt *p) noexcept
        : p(p)
        {}

        void next() noexcept
        {
            ++d;
            if (d == int(sizeof(UInt) * 8)) {
                d = 0;
                ++p;
            }
        }

        UInt flag(UInt x = 1) const
        {
            return UInt(x) << d;
        }

        UInt * p;
        int d = 0;
    };

    template class BitsetStreamIterator<unsigned long long>;
    template class BitsetStreamIterator<unsigned long long const>;
}

struct BitsetOutStream
{
    using underlying_type = unsigned long long;

    explicit BitsetOutStream(underlying_type * p = nullptr) noexcept
    : base_{p}
    {}

    void write(bool x) noexcept
    {
        *base_.p = (*base_.p & ~base_.flag(1)) | base_.flag(x);
        base_.next();
    }

    underlying_type * current() const noexcept
    {
        return base_.p;
    }

    bool is_partial() const noexcept
    {
        return base_.d;
    }

private:
    detail::BitsetStreamIterator<underlying_type> base_;
};

struct BitsetInStream
{
    using underlying_type = unsigned long long;

    explicit BitsetInStream(underlying_type const * p = nullptr) noexcept
    : base_{p}
    {}

    bool read() noexcept
    {
        bool r = *base_.p & base_.flag(1);
        base_.next();
        return r;
    }

    underlying_type const * current() const noexcept
    {
        return base_.p;
    }

    bool is_partial() const noexcept
    {
        return base_.d;
    }

private:
    detail::BitsetStreamIterator<const underlying_type> base_;
};
