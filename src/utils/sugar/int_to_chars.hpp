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
Copyright (C) Wallix 2021
Author(s): Proxy Team
*/

#pragma once

#include <type_traits>
#include <cstring>


struct int_to_chars_result
{
    char* data() noexcept { return buffer; }
    char const* data() const noexcept { return buffer + ibeg; }
    std::size_t size() const noexcept { return std::size_t(20 - ibeg); }

    template<class T>
    friend int_to_chars_result int_to_chars(T n) noexcept;

private:
    char buffer[20];
    unsigned ibeg;
};

struct int_to_zchars_result
{
    char* data() noexcept { return buffer; }
    char const* data() const noexcept { return buffer + ibeg; }
    std::size_t size() const noexcept { return std::size_t(20 - ibeg); }

    char const* c_str() const noexcept { return data(); }

    template<class T>
    friend int_to_zchars_result int_to_zchars(T n) noexcept;

private:
    char buffer[20];
    unsigned ibeg;
};

template<class T>
int_to_chars_result int_to_chars(T n) noexcept;

template<class T>
int_to_zchars_result int_to_zchars(T n) noexcept;


namespace detail
{
    inline constexpr char digit_pairs[][2] = {
        {'0', '0'}, {'0', '1'}, {'0', '2'}, {'0', '3'}, {'0', '4'}, {'0', '5'},
        {'0', '6'}, {'0', '7'}, {'0', '8'}, {'0', '9'}, {'1', '0'}, {'1', '1'},
        {'1', '2'}, {'1', '3'}, {'1', '4'}, {'1', '5'}, {'1', '6'}, {'1', '7'},
        {'1', '8'}, {'1', '9'}, {'2', '0'}, {'2', '1'}, {'2', '2'}, {'2', '3'},
        {'2', '4'}, {'2', '5'}, {'2', '6'}, {'2', '7'}, {'2', '8'}, {'2', '9'},
        {'3', '0'}, {'3', '1'}, {'3', '2'}, {'3', '3'}, {'3', '4'}, {'3', '5'},
        {'3', '6'}, {'3', '7'}, {'3', '8'}, {'3', '9'}, {'4', '0'}, {'4', '1'},
        {'4', '2'}, {'4', '3'}, {'4', '4'}, {'4', '5'}, {'4', '6'}, {'4', '7'},
        {'4', '8'}, {'4', '9'}, {'5', '0'}, {'5', '1'}, {'5', '2'}, {'5', '3'},
        {'5', '4'}, {'5', '5'}, {'5', '6'}, {'5', '7'}, {'5', '8'}, {'5', '9'},
        {'6', '0'}, {'6', '1'}, {'6', '2'}, {'6', '3'}, {'6', '4'}, {'6', '5'},
        {'6', '6'}, {'6', '7'}, {'6', '8'}, {'6', '9'}, {'7', '0'}, {'7', '1'},
        {'7', '2'}, {'7', '3'}, {'7', '4'}, {'7', '5'}, {'7', '6'}, {'7', '7'},
        {'7', '8'}, {'7', '9'}, {'8', '0'}, {'8', '1'}, {'8', '2'}, {'8', '3'},
        {'8', '4'}, {'8', '5'}, {'8', '6'}, {'8', '7'}, {'8', '8'}, {'8', '9'},
        {'9', '0'}, {'9', '1'}, {'9', '2'}, {'9', '3'}, {'9', '4'}, {'9', '5'},
        {'9', '6'}, {'9', '7'}, {'9', '8'}, {'9', '9'}
    };

    template<class UInt>
    inline char* decimal_to_chars_impl(char *end, UInt value) noexcept
    {
        static_assert(std::is_unsigned_v<UInt>);

        char* out = end;

        while (value >= 100) {
            out -= 2;
            memcpy(out, digit_pairs[value % 100], 2);
            value /= 100;
        }

        if (value < 10) {
            *--out = static_cast<char>('0' + value);
            return out;
        }

        out -= 2;
        memcpy(out, digit_pairs[value], 2);
        return out;
    }

    template<class Int>
    inline char* decimal_to_chars(char *end, Int value) noexcept
    {
        static_assert(std::is_integral_v<Int>);
        static_assert(sizeof(Int) <= 64);

        if constexpr (std::is_signed_v<Int>) {
            using UInt = std::conditional_t<(sizeof(Int) < sizeof(int)), unsigned, std::make_unsigned_t<Int>>;
            auto abs_value = static_cast<UInt>(value);
            bool negative = (value < 0);
            if (negative) abs_value = 0 - abs_value;
            char* begin = decimal_to_chars_impl(end, abs_value);
            if (negative) *--begin = '-';
            return begin;
        }
        else if constexpr (sizeof(Int) < sizeof(unsigned)) {
            return decimal_to_chars_impl(end, unsigned(value));
        }
        else {
            return decimal_to_chars_impl(end, value);
        }
    }
}

template<class T>
inline int_to_chars_result int_to_chars(T n) noexcept
{
    int_to_chars_result r;
    char* end = r.buffer + 20;
    r.ibeg = unsigned(detail::decimal_to_chars(end, n) - r.buffer);
    return r;
}

template<class T>
inline int_to_zchars_result int_to_zchars(T n) noexcept
{
    int_to_zchars_result r;
    char* end = r.buffer + 20;
    *end = '\0';
    r.ibeg = unsigned(detail::decimal_to_chars(end, n) - r.buffer);
    return r;
}
