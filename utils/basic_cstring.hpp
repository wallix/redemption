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

#ifndef REDEMPTION_UTILS_BASIC_CSTRING_HPP
#define REDEMPTION_UTILS_BASIC_CSTRING_HPP

#include <iosfwd>

#include <boost/type_traits/remove_const.hpp>

template<typename CharT>
struct basic_cstring
{
    typedef CharT basic_char_type;
    typedef typename boost::remove_const<CharT>::type char_type;

    basic_cstring(CharT * first, CharT * last)
    : first(first)
    , last(last)
    {}

    basic_cstring()
    : first("")
    , last("")
    {}

    bool empty() const
    { return this->first == this->last; }

    size_t size() const
    {return this->last - this->first; }

    void assign(CharT * first, CharT * last)
    {
        this->first = first;
        this->last = last;
    }

    bool operator==(CharT * s) const
    {
        CharT * p = this->first;
        while (p != last && *p == *s) {
            ++p;
            ++s;
        }
        return p == this->last && !*s;
    }

    bool operator==(const basic_cstring& other) const
    {
        if (this->size() != other.size()) {
            return false;
        }
        CharT * p1 = this->first;
        CharT * p2 = other.first;
        while (p1 != this->last && *p1 == *p2) {
            ++p1;
            ++p2;
        }
        return p1 == this->last;
    }

    bool operator<(CharT * s) const
    {
        CharT * p = this->first;
        while (p != last && *p < *s) {
            ++p;
            ++s;
        }
        return p == this->last ? *s : (*p < *s);
    }

    bool operator<(const basic_cstring& other) const
    {
        size_t min = this->size() < other.size() ? this->size() : other.size();
        CharT * p1 = this->first;
        CharT * p2 = other.first;
        CharT * e = p1 + min;
        while (p1 != e && *p1 < *p2) {
            ++p1;
            ++p2;
        }
        return p1 == e ? (other.size() > min) : (*p1 < *p2);
    }

    const CharT& operator[](size_t i) const
    {return this->first[i]; }

    CharT& operator[](size_t i)
    {return this->first[i]; }

    const CharT * begin() const
    { return this->first; }

    const CharT * end() const
    { return this->last; }

    CharT * begin()
    { return this->first; }

    CharT * end()
    { return this->last; }

private:
    CharT * first;
    CharT * last;
};

typedef basic_cstring<char> cstring;
typedef basic_cstring<const char> const_cstring;

template<typename CharT, typename Trait>
std::basic_ostream<CharT, Trait>&
operator<<(std::basic_ostream<CharT, Trait>& os, const basic_cstring<CharT>& cs)
{
    return os.write(cs.begin(), cs.size());
}

template<typename CharT, typename Trait>
std::basic_ostream<CharT, Trait>&
operator<<(std::basic_ostream<CharT, Trait>& os, const basic_cstring<const CharT>& cs)
{
    return os.write(cs.begin(), cs.size());
}

#endif
