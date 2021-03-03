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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include <cassert>


struct FilePermissions
{
    explicit FilePermissions(unsigned mode) noexcept
    : mode_(mode)
    {
        assert(mode <= 0777);
    }

    unsigned permissions_as_uint() const noexcept
    {
        return mode_;
    }

    FilePermissions operator | (FilePermissions other) const noexcept
    {
        return FilePermissions(mode_ | other.mode_);
    }

    FilePermissions& operator |= (FilePermissions other) noexcept
    {
        mode_ |= other.mode_;
        return *this;
    }

    bool operator == (FilePermissions other) const noexcept
    {
        return mode_ == other.mode_;
    }

    bool operator != (FilePermissions other) const noexcept
    {
        return mode_ != other.mode_;
    }

    enum class BitPermissions : unsigned char
    {
        read = 4,
        write = 2,
        execute = 1,

        rw = read | write,
        all = read | write | execute,
    };

    friend BitPermissions operator | (BitPermissions lhs, BitPermissions rhs) noexcept
    {
        return BitPermissions(unsigned(lhs) | unsigned(rhs));
    }

    static FilePermissions user_permissions(BitPermissions bits) noexcept
    {
        assert(unsigned(bits) <= 7);
        return FilePermissions(unsigned(bits) << 6);
    }

    static FilePermissions group_permissions(BitPermissions bits) noexcept
    {
        assert(unsigned(bits) <= 7);
        return FilePermissions(unsigned(bits) << 3);
    }

    static FilePermissions user_and_group_permissions(BitPermissions bits) noexcept
    {
        return user_permissions(bits) | group_permissions(bits);
    }

    static FilePermissions other_permissions(BitPermissions bits) noexcept
    {
        assert(unsigned(bits) <= 7);
        return FilePermissions(unsigned(bits));
    }

private:
    unsigned mode_;
};

using BitPermissions = FilePermissions::BitPermissions;
