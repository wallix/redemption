/*
* Copyright (C) 2016 Wallix
* 
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
* 
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
* 
* You should have received a copy of the GNU Lesser General Public License along
* with this library; if not, write to the Free Software Foundation, Inc., 59
* Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#pragma once

namespace ppocr
{
    template<class T>
    struct integer_iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using const_reference = value_type const &;
        using reference = const_reference;

        value_type n;
        integer_iterator & operator++() { ++n; return *this; }
        reference operator*() const { return n; }
        difference_type operator-(integer_iterator const & other) const { return this->n - other.n; }

        integer_iterator& operator+=(difference_type n) { this->n += n; return *this; }
        integer_iterator& operator-=(difference_type n) { this->n -= n; return *this; }

        integer_iterator operator+(difference_type n) const { return integer_iterator{T(this->n + n)}; }
        integer_iterator operator-(difference_type n) const { return integer_iterator{T(this->n - n)}; }

        bool operator==(integer_iterator const & other) const { return this->n == other.n; }
        bool operator!=(integer_iterator const & other) const { return this->n != other.n; }
        bool operator<(integer_iterator const & other) const { return this->n < other.n; }
        bool operator<=(integer_iterator const & other) const { return this->n < other.n; }
    };
}
