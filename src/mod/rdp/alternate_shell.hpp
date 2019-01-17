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

#include "utils/sugar/algostring.hpp"

#include <string>
#include <cstring>

struct get_alternate_shell_arguments
{
    struct App { char const* s; };
    struct Account { char const* s; };
    struct Password { char const* s; };

    [[nodiscard]] std::string operator()(
        std::string shell_arguments,
        App app,
        Account account,
        Password password
    ) const
    {
        auto replace = [&](char const* marker, char const* replacement){
            if (replacement && *replacement) {
                size_t pos = shell_arguments.find(marker, 0);
                if (pos != std::string::npos) {
                    shell_arguments.replace(pos, strlen(marker), replacement);
                }
            }
        };

        replace("${APPID}", app.s);
        replace("${USER}", account.s);
        replace("${PASSWORD}", password.s);

        return shell_arguments;
    }
};

constexpr get_alternate_shell_arguments get_alternate_shell_arguments {};


