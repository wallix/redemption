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
        constexpr const char * needle = "<br>";
        constexpr std::string::size_type needlelen = 4; // strlen(needle);
        constexpr const char * replacement = "\n";
        constexpr std::string::size_type replacementlen = 1; // strlen(replacement);

        auto haystack = brmsg.data();
        auto haystack_end = brmsg.data()+brmsg.size();
        while (true){
            auto it = std::search(haystack, haystack_end, needle, needle+needlelen);
            this->msg.insert(this->msg.end(), haystack, it);
            if (it == haystack_end){
                break;
            }
            this->msg.insert(this->msg.end(), replacement, replacement+replacementlen);
            haystack = it + needlelen;
        }
    }

    zstring_view zstring() const noexcept
    {
        return this->msg;
    }

private:
    std::string msg;
};
