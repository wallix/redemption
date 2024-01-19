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
Copyright (C) Wallix 2022
Author(s): Proxies Team
*/

#pragma once

#include "utils/sugar/chars_to_int.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include <string_view>

#include <cstdint>

#include <unistd.h>
#include <fcntl.h>


inline int64_t extract_memory_available(char const* str, ssize_t len)
{
    // format:
    //
    // MemTotal:       32786092 kB
    // MemFree:        26590104 kB
    // MemAvailable:   28614956 kB
    // ....

    // skip MemTotal
    char const* p = strchr(str, '\n');
    if (p) {
        // skip MemFree
        p = strchr(p + 1, '\n');
        using SV = std::string_view;
        SV prefix = "\nMemAvailable: ";
        // MemAvailable
        size_t remaining = checked_int(str + len - p);
        if (p && prefix.size() < remaining && prefix == SV(p, prefix.size())) {
            p += prefix.size();
            for (; *p; ++p) {
                if ('0' <= *p && *p <= '9') {
                    return decimal_chars_to_int<int64_t>(p).val;
                }
            }
        }
    }
    return -1;
}

inline bool check_memory_available(uint64_t min_available_kibi)
{
    if (min_available_kibi) {
        int fd = open("/proc/meminfo", O_RDONLY);
        if (fd >= 0) {
            unsigned maxlen = 256;
            char buffer[256];
            ssize_t len = read(fd, buffer, maxlen-1);
            close(fd);

            if (len > 3) {
                buffer[len] = '\0';
                auto available = extract_memory_available(buffer, len);
                if (available < 0) {
                    return true;
                }
                return min_available_kibi < uint64_t(available);
            }
        }
    }

    return true;
}
