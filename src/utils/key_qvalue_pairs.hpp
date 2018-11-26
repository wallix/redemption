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

#include <string>

#include <cstring>


struct charp_or_string
{
    const array_view_const_char data;

    charp_or_string(const char * const data)
        : data(data, strlen(data)){}
    charp_or_string(const array_view_const_char data)
        : data(data) {}
    charp_or_string(const std::string & data)
        : data(data) {}
};

struct kv_pair_
{
    array_view_const_char key;
    array_view_const_char value;

    template<class T, class U> kv_pair_(T const & key, U const & value)
        : key{charp_or_string(key).data}
        , value{charp_or_string(value).data}
    {}
};

using kv_pair = const kv_pair_;


inline void escaped_key_qvalue(std::string & escaped_subject, array_view_const_char subject)
{
    struct EscapedTable
    {
        constexpr EscapedTable() noexcept
          : t{}
        {
            t[int('\\')] = '\\';
            t[int('"')] = '"';
            t[int('\n')] = 'n';
            t[int('\r')] = 'r';
        }

        char operator[](char c) const
        {
            // char -> uchar because char(128) must be negative
            using uchar = unsigned char;
            return this->t[unsigned(uchar(c))];
        }

    private:
        char t[256];
    };

    constexpr EscapedTable escaped_table;

    auto pred = [&](char c){
        return bool(escaped_table[c]);
    };

    auto first = subject.begin();
    auto last = subject.end();

    decltype(first) p;
    while ((p = std::find_if(first, last, pred)) != last) {
        escaped_subject.append(first, p);
        escaped_subject += '\\';
        escaped_subject += escaped_table[*p];
        first = p + 1;
    }

    escaped_subject.append(first, last);
}

// Precondition: input array view should never be empty
// internal pointers should never be nullptr
inline std::string key_qvalue_pairs(std::string & buffer, array_view<kv_pair> pairs)
{
    if (!buffer.empty()) {
        buffer += " ";
    }
    for (auto p: pairs){
        buffer.append(p.key.data(), p.key.size());
        buffer += "=\"";
        escaped_key_qvalue(buffer, p.value);
        buffer += "\" ";
    }
    buffer.pop_back();
    return buffer;
}

inline std::string key_qvalue_pairs(std::string & buffer, std::initializer_list<kv_pair> pairs)
{
    return key_qvalue_pairs(buffer, array_view<kv_pair>{pairs.begin(), pairs.end()});
}

inline std::string key_qvalue_pairs(array_view<kv_pair> pairs)
{
    std::string buffer;
    // Ensure string is large enough for our use, to avoid useless internal resize
    size_t maj = 0;
    for (auto const& p : pairs) {
        maj += p.key.size() + p.value.size() + 8;
    }
    // reserve some space for 8 quoted chars inside value
    // if there is more string is on it's own and will spend slightly more time
    buffer.reserve(maj+8);
    return key_qvalue_pairs(buffer, pairs);
}

inline std::string key_qvalue_pairs(std::initializer_list<kv_pair> pairs)
{
    return key_qvalue_pairs(array_view<kv_pair>{pairs.begin(), pairs.end()});
}


struct KeyQvalueFormatter
{
    array_view_const_char assign(charp_or_string type, array_view<kv_pair> pairs)
    {
        this->buf = "type=\"";
        this->buf.append(type.data.data(), type.data.size());
        this->buf += '\"';
        for (auto const& kv : pairs) {
            this->buf += ' ';
            this->buf.append(kv.key.data(), kv.key.size());
            this->buf += "=\"";
            escaped_key_qvalue(this->buf, kv.value);
            this->buf += '"';
        }
        return this->av();
    }

    array_view_const_char assign(charp_or_string type, std::initializer_list<kv_pair> pairs)
    {
        return this->assign(type, array_view<kv_pair>{pairs.begin(), pairs.end()});
    }

    array_view_const_char av() const noexcept
    {
        return this->buf;
    }

    void clear()
    {
        this->buf.clear();
    }

    std::string const & str() const noexcept
    {
        return this->buf;
    }

    explicit KeyQvalueFormatter()
    {
        this->buf.reserve(64);
    }

private:
    std::string buf;
};
