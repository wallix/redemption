/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "utils/sugar/chars_to_int.hpp"


constexpr std::size_t config_description_count_values(char const* s)
{
    std::size_t len = 0;
    for (; *s; ++s) {
        if (*s == '=') {
            ++len;
        }
    }
    return len;
}

template<std::size_t N>
struct ConfigNamePos
{
    static const std::size_t size = N;

    std::array<std::string_view, N> names;
    std::array<uint8_t, N> bit_positions;
};

// Box is a type who looks like
// struct { static constexpr char const* string = "..."; }
// with as string format:
// "- name    = 0x....\n"
// "..."
// "- name    = 0x...."
template<class Box>
constexpr auto config_enum_description_to_array()
{
    ConfigNamePos<config_description_count_values(Box::string)> result {};
    std::string_view* name_ptr = result.names.data();
    uint8_t* bit_pos_ptr = result.bit_positions.data();
    for (char const* s = Box::string;;) {
        // skip "- "
        s += 2;

        // search end name
        auto end = s + 1;
        while (*end != ' ') {
            ++end;
        }
        *name_ptr++ = {s, end};

        // skip spaces
        s = end + 1;
        while (*s != '=') {
            ++s;
        }
        // skip "= 0x"
        s += 4;

        auto r = hexadecimal_chars_to_int<uint64_t>(s);
        s = r.ptr;

        uint8_t bitpos = 1;
        while (r.val >>= 1) {
            ++bitpos;
        }
        *bit_pos_ptr++ = bitpos;

        // skip "\n"
        if (!*s || !*++s) {
            break;
        }
    }

    return result;
}
