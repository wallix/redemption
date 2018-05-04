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

#include <algorithm>
#include <iterator>
#include <cassert>

// format: $directory '/' $year'-'$month'-'$day '/' $filename
DateDirFromFilename::DateDirFromFilename(array_view_const_char path) noexcept
: start_path(path.begin())
, end_path(path.end())
{
    auto const e = path.end();
    auto before_senti = path.begin();
    auto date_senti = std::find(before_senti, e, '/');

    if (date_senti != e) {
        ++date_senti;
        decltype(date_senti) it;
        while (e != (it = std::find(date_senti, e, '/'))) {
            before_senti = date_senti;
            date_senti = it+1;
        }

        auto is_digit = [&](int i) { return '0' <= before_senti[i] && before_senti[i] <= '9'; };
        if (date_senti - before_senti == 11
         && is_digit(0) && is_digit(1) && is_digit(2) && is_digit(3)
         && before_senti[4] == '-'
         && is_digit(5) && is_digit(6)
         && before_senti[4] == '-'
         && is_digit(8) && is_digit(9)
         && before_senti[10] == '/'
        ) {
            this->start_date_it = before_senti;
        }
        else {
            this->start_date_it = date_senti;
        }
        this->start_filename_it = date_senti;
    }
    else {
        this->start_date_it = this->start_path;
        this->start_filename_it = this->start_path;
    }
}
