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
 *   Author(s): Christophe Grosjean, Jonathan Poelen
 */

#if !defined(__RECORDER_RANGE_TIME_POINT_HPP__)
#define __RECORDER_RANGE_TIME_POINT_HPP__

#include <limits>
#include "time_point.hpp"

struct range_time_point
{
    time_point left;
    time_point right;

    range_time_point()
    : left()
    , right(std::numeric_limits<std::size_t>::max())
    {}

    range_time_point(const range_time_point& other)
    : left(other.left)
    , right(other.right)
    {}

    range_time_point(const std::string& s)
    : left()
    , right(std::numeric_limits<std::size_t>::max())
    {
        this->parse(s);
    }

    range_time_point(const time_point& l, const time_point& r)
    : left(l)
    , right(r)
    {}

    range_time_point(const std::size_t& l, const std::size_t& r)
    : left(l)
    , right(r)
    {}

    /*range_time_point& operator=(const range_time_point& other)
    {
        this->left = other.left;
        this->right = other.right;
        return *this;
    }*/

    /**
     * @example "+30m"
     * @example "1h,-20m"
     */
    void parse(const std::string& s)
    {
        std::string::size_type p = s.find_first_of(',');
        if (std::string::npos == p){
            if (s.empty()){
                this->right = std::numeric_limits<std::size_t>::max();
            } else {
                this->right.parse(s);
            }
            this->left = 0;
        } else if (s.length() == p+1) {
            this->left.parse(s.substr(0, p));
            this->right = std::numeric_limits<std::size_t>::max();
        } else {
            this->right.parse(s.substr((('+' == s[p+1] || '-' == s[p+1]) && s.length() != p+2) ? p+2 : p+1));
            if ('-' == s[0]){
                this->left.parse(s.substr(1, p-1));
            } else {
                this->left.parse(s.substr(0, p));
            }
            if ('-' == s[0]){
                if (this->left > this->right){
                    throw std::runtime_error("left time_point greater to right time_point");
                }
                this->left = this->right.time - this->left.time;
            }
            if ('-' == s[p+1]){
                if (this->left < this->right){
                    throw std::runtime_error("left time_point greater to right time_point");
                }
                this->right = this->left.time - this->right.time;
            }
            if ('+' == s[p+1]){
                this->right += this->left;
            }
        }
    }

    bool valid() const
    { return this->left.time <= this->right.time; }

    bool empty() const
    { return this->left.time == this->right.time; }
};

inline bool operator==(const range_time_point& r, const range_time_point& other)
{
    return r.left == other.left && r.right == other.right;
}
inline bool operator!=(const range_time_point& r, const range_time_point& other)
{
    return !(r == other);
}

template<typename _CharT, typename _TypeTraits>
inline std::basic_ostream<_CharT, _TypeTraits>&
operator<<(std::basic_ostream<_CharT, _TypeTraits>& os,
           const range_time_point& r)
{
    return os << '[' << r.left << ',' << r.right << ']';
}

#endif