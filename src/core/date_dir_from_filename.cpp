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
Copyright (C) Wallix 2018
Author(s): Jonathan Poelen
*/

#include "core/date_dir_from_filename.hpp"

#include <iterator>
#include <cassert>


char const* DateDirFromFilename::c_str() const noexcept
{
    assert(!this->has_error());
    return str_date;
}

char const* DateDirFromFilename::begin() const noexcept
{
    assert(!this->has_error());
    return std::begin(str_date);
}

char const* DateDirFromFilename::end() const noexcept
{
    assert(!this->has_error());
    return std::end(str_date)-1;
}

bool DateDirFromFilename::has_error() const noexcept
{
    return !bool(*str_date);
}

// format: $id ',' $user ',' $target ',' $year$month$day '-' .....
DateDirFromFilename DateDirFromFilename::extract_date(const char* filename) noexcept
{
    const char* start_date = filename;
    for (int count = 0; count < 3 && *start_date; ++start_date) {
        if (*start_date == ',') {
            ++count;
        }
    }

    DateDirFromFilename date_from_file;
    char* str_date_end_pos = date_from_file.str_date;

    auto copy_n = [&](int max_digit) {
        for (int count = 0
            ; count < max_digit && *start_date && '0' <= *start_date && *start_date <= '9'
            ; ++count, ++str_date_end_pos, ++start_date
        ) {
            *str_date_end_pos = *start_date;
        }
    };

    // year
    copy_n(4); *str_date_end_pos++ = '-';
    // month
    copy_n(2); *str_date_end_pos++ = '-';
    // day
    copy_n(2); *str_date_end_pos++ = '/';
    *str_date_end_pos = 0;

    assert(str_date_end_pos <= std::end(date_from_file.str_date));

    if (str_date_end_pos != date_from_file.end()) {
        date_from_file.str_date[0] = 0;
    }

    return date_from_file;
}
