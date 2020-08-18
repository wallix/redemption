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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/test_card_mod.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/test_card_mod/"

RED_AUTO_TEST_CASE(TestTestCardMod)
{
    uint16_t width = 800;
    uint16_t height = 600;
    TestGraphic gd(width, height);
    GdForwarder gd_forwarder(gd);
    TestCardMod mod(gd_forwarder, width, height, global_font());
    RED_CHECK_NO_THROW(mod.init());
    RED_CHECK_IMG(gd, IMG_TEST_PATH "card_mod_1.png");
}
