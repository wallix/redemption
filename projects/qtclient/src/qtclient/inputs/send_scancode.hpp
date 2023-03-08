/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "keyboard/kbdtypes.hpp"

class mod_api;
class Keymap;

namespace qtclient
{

void x11_send_scancode(mod_api& mod, kbdtypes::KbdFlags flag, Keymap const& keymap, uint32_t native_scancode);

void win_send_scancode(mod_api& mod, kbdtypes::KbdFlags flag, Keymap const& keymap, uint32_t native_scancode);

}
