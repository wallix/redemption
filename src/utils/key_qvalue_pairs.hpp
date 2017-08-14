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

#pragma once

#include "utils/sugar/array_view.hpp"

#include <cstring>
#include <numeric>
#include <utility>

//typedef std::tuple<const char * const, const char * const> const kv_pair;
class charp_or_string {
public:
    const array_view_const_char data;
    charp_or_string(const char * const data) 
        : data(data, strlen(data)){}
    charp_or_string(const array_view_const_char data) 
        : data(data) {}
};

class kv_pair_ {
public:
    array_view_const_char key;
    array_view_const_char value;
    template<class T, class U> kv_pair_(T const & key, U const & value)
        : key{charp_or_string(key).data}
        , value{charp_or_string(value).data}
    {
    }
};

typedef const kv_pair_ kv_pair;
//using kv_pair = const kv_pair_;

// Precondition: input array view should never be empty
// internal pointers should never be nullptr
inline std::string key_qvalue_pairs(array_view<kv_pair> pairs)
{
    std::string buffer;

    // Ensure string is large enough for our use, to avoid useless internal resize
    size_t maj = std::accumulate(pairs.begin(), pairs.end(), size_t{0}, 
        [](size_t acc, kv_pair p){return acc + p.key.size()+p.value.size()+8;});
    // reserve some space for 8 quoted chars inside value
    // if there is more string is on it's own and will spend slightly more time
    buffer.reserve(maj+8);
    
    for (auto p: pairs){
        buffer.append(p.key.data(), p.key.size());
        buffer += "=\"" ;
        for (auto c : p.value){
            if (c == '\\' || c == '\"') {
                buffer += '\\';
            }
            buffer += c;
        }
        buffer += "\" ";
    }
    buffer.pop_back();
    return buffer;
}

inline std::string key_qvalue_pairs(std::initializer_list<kv_pair> pairs)
{
    return key_qvalue_pairs(array_view<kv_pair>{pairs.begin(), pairs.end()}); 
}

