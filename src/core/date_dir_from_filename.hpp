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

#pragma once

#include "utils/sugar/array_view.hpp"


struct DateDirFromFilename
{
    explicit DateDirFromFilename(array_view_const_char path) noexcept;

    bool has_date() const noexcept
    {
        return this->start_date_it != this->start_filename_it;
    }

    array_view_const_char full_path() const noexcept { return {this->start_path, this->end_path}; }
    /// \return \a base_path() + \a dir_date_path()
    array_view_const_char directory() const noexcept { return {this->start_path, this->start_filename_it}; }
    array_view_const_char base_path() const noexcept { return {this->start_path, this->start_date_it}; }
    array_view_const_char date_path() const noexcept { return {this->start_date_it, this->start_filename_it}; }
    array_view_const_char filename() const noexcept { return {this->start_filename_it, this->end_path}; }

private:
    char const* start_path;
    char const* start_date_it;
    char const* start_filename_it;
    char const* end_path;
};
