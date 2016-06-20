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

#include "utils/log.hpp"

template<typename F>
bool contains_pattern(F f, unsigned int expected_result,
        const char * soh_separated_patterns) {
    unsigned int actual_result = 0;

    const char * pattern = soh_separated_patterns;

    while (*pattern)
    {
        if ((*pattern == '\x01') || (*pattern == '\t') || (*pattern == ' ')) {
            pattern++;
            continue;
        }

        actual_result |= f(pattern);

        const char * separator = strchr(pattern, '\x01');
        if (!separator) {
            break;
        }

        pattern = separator + 1;
    }

    return (actual_result & expected_result);
}

#define KBD_PATTERN 1
#define OCR_PATTERN 2

inline static unsigned int get_pattern_type(const char * pattern) {
    //LOG(LOG_INFO, "pattern=\"%s\" pattern_length=%u", pattern, pattern_length);
    if (strcasestr(pattern, "$kbd:") == pattern) {
        return KBD_PATTERN;
    }

    if ((*pattern != '$') ||
        (strcasestr(pattern, "$ocr:") == pattern)) {
        return OCR_PATTERN;
    }

    if ((strcasestr(pattern, "$ocr-kbd:") == pattern) ||
        (strcasestr(pattern, "$kbd-ocr:") == pattern)) {
        return KBD_PATTERN | OCR_PATTERN;
    }

    return 0;
}

inline static bool contains_kbd_pattern(const char * soh_separated_patterns) {
    return contains_pattern(
            get_pattern_type,
            KBD_PATTERN,
            soh_separated_patterns
        );
}

inline static bool contains_kbd_or_ocr_pattern(const char * soh_separated_patterns) {
    return contains_pattern(
            get_pattern_type,
            KBD_PATTERN | OCR_PATTERN,
            soh_separated_patterns
        );
}

inline static bool contains_ocr_pattern(const char * soh_separated_patterns) {
    return contains_pattern(
            get_pattern_type,
            OCR_PATTERN,
            soh_separated_patterns
        );
}

