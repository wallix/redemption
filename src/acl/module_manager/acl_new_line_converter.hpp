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
        unsigned ctx = 0;
        for (auto x: brmsg){
            this->msg.push_back(x);
            if (x == '<' && ctx == 0){ ctx = 1; continue; }
            if (x == 'b' && ctx == 1){ ctx = 2; continue; }
            if (x == 'r' && ctx == 2){ ctx = 3; continue; }
            if (x == '>' && ctx == 3){
                this->msg.pop_back();
                this->msg.pop_back();
                this->msg.pop_back();
                this->msg.pop_back();
                this->msg.push_back('\n');
            }
            ctx = 0;
        }
    }

    zstring_view zstring() const noexcept
    {
        return this->msg;
    }

private:
    std::string msg;
};
