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

#include "utils/sugar/movable_ptr.hpp"
#include "utils/sugar/bytes_view.hpp"

#include <string>
#include <cstdio>


struct File
{
    File(std::nullptr_t p) noexcept
      : f(p)
    {}

    File(File&&) = default;
    File& operator=(File&&) = default;

    File(char const* filename, const char* mode) noexcept
      : f(std::fopen(filename, mode))
    {}

    File(std::string const& filename, char const* mode) noexcept
      : File(filename.c_str(), mode)
    {}

    File(std::FILE* f) noexcept
      : f(f)
    {}

    explicit operator bool () const noexcept
    {
        return f;
    }

    std::FILE* get() noexcept
    {
        return f;
    }

    bool is_eof() const noexcept
    {
        return std::feof(f);
    }

    bool has_error() const noexcept
    {
        return std::ferror(f);
    }

    bytes_view read(bytes_view buffer) noexcept
    {
        return {buffer.data(), std::fread(buffer.data(), 1, buffer.size(), f)};
    }

    size_t write(const_bytes_view buffer) noexcept
    {
        return std::fwrite(buffer.data(), 1, buffer.size(), f);
    }

    bool flush() noexcept
    {
        return !std::fflush(f);
    }

    void close() noexcept
    {
        if (f) {
            std::fclose(f); /*NOLINT*/
            f = nullptr;
        }
    }

    ~File()
    {
        if (f) {
            std::fclose(f); /*NOLINT*/
        }
    }

private:
    movable_ptr<std::FILE> f;
};
