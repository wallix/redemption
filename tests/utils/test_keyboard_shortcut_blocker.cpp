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
    Copyright (C) Wallix 2021
    Author(s): Christophe Grosjean, Jonathan Poelen,
               Meng Tan, Raphael Zhou
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/keyboard_shortcut_blocker.hpp"

RED_AUTO_TEST_CASE(Test_KeyboardShortcutBlocker_0)
{
    KeyboardShortcutBlocker keyboard_shortcut_blocker(0x40c, std::string("Ctrl+S"), true);

    RED_CHECK(!keyboard_shortcut_blocker.scancode_mast_be_blocked(0x0, 0x1D));

    RED_CHECK(keyboard_shortcut_blocker.scancode_mast_be_blocked(0x0, 0x1F));

    RED_CHECK(keyboard_shortcut_blocker.scancode_mast_be_blocked(0x08000, 0x1F));

    RED_CHECK(!keyboard_shortcut_blocker.scancode_mast_be_blocked(0x08000, 0x1D));
}
