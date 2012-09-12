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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(__UTILS_RELATIVE_TIME_POINT_HPP__)
#define __UTILS_RELATIVE_TIME_POINT_HPP__

#include "time_point.hpp"

struct relative_time_point
{
    char symbol;
    time_point point;

    relative_time_point()
    : symbol(0)
    , point()
    {}

    relative_time_point(const std::string& s)
    : symbol(0)
    {
        bool is_minus = false;
        std::size_t pos = 0;
        for (; pos != s.size(); ++pos)
        {
            if ('-' == s[pos])
                is_minus = !is_minus;
            else if ('+' != s[pos])
                break;
        }
        if (pos)
        {
            this->symbol = is_minus ? '-' : '+';
            this->point.parse(s.substr(pos));
        }
        else
            this->point.parse(s);
    }
};

inline bool operator==(const relative_time_point& a,
                       const relative_time_point& b)
{
    return a.point == b.point && a.symbol == b.symbol;
}

inline bool operator!=(const relative_time_point& a,
                       const relative_time_point& b)
{
    return !(a == b);
}

struct relative_time_point_less_only_point
{
    bool operator()(const relative_time_point& a,
                    const relative_time_point& b)
    {
        return a.point < b.point;
    }
};

template<typename _CharT, typename _TypeTraits>
inline std::basic_ostream<_CharT, _TypeTraits>&
operator<<(std::basic_ostream<_CharT, _TypeTraits>& os,
           const relative_time_point& rtime)
{
    if (rtime.symbol)
        os << rtime.symbol;
    return os << rtime.point;
}

#endif