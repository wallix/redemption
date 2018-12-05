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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <iterator>
#include <type_traits>

using std::begin; /*NOLINT*/
using std::end; /*NOLINT*/

template<class It>
struct range
{
    It first_;
    It last_;

    using value_type = typename std::iterator_traits<It>::value_type;
    using reference = typename std::conditional<
        std::is_const<decltype(*std::declval<It>())>::value,
        value_type,
        value_type const
    >::type &;
    using const_reference = value_type const &;
    using iterator = It;
    using const_iterator = It;

    std::size_t size() const { return this->last_ - this->first_; }

    bool empty() const { return this->last_ == this->first_; }

    const_reference front() const { return *(this->first_); }
    reference       front()       { return *(this->first_); }

    const_reference back() const  { return *(this->last_-1); }
    reference       back()        { return *(this->last_-1); }

    const_reference operator[](std::size_t i) const { return this->first_[i]; }
    reference       operator[](std::size_t i)       { return this->first_[i]; }

    friend bool operator == (range const & a, range const & b) {
        return a.first_ == b.first_ && a.last_ == b.last_;
    }

    friend bool operator != (range const & a, range const & b) {
        return !(a == b);
    }

    It begin() const { return this->first_; }
    It end()   const { return this->last_; }
};

template<class Cont>
auto make_range(Cont & cont) -> range<decltype(begin(cont))> {
    return {begin(cont), end(cont)};
}

template<class It>
range<It> make_range(It first, It last) {
    return {first, last};
}

