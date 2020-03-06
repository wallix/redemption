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
Copyright (C) Wallix 2010-2020
Author(s): Jonathan Poelen
*/

#pragma once

#include <algorithm>
#include <vector>
#include <cassert>


template<class T>
std::vector<T>& unordered_erase_element(std::vector<T>& v, T* p)
{
    assert(v.data() <= p && p <= v.data() + v.size());
    if (p != &v.back()) {
        v[size_t(p - &v.front())] = std::move(v.back());
    }
    v.pop_back();

    return v;
}

template<class T, class F>
std::vector<T>& unordered_erase_if(std::vector<T>& v, F&& pred)
{
    auto first = v.begin();
    auto last = v.end();

    for (; first != last; ++first) {
        if (pred(*first)) {
            while(1) {
                if (first == --last) {
                    goto erase;
                }

                if (not pred(*last)) {
                    *first = std::move(*last);
                    break;
                }
            }
        }
    }

    erase:
    v.erase(last, v.end());

    return v;
}
