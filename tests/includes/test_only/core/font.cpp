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

#include "test_only/core/font.hpp"
#include "core/font.hpp"

namespace
{
    Font font;
    Font deja_vu_14;
    Font lato_light_16;
}

Font const& global_font()
{
    return font;
}

Font const& global_font_deja_vu_14()
{
    if (!deja_vu_14.is_loaded()) {
        deja_vu_14 = Font(FIXTURES_PATH "/dejavu_14.rbf");
    }
    return deja_vu_14;
}

Font const& global_font_lato_light_16()
{
    if (!lato_light_16.is_loaded()) {
        lato_light_16 = Font{FIXTURES_PATH "/Lato-Light_16.rbf"};
    }
    return lato_light_16;
}
