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

#if !defined(__RECORDER_TIME_POINT_HPP__)
#define __RECORDER_TIME_POINT_HPP__

#include <iosfwd>
#include <string>
#include <stdexcept>
#include <boost/lexical_cast.hpp>

struct time_point {
    std::size_t time;

private:
    static std::size_t to_impl_time_point(const std::string& str)
    {
        std::size_t p = str.find_first_not_of("+-hms0123456789");
        if (p != std::string::npos) {
            std::string error = "invalid character 'x' at index ";
            error[19] = str[p];
            throw std::runtime_error(error + boost::lexical_cast<std::string>(p));
        }

        p = 0;
        std::size_t len = str.length();
        std::size_t plus = 0;
        std::size_t minus = 0;

        while (p < len){
            bool negate = false;
            for (; p != len; ++p){
                if ('-' == str[p]){
                    negate = !negate;
                }
                else if ('+' != str[p]) {
                    break;
                }
            }
            std::size_t s;
            std::size_t last = str.find_first_not_of("0123456789", p);
            if (p == last){
                throw std::runtime_error(std::string("bad format ([[+|-]time[h|m|s][...]]) at index ") + boost::lexical_cast<std::string>(p));
            }
            if (last != std::string::npos)
            {
                s = boost::lexical_cast<std::size_t>(str.substr(p, last - p));
                std::size_t h = 0;
                std::size_t m = 0;
                p = last;
                std::size_t coef = 1;

                for (int loop = 0; loop != 2; ++loop){
                    if (last != std::string::npos && (loop ? 'm' : 'h') == str[last]){
                        if (loop){
                            m = s;
                            coef = 1;
                        }
                        else {
                            h = s;
                            coef = 60;
                        }
                        ++p;
                        last = str.find_first_not_of("0123456789", p);
                        if (last == std::string::npos)
                            last = len;
                        if (p != last){
                            s = boost::lexical_cast<std::size_t>(str.substr(p, last - p));
                            p = last;
                        }
                        else {
                            s = 0;
                            break;
                        }
                    }
                }

                if (last == std::string::npos){
                    p = len;
                }
                else if ('s' == str[last]) {
                    coef = 1;
                    p += 2;
                }
                else if ('+' == str[p] && '-' == str[p]) {
                    ++p;
                }
                s = s * coef + h * (60*60) + m * 60;
            }
            else
            {
                s = boost::lexical_cast<std::size_t>(str.substr(p));
                p = len;
            }
            if (negate){
                minus += s;
            } else {
                plus += s;
            }
        }

        if (minus > plus){
            throw std::runtime_error(std::string("negative time"));
        }

        return plus - minus;
    }

public:
    ///@see time_point::parse()
    time_point(const std::string& s)
    : time(to_impl_time_point(s))
    {}

    time_point(std::size_t t = 0)
    : time(t)
    {}

    time_point& operator=(std::size_t t)
    {
        this->time = t;
        return *this;
    }

    /*time_point& operator=(const time_point& t)
    {
        this->time = t.time;
        return *this;
    }*/

    /**
     * @example "7h-20min+2s"
     * @example "-20m+5h"
     * @example "50000"
     * @example "50000s"
     * @example "1h+1h30"
     * @example "2h10m-5s"
     */
    void parse(const std::string& s)
    {
        this->time = to_impl_time_point(s);
    }

    operator std::size_t&()
    {
        return this->time;
    }

    operator const std::size_t&() const
    {
        return this->time;
    }
};

/*bool operator==(const time_point& t, const time_point& other)
{
    return t.time == other.time;
}
bool operator<(const time_point& t, const time_point& other)
{
    return t.time < other.time;
}
bool operator<=(const time_point& t, const time_point& other)
{
    return t.time <= other.time;
}
bool operator>(const time_point& t, const time_point& other)
{
    return t.time > other.time;
}
bool operator>=(const time_point& t, const time_point& other)
{
    return t.time >= other.time;
}
bool operator!=(const time_point& t, const time_point& other)
{
    return t.time != other.time;
}*/

inline time_point& operator+=(time_point& t, std::size_t n)
{
    t.time += n;
    return t;
}
inline time_point& operator-=(time_point& t, std::size_t n)
{
    t.time -= n;
    return t;
}
inline time_point& operator+=(time_point& t, time_point& other)
{
    t.time += other.time;
    return t;
}
inline time_point& operator-=(time_point& t, time_point& other)
{
    t.time -= other.time;
    return t;
}

template<typename _CharT, typename _TypeTraits>
inline std::basic_ostream<_CharT, _TypeTraits>&
operator<<(std::basic_ostream<_CharT, _TypeTraits>& os,
           const time_point& t)
{
    return os << t.time << 's';
}

namespace std {
    inline void swap(time_point& t1, time_point& t2)
    {
        std::size_t t = t1.time;
        t1.time = t2.time;
        t2.time = t;
    }
}

#endif