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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/sugar/checked_iterator.hpp"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <memory>
#include <cassert>


template<class T, std::size_t Capacity>
struct limited_array
{
    static_assert(std::is_trivially_copyable<T>::value);

    using iterator = checked_iterator<T*>;
    using const_iterator = checked_iterator<T*>;
    using pointer = T*;
    using const_pointer = T const*;
    using size_type = std::size_t;

    template<class Iterator>
    limited_array(Iterator first, Iterator const& last)
    noexcept(noexcept((void(++first), T(*first))))
    {
        this->_check_distance(first, last);
        std::copy(first, last, array);
    }

    pointer data() noexcept { return this->array; }
    const_pointer data() const noexcept { return this->array; }
    size_type size() const noexcept { return this->len; }

    iterator begin() noexcept { return iterator(data(), size()); }
    iterator end() noexcept { return iterator(data() + size(), 0); }
    const_iterator begin() const noexcept { return const_iterator(data(), size()); }
    const_iterator end() const noexcept { return const_iterator(data() + size(), 0); }

    void uninit_resize(std::size_t n)
    {
        assert(n < Capacity);
        this->len = n;
    }

private:
    iterator _writable_iterator(const_iterator pos)
    {
        assert(pos > array);
        assert(pos < array + Capacity);
        return array + (pos - array);
    }

    template<class Iterator>
    void _check_distance(Iterator const& first, Iterator const& last)
    {
        if constexpr (!std::is_same_v<std::input_iterator_tag,
            typename std::iterator_traits<Iterator>::iterator_category>
        ) {
            assert(std::distance(first, last) <= Capacity);
        }
    }

    template<class Iterator>
    void _insert_init(const_iterator pos, Iterator first, Iterator const& last)
    {
        std::uninitialized_copy(first, last, writable_iterator(pos));
    }

    std::size_t len = 0; /* TODO select minimal sized type */
    T array[Capacity];
};
