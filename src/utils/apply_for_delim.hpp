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
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_UTILS_APPLY_FOR_DELIM_HPP
#define REDEMPTION_UTILS_APPLY_FOR_DELIM_HPP

//#include "log.hpp"

struct is_blanck_fn
{
    bool operator()(char c) const noexcept
    { return c == ' ' || c == '\t'; }
};

template<class Fn, class IgnoreFn = is_blanck_fn>
void apply_for_delim(const char * cstr, char delim, Fn fn, IgnoreFn ignore = IgnoreFn(),
                     bool complete_item_extraction = false)
{
    while (ignore(*cstr)) {
        ++cstr;
    }

    auto delim_plus = [](const char * cstr, char delim, Fn fn, IgnoreFn ignore) {
        const char * delim_pos = ::strchr(cstr, delim);
        if (!delim_pos) {
            delim_pos = cstr + ::strlen(cstr) - 1;
        }
        else {
            if (delim_pos == cstr) {
                return;
            }

            --delim_pos;
        }
        while (ignore(*delim_pos)) {
            --delim_pos;
        }

        uint32_t data_length = delim_pos - cstr + 1;

        char * cstr_tmp = reinterpret_cast<char *>(::alloca(data_length + 1));
        memcpy(cstr_tmp, cstr, data_length);
        cstr_tmp[data_length] = '\0';

        const char * ccstr_tmp = cstr_tmp;
        fn(ccstr_tmp);
    };

    if (*cstr) {
        if (complete_item_extraction) {
            delim_plus(cstr, delim, fn, ignore);
        }
        else {
            fn(cstr);
        }

        while (*cstr) {
            while (*cstr && *cstr != delim) {
                ++cstr;
            }
            if (*cstr) {
                ++cstr;
                while (ignore(*cstr)) {
                    ++cstr;
                }

                if (complete_item_extraction) {
                    if (*cstr) {
                        delim_plus(cstr, delim, fn, ignore);
                    }
                }
                else {
                    fn(cstr);
                }
            }
        }
    }
}

#endif
