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
#ifndef REDEMPTION_UTILS_DARRAY_HPP
#define REDEMPTION_UTILS_DARRAY_HPP

#include "log.hpp"
#include "unique_ptr.hpp"

#include <algorithm>

template<class T>
class DArray {
    typedef std::size_t size_t;

    size_t item_count;
    unique_ptr<T[]> items;

public:
    DArray(size_t count)
    : item_count(count)
    , items(new T[count])
    {
        // LOG(LOG_INFO,"###### DArray Constructor ######");
    }

    DArray(const DArray & vect)
    : item_count(vect.size())
    , items(new T[vect.size()])
    {
        // LOG(LOG_INFO,"###### DArray Copy Constructor ######");
        std::copy(vect.items.get(), vect.items.get() + this->size(), this->items.get());
    }

    const T & operator[](size_t i) const {
        REDASSERT(i < this->item_count);
        return this->items.get()[i];
    }

    T & operator[](size_t i) {
        REDASSERT(i < this->item_count);
        return this->items.get()[i];
    }

    size_t size() const {
        return this->item_count;
    }
};

#endif
