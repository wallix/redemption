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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Meng Tan, Raphael Zhou
*/

#include "utils/theme.hpp"
#include "core/events.hpp"
#include "mod/internal/widget_test_mod.hpp"
#include "test_only/front/fake_front.hpp"
#include "test_only/core/font.hpp"

int main()
{
    ScreenInfo screen_info{800, 600, BitsPerPixel{24}};

    FakeFront front(screen_info);

    EventContainer events;
    WidgetTestMod d(front.gd(), events, front, screen_info.width, screen_info.height, global_font(), Theme());
}
