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

namespace detail
{
    struct int_to_chars_result_access;
}

struct int_to_chars_result
{
    int_to_chars_result() = default;

    char const* data() const noexcept { return buffer + ibeg; }
    std::size_t size() const noexcept { return std::size_t(20 - ibeg); }

private:
    friend detail::int_to_chars_result_access;

    char buffer[20];
    unsigned ibeg = 20;
};

struct int_to_zchars_result
{
    int_to_zchars_result() noexcept
    {
        buffer[20] = '\0';
    }

    char const* data() const noexcept { return buffer + ibeg; }
    std::size_t size() const noexcept { return std::size_t(20 - ibeg); }

    char const* c_str() const noexcept { return data(); }

private:
    friend detail::int_to_chars_result_access;

    char buffer[21];
    unsigned ibeg = 20;
};


// to decimal chars
//@{
template<class T>
int_to_chars_result int_to_decimal_chars(T n) noexcept;

template<class T>
int_to_zchars_result int_to_decimal_zchars(T n) noexcept;

template<class T>
void int_to_decimal_chars(int_to_chars_result& out, T n) noexcept;

template<class T>
void int_to_decimal_zchars(int_to_zchars_result& out, T n) noexcept;
//@}


// to hexadecimal upper case chars
//@{
template<class T>
int_to_chars_result int_to_hexadecimal_chars(T n) noexcept;

template<class T>
int_to_zchars_result int_to_hexadecimal_zchars(T n) noexcept;

template<class T>
void int_to_hexadecimal_chars(int_to_chars_result& out, T n) noexcept;

template<class T>
void int_to_hexadecimal_zchars(int_to_zchars_result& out, T n) noexcept;
//@}


// to hexadecimal lower case chars
//@{
template<class T>
int_to_chars_result int_to_hexadecimal_lower_chars(T n) noexcept;

template<class T>
int_to_zchars_result int_to_hexadecimal_lower_zchars(T n) noexcept;

template<class T>
void int_to_hexadecimal_lower_chars(int_to_chars_result& out, T n) noexcept;

template<class T>
void int_to_hexadecimal_lower_zchars(int_to_zchars_result& out, T n) noexcept;
//@}


// to fixed hexadecimal upper case chars
//@{
template<int NbBytes = -1, class T>
int_to_chars_result int_to_fixed_hexadecimal_chars(T n) noexcept;

template<int NbBytes = -1, class T>
int_to_zchars_result int_to_fixed_hexadecimal_zchars(T n) noexcept;

template<int NbBytes = -1, class T>
void int_to_fixed_hexadecimal_chars(int_to_chars_result& out, T n) noexcept;

template<int NbBytes = -1, class T>
void int_to_fixed_hexadecimal_zchars(int_to_zchars_result& out, T n) noexcept;

template<int NbBytes = -1, class T>
char* int_to_fixed_hexadecimal_chars(char* out, T n) noexcept;
//@}


// to fixed hexadecimal upper case chars
//@{
template<int NbBytes = -1, class T>
int_to_chars_result int_to_fixed_hexadecimal_lower_chars(T n) noexcept;

template<int NbBytes = -1, class T>
int_to_zchars_result int_to_fixed_hexadecimal_lower_zchars(T n) noexcept;

template<int NbBytes = -1, class T>
void int_to_fixed_hexadecimal_lower_chars(int_to_chars_result& out, T n) noexcept;

template<int NbBytes = -1, class T>
void int_to_fixed_hexadecimal_lower_zchars(int_to_zchars_result& out, T n) noexcept;

template<int NbBytes = -1, class T>
char* int_to_fixed_hexadecimal_lower_chars(char* out, T n) noexcept;
//@}


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
    inline char* to_decimal_chars_impl(char *end, UInt value) noexcept
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
    inline char* to_decimal_chars(char *end, Int value) noexcept
    {
        static_assert(std::is_integral_v<Int>);
        static_assert(sizeof(Int) <= 64);

        if constexpr (std::is_signed_v<Int>) {
            using UInt = std::conditional_t<(sizeof(Int) < sizeof(int)), unsigned, std::make_unsigned_t<Int>>;
            auto abs_value = static_cast<UInt>(value);
            bool negative = (value < 0);
            if (negative) abs_value = 0 - abs_value;
            char* begin = to_decimal_chars_impl(end, abs_value);
            if (negative) *--begin = '-';
            return begin;
        }
        else if constexpr (sizeof(Int) < sizeof(unsigned)) {
            return to_decimal_chars_impl(end, unsigned(value));
        }
        else {
            return to_decimal_chars_impl(end, value);
        }
    }

    constexpr inline auto& hex_upper_table = "0123456789ABCDEF";
    constexpr inline auto& hex_lower_table = "0123456789abcdef";

    template<class UInt>
    inline char* to_hexadecimal_chars(char *end, UInt value, char const(&hex_table)[17]) noexcept
    {
        static_assert(std::is_unsigned_v<UInt>);
        static_assert(sizeof(UInt) <= 64);

        char* out = end;

        while (value > 0xf) {
            *--out = hex_table[value & 0xf];
            value >>= 4;
        }

        if (value <= 0xf) {
            *--out = hex_table[value & 0xf];
        }

        return out;
    }

    template<int NbBytes, class T>
    inline char* to_fixed_hexadecimal_chars(char* out, T n, char const(&hex_table)[17]) noexcept
    {
        static_assert(std::is_unsigned_v<T>);
        static_assert(sizeof(T) <= 64);
        static_assert(NbBytes == -1 || NbBytes <= int(sizeof(T)),
        "number of NbBytes must not exceed sizeof(T)");

        constexpr std::size_t start = (NbBytes == -1)
            ? 0
            : (sizeof(T) - std::size_t(NbBytes));

        for (std::size_t i = start; i < sizeof(T); ++i) {
            std::size_t d = (sizeof(T) - 1 - i) * 8;
            *out++ = hex_table[(n >> (d+4)) & 0xfu];
            *out++ = hex_table[(n >>  d   ) & 0xfu];
        }

        return out;
    }

    struct int_to_chars_result_access
    {
        template<class T>
        inline static char* buffer(T& r) noexcept
        {
            return r.buffer;
        }

        template<class T>
        inline static void set_ibeg(T& r, std::ptrdiff_t n) noexcept
        {
            r.ibeg = unsigned(n);
        }
    };
}

template<class T>
inline int_to_chars_result int_to_decimal_chars(T n) noexcept
{
    int_to_chars_result r;
    int_to_decimal_chars(r, n);
    return r;
}

template<class T>
inline int_to_zchars_result int_to_decimal_zchars(T n) noexcept
{
    int_to_zchars_result r;
    int_to_decimal_zchars(r, n);
    return r;
}

template<class T>
inline int_to_chars_result int_to_hexadecimal_chars(T n) noexcept
{
    int_to_chars_result r;
    int_to_hexadecimal_chars(r, n);
    return r;
}

template<class T>
inline int_to_zchars_result int_to_hexadecimal_zchars(T n) noexcept
{
    int_to_zchars_result r;
    int_to_hexadecimal_zchars(r, n);
    return r;
}

template<class T>
inline int_to_chars_result int_to_hexadecimal_lower_chars(T n) noexcept
{
    int_to_chars_result r;
    int_to_hexadecimal_lower_chars(r, n);
    return r;
}

template<class T>
inline int_to_zchars_result int_to_hexadecimal_lower_zchars(T n) noexcept
{
    int_to_zchars_result r;
    int_to_hexadecimal_lower_zchars(r, n);
    return r;
}

template<int NbBytes, class T>
inline int_to_chars_result int_to_fixed_hexadecimal_chars(T n) noexcept
{
    int_to_chars_result r;
    int_to_fixed_hexadecimal_chars<NbBytes>(r, n);
    return r;
}

template<int NbBytes, class T>
inline int_to_zchars_result int_to_fixed_hexadecimal_zchars(T n) noexcept
{
    int_to_zchars_result r;
    int_to_fixed_hexadecimal_zchars<NbBytes>(r, n);
    return r;
}

template<int NbBytes, class T>
inline int_to_chars_result int_to_fixed_hexadecimal_lower_chars(T n) noexcept
{
    int_to_chars_result r;
    int_to_fixed_hexadecimal_lower_chars<NbBytes>(r, n);
    return r;
}

template<int NbBytes, class T>
inline int_to_zchars_result int_to_fixed_hexadecimal_lower_zchars(T n) noexcept
{
    int_to_zchars_result r;
    int_to_fixed_hexadecimal_lower_zchars<NbBytes>(r, n);
    return r;
}


template<class T>
inline void int_to_decimal_chars(int_to_chars_result& out, T n) noexcept
{
    using access = detail::int_to_chars_result_access;
    auto buffer = access::buffer(out);
    char* end = buffer + 20;
    access::set_ibeg(out, detail::to_decimal_chars(end, n) - buffer);
}

template<class T>
inline void int_to_decimal_zchars(int_to_zchars_result& out, T n) noexcept
{
    using access = detail::int_to_chars_result_access;
    auto buffer = access::buffer(out);
    char* end = buffer + 20;
    access::set_ibeg(out, detail::to_decimal_chars(end, n) - buffer);
}

template<class T>
inline void int_to_hexadecimal_chars(int_to_chars_result& out, T n) noexcept
{
    using access = detail::int_to_chars_result_access;
    auto buffer = access::buffer(out);
    char* end = buffer + 20;
    char* begin = detail::to_hexadecimal_chars(end, n, detail::hex_upper_table);
    access::set_ibeg(out, begin - buffer);
}

template<class T>
inline void int_to_hexadecimal_zchars(int_to_zchars_result& out, T n) noexcept
{
    using access = detail::int_to_chars_result_access;
    auto buffer = access::buffer(out);
    char* end = buffer + 20;
    char* begin = detail::to_hexadecimal_chars(end, n, detail::hex_upper_table);
    access::set_ibeg(out, begin - buffer);
}

template<class T>
inline void int_to_hexadecimal_lower_chars(int_to_chars_result& out, T n) noexcept
{
    using access = detail::int_to_chars_result_access;
    auto buffer = access::buffer(out);
    char* end = buffer + 20;
    char* begin = detail::to_hexadecimal_chars(end, n, detail::hex_lower_table);
    access::set_ibeg(out, begin - buffer);
}

template<class T>
inline void int_to_hexadecimal_lower_zchars(int_to_zchars_result& out, T n) noexcept
{
    using access = detail::int_to_chars_result_access;
    auto buffer = access::buffer(out);
    char* end = buffer + 20;
    char* begin = detail::to_hexadecimal_chars(end, n, detail::hex_lower_table);
    access::set_ibeg(out, begin - buffer);
}

template<int NbBytes, class T>
inline void int_to_fixed_hexadecimal_chars(int_to_chars_result& out, T n) noexcept
{
    using access = detail::int_to_chars_result_access;
    auto buffer = access::buffer(out);
    constexpr int ibeg = 20 - (NbBytes == -1 ? int(sizeof(T)) : NbBytes) * 2;
    int_to_fixed_hexadecimal_chars<NbBytes>(buffer + ibeg, n);
    access::set_ibeg(out, ibeg);
}

template<int NbBytes, class T>
inline void int_to_fixed_hexadecimal_zchars(int_to_zchars_result& out, T n) noexcept
{
    using access = detail::int_to_chars_result_access;
    auto buffer = access::buffer(out);
    constexpr int ibeg = 20 - (NbBytes == -1 ? int(sizeof(T)) : NbBytes) * 2;
    int_to_fixed_hexadecimal_chars<NbBytes>(buffer + ibeg, n);
    access::set_ibeg(out, ibeg);
}

template<int NbBytes, class T>
inline void int_to_fixed_hexadecimal_lower_chars(int_to_chars_result& out, T n) noexcept
{
    using access = detail::int_to_chars_result_access;
    auto buffer = access::buffer(out);
    constexpr int ibeg = 20 - (NbBytes == -1 ? int(sizeof(T)) : NbBytes) * 2;
    int_to_fixed_hexadecimal_lower_chars<NbBytes>(buffer + ibeg, n);
    access::set_ibeg(out, ibeg);
}

template<int NbBytes, class T>
inline void int_to_fixed_hexadecimal_lower_zchars(int_to_zchars_result& out, T n) noexcept
{
    using access = detail::int_to_chars_result_access;
    auto buffer = access::buffer(out);
    constexpr int ibeg = 20 - (NbBytes == -1 ? int(sizeof(T)) : NbBytes) * 2;
    int_to_fixed_hexadecimal_lower_chars<NbBytes>(buffer + ibeg, n);
    access::set_ibeg(out, ibeg);
}

template<int NbBytes, class T>
inline char* int_to_fixed_hexadecimal_chars(char* out, T n) noexcept
{
    return detail::to_fixed_hexadecimal_chars<NbBytes>(out, n, detail::hex_upper_table);
}

template<int NbBytes, class T>
inline char* int_to_fixed_hexadecimal_lower_chars(char* out, T n) noexcept
{
    return detail::to_fixed_hexadecimal_chars<NbBytes>(out, n, detail::hex_lower_table);
}
