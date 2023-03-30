/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/mod/accumulate_input_mod.hpp"

#include "keyboard/keymap.hpp"
#include "headlessclient/input_collector.hpp"


RED_AUTO_TEST_CASE(TestExecuteCommand)
{
    AccumulateInputMod mod;
    InputCollector input_collector;
    Keymap keymap(KeyLayout::null_layout());
    using ConsumedInput = InputCollector::ConsumedInput;
    using KbdFlags = kbdtypes::KbdFlags;
    using KeyLocks = kbdtypes::KeyLocks;
    using Scancode = kbdtypes::Scancode;

#define CHECK_NEXT_INPUT(next_input, msg)                                  \
    RED_CHECK(input_collector.send_next_input(mod, keymap) == next_input); \
    RED_CHECK(mod.events() == msg ""_av)

    RED_CHECK(!input_collector.has_input());
    CHECK_NEXT_INPUT(ConsumedInput::None, ""_av);

    input_collector.rdp_input_scancode(KbdFlags::NoFlags, Scancode::Q, 0, keymap);
    RED_CHECK(input_collector.has_input());
    CHECK_NEXT_INPUT(ConsumedInput::None, "{KbdFlags=0x0000, Scancode=0x10}");
    RED_CHECK(!input_collector.has_input());

    input_collector.rdp_input_unicode(KbdFlags::NoFlags, 'a');
    CHECK_NEXT_INPUT(ConsumedInput::None, "{KbdFlags=0x0000, Unicode=0x0061}");

    input_collector.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 1, 3);
    CHECK_NEXT_INPUT(ConsumedInput::None, "{flags=0x1000, x=1, y=3}");

    input_collector.rdp_input_synchronize(KeyLocks::NumLock);
    CHECK_NEXT_INPUT(ConsumedInput::None, "{KeyLocks=0x02}");

    RED_CHECK(!input_collector.has_input());
    input_collector.rdp_input_scancode(KbdFlags::NoFlags, Scancode::Q, 0, keymap);
    input_collector.rdp_input_unicode(KbdFlags::NoFlags, 'a');
    input_collector.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 1, 3);
    input_collector.rdp_input_synchronize(KeyLocks::NumLock);

    RED_CHECK(input_collector.has_input());
    CHECK_NEXT_INPUT(ConsumedInput::KeyEvent, "{KbdFlags=0x0000, Scancode=0x10}");
    CHECK_NEXT_INPUT(ConsumedInput::KeyEvent, "{KbdFlags=0x0000, Unicode=0x0061}");
    CHECK_NEXT_INPUT(ConsumedInput::MouseEvent, "{flags=0x1000, x=1, y=3}");
    CHECK_NEXT_INPUT(ConsumedInput::None, "{KeyLocks=0x02}");
    RED_CHECK(!input_collector.has_input());

#undef check_next_input
}
