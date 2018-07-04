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

#pragma once

#include <mln/trait_value_.hh>

namespace ocr
{
    class rgb8
    {
    public:
        typedef unsigned char value_type;

    public:
        explicit rgb8() = default;

        constexpr rgb8(value_type c1, value_type c2, value_type c3) noexcept
        : components{c1, c2, c3}
        {}

        constexpr value_type red()   const noexcept { return this->components[0]; }
        constexpr value_type green() const noexcept { return this->components[1]; }
        constexpr value_type blue()  const noexcept { return this->components[2]; }

        void red(value_type x)   noexcept { this->components[0] = x; }
        void green(value_type x) noexcept { this->components[1] = x; }
        void blue(value_type x)  noexcept { this->components[2] = x; }

        template<class Color>
        constexpr bool operator==(const Color & c) const noexcept
        {
            return this->red() == c.red()
                && this->green() == c.green()
                && this->blue() == c.blue();
        }

        template<class Color>
        constexpr bool operator!=(const Color & c) const noexcept
        { return !(*this == c); }

    private:
        value_type components[3];
    };
} // namespace ocr


namespace mln
{
    namespace trait
    {
        template <>
        struct value_< ::ocr::rgb8>
        {
            static ::ocr::rgb8::value_type max() { return 255u; }

            template<class Ch, class Tr>
            static void read_value(std::basic_istream<Ch, Tr>& is, ::ocr::rgb8 & v)
            { is.read(reinterpret_cast<char*>(&v), sizeof(::ocr::rgb8)); }
        };
    } // namespace trait
} // namespace mln
