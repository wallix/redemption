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

#include "log.hpp"

namespace
{
    static char log_proxy_psid[32] = "42";
}

namespace detail
{
    char const* get_log_proxy_psid() noexcept
    {
        return log_proxy_psid;
    }

    void set_log_proxy_psid(char const* spid) noexcept
    {
        char* s = log_proxy_psid;
        char* e = s + sizeof(log_proxy_psid) - 1;
        while (s != e && *spid) {
            *s++ = *spid++;
        }
        *s = '\0';
    }
}
