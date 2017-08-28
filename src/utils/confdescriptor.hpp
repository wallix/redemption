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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   Configuration file descriptor objects
*/

#pragma once

#include <string>

class GeneralCaps;
class BitmapCaps;
class OrderCaps;

bool general_caps_load(GeneralCaps & caps, char const * filename);
bool bitmap_caps_load(BitmapCaps & caps, char const * filename);
bool order_caps_load(OrderCaps & caps, char const * filename);

bool general_caps_load(GeneralCaps & caps, std::string const & filename)
{
    return general_caps_load(caps, filename.c_str());
}

bool bitmap_caps_load(BitmapCaps & caps, std::string const & filename)
{
    return bitmap_caps_load(caps, filename.c_str());
}

bool order_caps_load(OrderCaps & caps, std::string const & filename)
{
    return order_caps_load(caps, filename.c_str());
}
