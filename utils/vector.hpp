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
#ifndef REDEMPTION_UTILS_VECTOR_HPP
#define REDEMPTION_UTILS_VECTOR_HPP
#include "log.hpp"

template<class T>
class Vector {
    size_t item_count;
    T * items;

public:
    Vector(size_t count)
    : item_count(count)
    , items(new T[count])
    {
        // LOG(LOG_INFO,"###### Vector Constructor ######");
    }
    Vector(const Vector & vect)
    : item_count(vect.size())
    , items(new T[vect.size()])
    {
        // LOG(LOG_INFO,"###### Vector Copy Constructor ######");
        for (size_t i = 0; i < vect.size(); i++) {
            this->items[i] = vect[i];
        }
    }
    virtual ~Vector() {
        // LOG(LOG_INFO,"###### Vector Destructor ######");
        delete [] this->items;
        this->items = NULL;
    }

    T & operator[](size_t i) const {
        REDASSERT(i < this->item_count);
        return this->items[i];
    }

    size_t size() const {
        return this->item_count;
    }
};

#endif
