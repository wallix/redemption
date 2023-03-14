/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtclient/inputs/send_mouse_event.hpp"
#include "core/callback.hpp"
#include "cxx/cxx.hpp"


void qtclient::send_mouse_button(RdpInput& mod, uint16_t flags, Qt::MouseButton button, int x, int y)
{
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
    switch (button) {
        case Qt::LeftButton:    flags |= MOUSE_FLAG_BUTTON1; break;
        case Qt::RightButton:   flags |= MOUSE_FLAG_BUTTON2; break;
        case Qt::MiddleButton:  flags |= MOUSE_FLAG_BUTTON3; break;
        case Qt::BackButton:    flags |= MOUSE_FLAG_BUTTON4; break;
        case Qt::ForwardButton: flags |= MOUSE_FLAG_BUTTON5; break;
        default: return;
    }
    REDEMPTION_DIAGNOSTIC_POP()

    mod.rdp_input_mouse(flags, checked_int(x), checked_int(y));
}

void qtclient::send_mouse_wheel(RdpInput& mod, uint16_t whell_flag, int delta)
{
    if (delta) {
        whell_flag |= delta < 0 ? MOUSE_FLAG_WHEEL_NEGATIVE : uint16_t();
        mod.rdp_input_mouse(whell_flag, 0, 0);
    }
}
