/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2012
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
   Martin Potier, Jonathan Poelen, Meng Tan
*/


#pragma once

#include "utils/difftimeval.hpp"


class FrozenTime : public TimeObj
{
    timeval tv;

public:
    explicit FrozenTime(std::chrono::microseconds ustime = 1533211681s) /*NOLINT*/
    {
        tv.tv_sec = ustime.count()/1000000;
        tv.tv_usec = ustime.count()%1000000;
    }

    timeval get_time() override
    {
        return tv;
    }
};
