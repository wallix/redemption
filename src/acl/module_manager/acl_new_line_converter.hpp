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
Copyright (C) Wallix 2010-2020
*/

#pragma once

#include <memory>
#include "utils/sugar/zstring_view.hpp"

// convert <br> to \n

struct AclNewLineConverter
{
    AclNewLineConverter(zstring_view brmsg) 
    {
        constexpr const char * substring = "<br>";
        constexpr std::string::size_type sublen = strlen(substring);
        constexpr const char * replacement = "\n";
        std::string brstr = std::string(brmsg);
        std::string::size_type n = 0;
        std::string::size_type last_n = 0;
        while ((n = brstr.find(substring, n)) != std::string::npos)
        {
            this->msg.append(brstr, last_n, n - last_n);
            n += sublen;
            this->msg.append(replacement);
            last_n = n;
        }
        this->msg.append(brstr, last_n, n - last_n);
    }

    zstring_view zstring() const noexcept
    {
        return this->msg;
    }

private:
    std::string msg;
};
