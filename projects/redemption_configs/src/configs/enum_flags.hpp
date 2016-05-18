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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <ios>
#include <type_traits>
#include <cassert>
#include <cstring>

#include "type_name.hpp"

namespace configs {


struct string_literal
{
  template<std::size_t N>
  constexpr string_literal(char const (&s)[N]) noexcept
  : s_(s)
  , size_(N-1)
  {}

  constexpr std::size_t size() const noexcept { return this->size_; }
  constexpr char const * data() const noexcept { return this->s_; }
  constexpr char const * c_str() const noexcept { return this->s_; }
  constexpr char operator[](std::size_t i) const noexcept { return this->s_[i]; }

  template<class Ch, class Tr>
  friend std::basic_ostream<Ch, Tr> &
  operator << (std::basic_ostream<Ch, Tr> & out, string_literal str) {
    return out.write(str.s_, std::streamsize(str.size_));
  }

private:
  char const * s_;
  std::size_t size_;
};

template<std::size_t... Sizes>
constexpr std::array<string_literal, sizeof...(Sizes)>
string_literals(char const (& ... s)[Sizes]) {
  return {{string_literal(s)...}};
}


template<class E>
struct enum_flags
{
    using type = E;

    constexpr unsigned to_uint() const noexcept { return i; }

    static E from_uint(unsigned i) noexcept {
        assert(E::check(i) && "invalid value");
        return static_cast<E>(enum_flags{safe_value{i}});
    }

    E & operator |= (enum_flags y) noexcept { i |= y.i; return static_cast<E&>(*this); }
    E & operator &= (enum_flags y) noexcept { i &= y.i; return static_cast<E&>(*this); }

    E & operator += (enum_flags y) noexcept { i |= y.i; return static_cast<E&>(*this); }
    E & operator -= (enum_flags y) noexcept { i &= ~y; return static_cast<E&>(*this); }

    constexpr E operator ~ () const noexcept { return from_uint(~i & mask()); }

    template<class Ch, class Tr, class EE>
    friend std::basic_ostream<Ch, Tr> &
    operator << (std::basic_ostream<Ch, Tr> & out, enum_flags<EE> e) {
        {
            auto enum_name = type_name<EE>(nullptr);
            out.write(enum_name.data(), std::streamsize(enum_name.size())) << "{";
        }
        auto && names = E::names();
        auto fisrt = names.begin();
        auto last = names.end();
        if (fisrt != last) {
            if (!e.i) {
                out << " " << *fisrt << "=0";
            }
            else {
                auto const oldf = out.setf(std::ios::hex, std::ios::basefield);
                bool has_value = 0;
                constexpr char const * s[] {" ", " | "};
                for (unsigned d = 1; ++fisrt != last; d <<= 1) {
                    if (e.i & d) {
                        out << s[has_value] << *fisrt << "=0x" << d;
                        has_value = 1;
                    }
                }
                out.setf(std::ios::hex, oldf);
            }
        }
        return out << " }";
    }

private:
    struct safe_value {
        constexpr explicit safe_value(unsigned i) : i(i) {}
        unsigned i;
    };

    constexpr enum_flags(safe_value x) noexcept : i(x.i) {
    }

protected:
    using base_type = enum_flags;

    template<unsigned i>
    struct static_value {
        static constexpr enum_flags value = enum_flags{safe_value{i}};
    };

    static constexpr unsigned mask() noexcept {
        // guaranted constexpr
        return std::integral_constant<unsigned, (1 << (decltype(E::names())().size() - 1)) - 1>::value;
    }

    static constexpr bool check(unsigned i) noexcept { return i <= mask(); }

private:
    unsigned i;
};

template<class E>
template<unsigned i>
constexpr enum_flags<E> enum_flags<E>::static_value<i>::value;


template<class E>
constexpr E operator | (enum_flags<E> x, enum_flags<E> y) noexcept {
    return enum_flags<E>::from_uint(x.to_uint() | y.to_uint());
}

template<class E>
constexpr E operator & (enum_flags<E> x, enum_flags<E> y) noexcept {
    return enum_flags<E>::from_uint(x.to_uint() & y.to_uint());
}

template<class E>
constexpr E operator + (enum_flags<E> x, enum_flags<E> y) noexcept {
    return enum_flags<E>::from_uint(x.to_uint() + y.to_uint());
}

template<class E>
constexpr E operator - (enum_flags<E> x, enum_flags<E> y) noexcept {
    return enum_flags<E>::from_uint(x.to_uint() - y.to_uint());
}



template<class E>
struct enum_list
{
    using type = E;

    constexpr unsigned to_uint() const noexcept { return i; }

    static E from_uint(unsigned i) noexcept {
        assert(E::check(i) && "invalid value");
        return static_cast<E>(enum_list{safe_value{i}});
    }

    template<class Ch, class Tr, class EE>
    friend std::basic_ostream<Ch, Tr> &
    operator << (std::basic_ostream<Ch, Tr> & out, enum_list<EE> e) {
        auto const enum_name = type_name<EE>(nullptr);
        return out.write(enum_name.data(), std::streamsize(enum_name.size()))
            << "{ " << E::names()[e.i] << "=" << e.i << " }";
    }

private:
    struct safe_value {
        constexpr explicit safe_value(unsigned i) : i(i) {}
        unsigned i;
    };

    constexpr enum_list(safe_value x) noexcept : i(x.i) {
    }

protected:
    using base_type = enum_list;

    template<unsigned i>
    struct static_value {
        static constexpr enum_list value = enum_list{safe_value{i}};
    };

    static constexpr unsigned max() noexcept {
        // guaranted constexpr
        return std::integral_constant<unsigned, decltype(E::names())().size() - 1>::value;
    }

    static constexpr bool check(unsigned i) noexcept { return i <= max(); }

private:
    unsigned i;
};

template<class E>
template<unsigned i>
constexpr enum_list<E> enum_list<E>::static_value<i>::value;

}
