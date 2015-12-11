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

#ifndef REDEMPTION_UTILS_ARRAY_VIEW_HPP
#define REDEMPTION_UTILS_ARRAY_VIEW_HPP

#include <cstddef>

template<class T>
struct array_view
{
    array_view() = default;

    array_view(T * p, std::size_t sz)
    : p(p)
    , sz(sz)
    {}

    template<std::size_t N>
    array_view(T (&a)[N])
    : p(a)
    , sz(N)
    {}

    explicit operator bool () const noexcept { return this->p; }

    std::size_t size() const noexcept { return this->sz; }

    T * data() noexcept { return this->p; }
    T const * data() const noexcept { return this->p; }

    T * begin() { return this->p; }
    T * end() { return this->p + this->sz; }
    T const * begin() const { return this->p; }
    T const * end() const { return this->p + this->sz; }

private:
    T * p;
    std::size_t sz;
};

#endif
