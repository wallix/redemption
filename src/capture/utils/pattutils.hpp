/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#pragma once

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/flags.hpp"


struct CapturePattern
{
    enum class PatternType : unsigned char
    {
        reg,
        str,
        exact_reg,
        exact_str,
    };

    enum class CaptureType : unsigned char
    {
        unknown,
        ocr = 0b01,
        kbd = 0b10,
    };

    struct Filters
    {
        bool is_ocr = false;
        bool is_kbd = false;
    };

    CapturePattern() = default;

    CapturePattern(Filters filters, PatternType patt_type, chars_view pattern) noexcept
    : flags(0u
      | unsigned(filters.is_ocr ? CaptureType::ocr : CaptureType::unknown) << 4
      | unsigned(filters.is_kbd ? CaptureType::kbd : CaptureType::unknown) << 4
      | (static_cast<unsigned>(patt_type) & 0b1111)
    )
    , patt_len(static_cast<unsigned>(pattern.size()))
    , patt(pattern.data())
    {}

    PatternType pattern_type() const noexcept { return PatternType(flags & 0b1111); }
    CaptureType capture_type() const noexcept { return CaptureType(flags >> 4); }

    bool is(CaptureType cap_type) const noexcept { return flags & (unsigned(cap_type) << 4); }
    bool is_ocr() const noexcept { return is(CaptureType::ocr); }
    bool is_kbd() const noexcept { return is(CaptureType::kbd); }

    chars_view pattern() const noexcept { return {patt, patt_len}; }

private:
    unsigned flags = 0;
    unsigned patt_len = 0;
    char const* patt = "";
};

constexpr char capture_pattern_separator = '\x01';

CapturePattern parse_capture_pattern(chars_view pattern_rule);

bool contains_ocr_pattern(chars_view soh_separated_patterns);
bool contains_kbd_pattern(chars_view soh_separated_patterns);
bool contains_kbd_or_ocr_pattern(chars_view soh_separated_patterns);
