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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#pragma once

#include <vector>
#include <string>
#include <string_view>


#define RED_CHECK_WORKSPACE(wd) do {           \
    auto const mess__ = wd.check_final_state();    \
    RED_CHECK_MESSAGE(mess__.empty(), mess__); \
} while (0)

struct WorkingDirectory
{
    WorkingDirectory(std::string dirname, std::initializer_list<std::string_view> files);
    WorkingDirectory(std::string dirname);

    WorkingDirectory neested(std::string directory, std::initializer_list<std::string_view> files);

    struct FileIndex
    {
        const std::size_t index;
    };

    FileIndex add_file(std::string_view filename);

    std::string const& operator[](FileIndex file_index) noexcept;
    std::string const& operator[](std::size_t i) noexcept;
    std::string const& operator[](std::string filename);

    std::string const& dirname() const noexcept;

    std::string clean_and_get();
    std::string check_final_state();

    ~WorkingDirectory() noexcept(false);

private:
    struct Path
    {
        std::string name;
        bool is_checked;
    };
    std::vector<Path> paths;
    std::string directory;
    bool is_checked = false;
};
