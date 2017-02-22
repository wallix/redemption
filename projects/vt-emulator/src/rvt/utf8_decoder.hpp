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

#include "rvt/ucs.hpp"

#include "cxx/keyword.hpp"
#include "cxx/attributes.hpp"
#include "utils/sugar/array.hpp"
#include "utils/sugar/bytes_t.hpp"
#include "utils/sugar/array_view.hpp"

#include <cassert>


namespace rvt
{

/// \return 1, 2, 3, 4 or 0 if an invalide unicode point
inline std::size_t unsafe_ucs4_to_utf8(uint32_t uc, bytes_t s)
{
    if (REDEMPTION_LIKELY(uc <= 0x7f)) {
        s[0] = uint8_t(uc);
        return 1;
    }

    if (REDEMPTION_LIKELY(uc <= 0x7ff)) {
        s[0] = uint8_t(0xc0 | ((uc >> 6)  & 0x1f));
        s[1] = uint8_t(0x80 |  (uc        & 0x3f));
        return 2;
    }

    if (uc <= 0xffff) {
        s[0] = uint8_t(0xe0 | ((uc >> 12) & 0x0f));
        s[1] = uint8_t(0x80 | ((uc >> 6)  & 0x3f));
        s[2] = uint8_t(0x80 |  (uc        & 0x3f));
        return 3;
    }

    if (uc <= 0x1ffff) {
        s[0] = uint8_t(0xf0 | ((uc >> 18) & 0x07));
        s[1] = uint8_t(0x80 | ((uc >> 12) & 0x3f));
        s[2] = uint8_t(0x80 | ((uc >> 6)  & 0x3f));
        s[3] = uint8_t(0x80 | ( uc        & 0x3f));
        return 4;
    }

    assert(uc && "invalide unicode point");
    return 0;
}

enum class Utf8ByteSize : unsigned char
{
    LenError,
    Len1,
    Len2,
    Len3,
    Len4,
};

constexpr uint8_t utf8_byte_size_table[] {
    // 0xxx x[xxx]
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    // 10xx x[xxx]  invalid value
    0, 0, 0, 0,
    0, 0, 0, 0,
    // 110x x[xxx]
    2, 2, 2, 2,
    // 1110 x[xxx]
    3, 3,
    // 1111 0[xxx]
    4,
    // 1111 1[xxx]  invalid value
    0,
};

constexpr Utf8ByteSize utf8_byte_size(uint8_t c) noexcept
{
    return Utf8ByteSize(utf8_byte_size_table[(c >> 3)]);
}


constexpr bool is_valid_utf8_sequence(uint8_t a) noexcept
{
    return (a & 0xc0) == 0x80;
}

constexpr uint32_t utf8_2_bytes_to_ucs(uint8_t a, uint8_t b) noexcept
{
    return ((a & 0x1F) << 6 ) | (b & 0x3F);
}


constexpr bool is_valid_utf8_sequence(uint8_t a, uint8_t b) noexcept
{
    return ((a & b & 0x80) | ((a | b) & 0x40)) == 0x80;
}

constexpr uint32_t utf8_3_bytes_to_ucs(uint8_t a, uint8_t b, uint8_t c) noexcept
{
    return ((a & 0x0F) << 12) | ((b & 0x3F) << 6) | (c & 0x3F);
}


constexpr bool is_valid_utf8_sequence(uint8_t a, uint8_t b, uint8_t c) noexcept
{
    return ((a & b & c & 0x80) | ((a | b | c) & 0x40)) == 0x80;
}

constexpr uint32_t utf8_4_bytes_to_ucs(uint8_t a, uint8_t b, uint8_t c, uint8_t d) noexcept
{
    return ((a & 0x07) << 18) | ((b & 0x3F) << 12) | ((c & 0x3F) << 6) | (d & 0x3F);
}

//constexpr ucs4_char replacement_character = 0xfffd; // � REPLACEMENT CHARACTER

struct Utf8Decoder
{
    using utf8_array = const_bytes_array;

    template<class F>
    F decode(utf8_array utf8_string, F && f)
    {
        if (utf8_string.empty()) {
            return f;
        }

        if (data_len_) {
            auto it = utf8_string.begin();
            auto end = utf8_string.end();
            auto data_it = data_;
            auto data_end = data_it + data_len_;

            while (data_it != data_end && it != end) {
                if (!advance_and_decode(checked_size{}, data_it, data_end, f)) {
                    if (data_end < std::end(data_)) {
                        *data_end = *it;
                        ++data_end;
                    }
                    else {
                        this->copy_to_data(data_it, data_end);
                        data_it = data_;
                        data_end = data_ + (data_end - data_);
                        utils::back(data_) = *it;
                    }
                    ++it;
                }
            }

            while (data_it != data_end && advance_and_decode(checked_size{}, data_it, data_end, f)) {
            }

            data_len_ = data_end - data_it;
            if (data_len_ && data_it != data_) {
                this->copy_to_data(data_it, data_end);
            }

            utf8_string = utf8_array{it, end};
            if (utf8_string.empty()) {
                return f;
            }
        }

        auto it = utf8_string.begin();
        auto last = it + utf8_string.size() / 4u * 4u;

        while (it < last) {
            advance_and_decode(no_checked_size{}, it, last, f);
        }

        last = utf8_string.end();
        while (it < last && advance_and_decode(checked_size{}, it, last, f)) {
        }

        data_len_ = this->copy_to_data(it, last);

        return f;
    }

    template<class F>
    F end_decode(F && f)
    {
        auto data_it = data_;
        auto data_end = data_it + data_len_;

        while (data_it != data_end) {
            if (!advance_and_decode(checked_size{}, data_it, data_end, f)) {
                f(*data_it);
                ++data_it;
            }
        }

        data_len_ = 0;

        return f;
    }

private:
    template<class CheckedSize, class It, class F>
    static bool advance_and_decode(CheckedSize checked_size, It & it, It const & last, F & f)
    {
        switch (utf8_byte_size(*it)) {
            case Utf8ByteSize::LenError:
            case Utf8ByteSize::Len1:
                f(*it);
                ++it;
                break;

            case Utf8ByteSize::Len2:
                if (checked_size && last - it < 2) {
                    return false;
                }

                if (is_valid_utf8_sequence(it[1])) {
                    f(utf8_2_bytes_to_ucs(*it, it[1]));
                    it += 2;
                }
                else {
                    f(*it);
                    ++it;
                }
                break;

            case Utf8ByteSize::Len3:
                if (checked_size && last - it < 3) {
                    return false;
                }

                if (is_valid_utf8_sequence(it[1], it[2])) {
                    f(utf8_3_bytes_to_ucs(*it, it[1], it[2]));
                    it += 3;
                }
                else {
                    f(*it);
                    ++it;
                }
                break;

            case Utf8ByteSize::Len4:
                if (checked_size && last - it < 4) {
                    return false;
                }

                if (is_valid_utf8_sequence(it[1], it[2], it[3])) {
                    f(utf8_4_bytes_to_ucs(*it, it[1], it[2], it[3]));
                    it += 4;
                }
                else {
                    f(*it);
                    ++it;
                }
                break;
        }

        return true;
    }

    // return std::copy(first, last, this->data_) - this->data_
    template<class It>
    std::size_t copy_to_data(It first, It const & last)
    {
        #ifdef NDEBUG
        auto const n = static_cast<int8_t>(last - first);
        #else
        auto const n = last - first;
        #endif
        switch (n) {
            case 4: data_[3] = first[3]; REDEMPTION_CXX_FALLTHROUGH;
            case 3: data_[2] = first[2]; REDEMPTION_CXX_FALLTHROUGH;
            case 2: data_[1] = first[1]; REDEMPTION_CXX_FALLTHROUGH;
            case 1: data_[0] = first[0]; REDEMPTION_CXX_FALLTHROUGH;
            case 0: break;
            default: assert(n && "bad value, isn't 0, 1, 2, 3 or 4");
        }
        return static_cast<std::size_t>(n);
    }

    using checked_size = std::true_type;
    using no_checked_size = std::false_type;
    uint8_t data_[4];
    unsigned data_len_ = 0;
};

}
