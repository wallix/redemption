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

#include <cstdint>

namespace configs
{

enum class LoggableCategory : char
{
    Unloggable,
    Loggable,
    LoggableButWithPassword,
};

template<std::size_t N>
struct U64BitFlags
{
    uint64_t loggable_bits[N];
    uint64_t unloggable_bits[N];

    LoggableCategory operator()(unsigned i) const noexcept
    {
        if (loggable_bits[i/64] & (uint64_t{1} << (i%64)))  {
            return LoggableCategory::Loggable;
        }

        if (unloggable_bits[i/64] & (uint64_t{1} << (i%64))) {
            return LoggableCategory::LoggableButWithPassword;
        }

        return LoggableCategory::Unloggable;
    }
};

} // namespace configs
