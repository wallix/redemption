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

#ifndef REDEMPTION_UTILS_DELETE_HPP
#define REDEMPTION_UTILS_DELETE_HPP

#include <iterator>

template<typename T>
struct default_deleter {
    void operator()(T * val) const {
        delete val;
    }
};

template<typename T>
class default_pointer_deleter;

template<typename T>
struct default_pointer_deleter<T*>
: default_deleter<T>
{};

template<typename ForwardIterator>
inline void delete_all(ForwardIterator first, ForwardIterator last)
{
    typedef typename std::iterator_traits<ForwardIterator>::value_type pointer_type;
    default_pointer_deleter<pointer_type> deleter;
    for (; first != last; ++first) {
        deleter(*first);
    }
}

#endif