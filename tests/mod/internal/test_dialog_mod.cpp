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
   Author(s): Christophe Grosjean, Meng Tan

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "configs/config.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "RAIL/client_execute.hpp"
#include "mod/internal/dialog_mod.hpp"
#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "test_only/front/fake_front.hpp"
#include "test_only/core/font.hpp"
#include "utils/timebase.hpp"

namespace
{
    struct DialogModContextTest
    {
        ScreenInfo screen_info{800, 600, BitsPerPixel{24}};
        FakeFront front{screen_info};
        WindowListCaps window_list_caps;
        TimeBase time_base;
        ClientExecute client_execute{time_base, front.gd(), front, window_list_caps, false};

        Inifile ini;
        Theme theme;

        DialogMod d;
        Keymap keymap{*find_layout_by_id(KeyLayout::KbdId(0x040C))};

        DialogModContextTest(char const* button_text, ChallengeOpt has_challenge = NO_CHALLENGE)
        : d(ini, front.gd(), front, screen_info.width, screen_info.height,
            Rect(0, 0, 799, 599), "Title", "Hello, World", button_text,
            client_execute, global_font(), theme, has_challenge)
        {
            d.init();
        }

        void keydown(uint8_t scancode)
        {
            keymap.event(Keymap::KbdFlags(), Keymap::Scancode(scancode));
            d.rdp_input_scancode(Keymap::KbdFlags(), Keymap::Scancode(scancode), 0, keymap);
        }
    };
}


RED_AUTO_TEST_CASE(TestDialogMod)
{
    DialogModContextTest dialog("Ok");
    dialog.keydown(0x1c); // enter
    RED_CHECK(dialog.ini.get<cfg::context::accept_message>());
}

RED_AUTO_TEST_CASE(TestDialogModReject)
{
    DialogModContextTest dialog("Cancel");
    dialog.keydown(0x01); // esc
    RED_CHECK(!dialog.ini.get<cfg::context::accept_message>());
}

RED_AUTO_TEST_CASE(TestDialogModChallenge)
{
    DialogModContextTest dialog("Cancel", CHALLENGE_ECHO);
    dialog.keydown(0x11); // 'z'
    dialog.keydown(0x12); // 'e'
    dialog.keydown(0x10); // 'a'
    dialog.keydown(0x10); // 'a'
    dialog.keydown(0x10); // 'a'
    dialog.keydown(0x10); // 'a'
    dialog.keydown(0x1c); // enter
    RED_CHECK_EQUAL("zeaaaa", dialog.ini.get<cfg::context::password>());
}
