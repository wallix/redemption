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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#include "keyboard/keymap2.hpp"
#include "mod/internal/bouncer2_mod.hpp"
#include "test_only/gdi/test_graphic.hpp"

// TEST missing
int main()
{
    ScreenInfo screen_info{1, 1, BitsPerPixel{24}};

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);

    EventContainer events;
    TestGraphic gd(screen_info.width, screen_info.height);
    Bouncer2Mod d(gd, events, screen_info.width, screen_info.height);
    d.rdp_input_scancode(0, 0, 0, 0, &keymap);
}
