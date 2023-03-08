/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "qtclient/inputs/send_mouse_event.hpp"
#include "mod/mod_api.hpp"
#include "cxx/cxx.hpp"

void qtclient::send_mouse_button(mod_api& mod, uint16_t flags, Qt::MouseButton button, uint16_t x, uint16_t y)
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

    mod.rdp_input_mouse(flags, x, y);
}
