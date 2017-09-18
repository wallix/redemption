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


struct PatternValue
{
    enum Cat { is_reg, is_str, is_exact_reg, is_exact_str };

    Cat cat = is_reg;
    bool is_ocr = false;
    bool is_kbd = false;
    array_view_const_char pattern = cstr_array_view("");
};

constexpr char string_pattern_separator = '\x01';

PatternValue get_pattern_value(array_view_const_char av);

bool contains_ocr_pattern(const char * soh_separated_patterns);
bool contains_kbd_pattern(const char * soh_separated_patterns);
bool contains_kbd_or_ocr_pattern(const char * soh_separated_patterns);
