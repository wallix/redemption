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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/keyboard_shortcut_blocker.hpp"

using Scancode = kbdtypes::Scancode;
using KbdFlags = kbdtypes::KbdFlags;

constexpr auto release = KbdFlags::Release;
constexpr auto downnnn = KbdFlags();

RED_AUTO_TEST_CASE(Test_KeyboardShortcutBlocker_Empty)
{
    KeyboardShortcutBlocker keyboard_shortcut_blocker(false);
    RED_CHECK(!keyboard_shortcut_blocker.has_shortcut());

    keyboard_shortcut_blocker.set_shortcuts(KeyLayout::KbdId(0x40c), ""_av);
    RED_CHECK(!keyboard_shortcut_blocker.has_shortcut());
}

RED_AUTO_TEST_CASE(Test_KeyboardShortcutBlocker_Error)
{
    KeyboardShortcutBlocker keyboard_shortcut_blocker(KeyLayout::KbdId(0x40c), "a+b"_av, false);
    RED_CHECK(!keyboard_shortcut_blocker.has_shortcut());
}

RED_AUTO_TEST_CASE(Test_KeyboardShortcutBlocker_0)
{
    KeyboardShortcutBlocker keyboard_shortcut_blocker(KeyLayout::KbdId(0x40c), "Ctrl+S"_av, true);
    RED_CHECK(keyboard_shortcut_blocker.has_shortcut());

    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::S));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::S));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::LCtrl));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::V));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::V));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::S));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::S));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::LCtrl));
}

RED_AUTO_TEST_CASE(Test_KeyboardShortcutBlocker_1)
{
    KeyboardShortcutBlocker keyboard_shortcut_blocker(KeyLayout::KbdId(0x40c), "Ctrl+Enter"_av, true);
    RED_CHECK(keyboard_shortcut_blocker.has_shortcut());

    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::Enter));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::Enter));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::LCtrl));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::V));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::V));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::Enter));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::Enter));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::LCtrl));
}

RED_AUTO_TEST_CASE(Test_KeyboardShortcutBlocker_2)
{
    KeyboardShortcutBlocker keyboard_shortcut_blocker(KeyLayout::KbdId(0x40c), "Ctrl+Enter,Alt+S"_av, true);
    RED_CHECK(keyboard_shortcut_blocker.has_shortcut());

    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::LCtrl));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::V));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::V));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::S));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::S));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::Enter));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::Enter));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::LAlt));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::S));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::S));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::LCtrl));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::S));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::S));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::LAlt));
}

RED_AUTO_TEST_CASE(Test_KeyboardShortcutBlocker_3)
{
    KeyboardShortcutBlocker keyboard_shortcut_blocker(KeyLayout::KbdId(0x40c), "Ctrl++"_av, true);
    RED_CHECK(keyboard_shortcut_blocker.has_shortcut());

    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::LCtrl));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x0D)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x0D)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::LShift));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x0D)));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x0D)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::LShift));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::LCtrl));
}

RED_AUTO_TEST_CASE(Test_KeyboardShortcutBlocker_4)
{
    KeyboardShortcutBlocker keyboard_shortcut_blocker(KeyLayout::KbdId(0x40c), ","_av, true);
    RED_CHECK(keyboard_shortcut_blocker.has_shortcut());

    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::LCtrl));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x32)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x32)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::LCtrl));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x32)));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x32)));
}

RED_AUTO_TEST_CASE(Test_KeyboardShortcutBlocker_5)
{
    KeyboardShortcutBlocker keyboard_shortcut_blocker(KeyLayout::KbdId(0x40c), "ctrl+,"_av, true);
    RED_CHECK(keyboard_shortcut_blocker.has_shortcut());

    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x32)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x32)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::LCtrl));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x32)));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x32)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::LCtrl));
}

RED_AUTO_TEST_CASE(Test_KeyboardShortcutBlocker_6)
{
    KeyboardShortcutBlocker keyboard_shortcut_blocker(KeyLayout::KbdId(0x40c), "ctrl++,"_av, true);
    RED_CHECK(keyboard_shortcut_blocker.has_shortcut());

    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x0D)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x0D)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::LCtrl));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::LShift));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x0D)));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x0D)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::LShift));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::LCtrl));
}

RED_AUTO_TEST_CASE(Test_KeyboardShortcutBlocker_7)
{
    KeyboardShortcutBlocker keyboard_shortcut_blocker(KeyLayout::KbdId(0x40c), "1"_av, true);
    RED_CHECK(keyboard_shortcut_blocker.has_shortcut());

    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x02)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x02)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode::LShift));
    // digit1
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x02)));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x02)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode::LShift));
    // numpad1
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x4F)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x4F)));
    // numlock
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x45)));
    RED_CHECK(!keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x45)));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(downnnn, Scancode(0x4F)));
    RED_CHECK( keyboard_shortcut_blocker.scancode_must_be_blocked(release, Scancode(0x4F)));
}
