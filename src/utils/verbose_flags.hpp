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

#include <type_traits>

#include <cinttypes>

template<class E>
struct implicit_bool_flags
{
    operator bool() const { return bool(this->flags_); }
    operator E() const { return this->flags_; }

    implicit_bool_flags operator | (E y) const
    { return {this->flags_ | y}; }

    implicit_bool_flags operator & (E y) const
    { return {this->flags_ & y}; }

    implicit_bool_flags(E flags) : flags_(flags) {}

#ifdef __clang__
private:
    template<class T> implicit_bool_flags operator | (T const &) const;
    template<class T> implicit_bool_flags operator & (T const &) const;
#else
    template<class T> implicit_bool_flags operator | (T const &) const = delete;
    template<class T> implicit_bool_flags operator & (T const &) const = delete;
#endif

private:
    E const flags_;
};

namespace detail
{
    struct to_verbose_flags_
    {
        uint32_t verbose;

        template<class E>
        operator E () const
        {
            static_assert(std::is_enum<E>::value, "must be a enum type");
            return static_cast<E>(this->verbose);
        }
    };
}

inline detail::to_verbose_flags_
to_verbose_flags(uint32_t verbose)
{ return {verbose}; }

#define REDEMPTION_VERBOSE_FLAGS(visibility, verbose_member_name)   \
    enum class Verbose : uint32_t;                             \
                                                                    \
    friend Verbose operator | (Verbose x, Verbose y) \
    { return Verbose(uint32_t(x) | uint32_t(y)); }             \
    friend Verbose operator & (Verbose x, Verbose y) \
    { return Verbose(uint32_t(x) & uint32_t(y)); }             \
                                                                    \
visibility:                                                         \
    implicit_bool_flags<Verbose> const verbose_member_name;    \
                                                                    \
public:                                                             \
    enum class Verbose : uint32_t


#define REDEMPTION_VERBOSE_FLAGS_DEF(enum_name)            \
    enum class enum_name : uint32_t;                       \
                                                           \
    inline enum_name operator | (enum_name x, enum_name y) \
    { return enum_name(uint32_t(x) | uint32_t(y)); }       \
    inline enum_name operator & (enum_name x, enum_name y) \
    { return enum_name(uint32_t(x) & uint32_t(y)); }       \
                                                           \
    enum class enum_name : uint32_t
