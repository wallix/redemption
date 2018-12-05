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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them

*/

#define RED_TEST_MODULE TestRdpClientTestCard
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/internal/test_card_mod.hpp"

#include "test_only/front/fake_front.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TestShowTestCard)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);


    SessionReactor session_reactor;
    TestCardMod mod(session_reactor, front, screen_info.width, screen_info.height, global_font());
    RED_CHECK_NO_THROW(mod.draw_event(time(nullptr), front));
}
