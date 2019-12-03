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
   Author(s): David Fort

   A proxy that will capture all the traffic to the target
*/

#pragma once

#include <utility>
#include <string>
#include <string_view>

// \return {username, domain}
static std::pair<std::string, std::string>
extract_user_domain(std::string_view target_user)
{
    std::string::size_type
    pos = target_user.find('\\');
    if (pos != std::string::npos) {
        return {
            // username
            std::string(target_user.begin() + pos + 1, target_user.end()),
            // domain
            std::string(target_user.begin(), target_user.begin() + pos),
        };
    }

    pos = target_user.find('@');
    if (pos != std::string::npos) {
        return {
            // username
            std::string(target_user.begin(), target_user.begin() + pos),
            // domain
            std::string(target_user.begin() + pos + 1, target_user.end()),
        };
    }

    return {
        // username
        std::string(target_user),
        // domain
        std::string(),
    };
}

