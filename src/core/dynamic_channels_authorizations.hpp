/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "utils/sugar/zstring_view.hpp"

#include <string>
#include <string_view>
#include <vector>

class DynamicChannelsAuthorizations
{
public:
    DynamicChannelsAuthorizations(zstring_view allow, zstring_view deny);

    [[nodiscard]] bool is_authorized(std::string_view name) const noexcept;

private:
    bool all_allowed = true;
    bool all_denied  = false;

    std::vector<std::string> allowed_names;
    std::vector<std::string> denied_names;
};
