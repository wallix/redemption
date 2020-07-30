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
        const char * substring = "<br>";
        const char * replacement = "\n";
        unsigned lsub = 4;
        unsigned ctx = 0;
        for (auto x: brmsg){
            this->msg.push_back(x);
            if (x == substring[0]){ ctx = 1; continue; }
            if (ctx > 0 and (x == substring[ctx])){
                if (ctx < (lsub - 1)){ ctx++; continue;}
                else {
                    this->msg.erase(this->msg.end() - lsub, this->msg.end());
                    this->msg += replacement;
                }
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
