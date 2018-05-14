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

#include "utils/sugar/cast.hpp"

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


struct get_session_probe_arguments
{
    struct Exe { char const* s; };
    struct Cookie { char const* s; };
    struct Title { char const* s; };
    struct Cbspl { char const* s; };

    [[nodiscard]] std::string operator()(
        std::string probe_arguments,
        Exe exe,
        Title title,
        Cookie cookie,
        Cbspl cbspl
    ) const
    {
        auto replace = [&](char const* marker, char const* replacement){
            size_t pos = 0;
            auto const marker_len = strlen(marker);
            auto const replacement_len = strlen(replacement);
            while ((pos = probe_arguments.find(marker, pos)) != std::string::npos) {
                probe_arguments.replace(pos, marker_len, replacement);
                pos += replacement_len;
            }
        };

        std::string cookie_param;
        if (cookie.s && *cookie.s) {
            cookie_param = "/#";
            cookie_param += cookie.s;
            cookie_param += ' ';
        }

        replace("${EXE_VAR}", exe.s);
        replace("${TITLE_VAR} ", title.s);
        replace("/${COOKIE_VAR} ", cookie_param.c_str());
        replace("${CBSPL_VAR} ", cbspl.s);

        return probe_arguments;
    }
};

constexpr get_session_probe_arguments get_session_probe_arguments {};
