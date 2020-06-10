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

#pragma once

#include "utils/sugar/zstring_view.hpp"

enum BackEvent_t
{
    BACK_EVENT_NONE = 0,
    BACK_EVENT_NEXT,     // MODULE FINISHED, ASKING FOR NEXT MODULE
    BACK_EVENT_STOP = 4, // MODULE FINISHED, ASKING TO LEAVE SESSION
    BACK_EVENT_REFRESH,  // MODULE ASKED DATA TO ACL, WAITING FOR ACL REFRESH
};

inline zstring_view signal_name(BackEvent_t signal)
{
     return signal == BACK_EVENT_NONE ? "BACK_EVENT_NONE"_zv
          : signal == BACK_EVENT_NEXT  ?"BACK_EVENT_NEXT"_zv
          : signal == BACK_EVENT_REFRESH ? "BACK_EVENT_REFRESH"_zv
          : signal == BACK_EVENT_STOP ? "BACK_EVENT_STOP"_zv
          : "BACK_EVENT_UNKNOWN"_zv;
}

