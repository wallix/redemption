/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

class mod_api;
class Keymap;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace qtclient
{

void send_mouse_move(mod_api& mod, QMouseEvent& event);
void send_mouse_wheel(mod_api& mod, QWheelEvent& event);
void send_mouse_press(mod_api& mod, QMouseEvent& event);
void send_mouse_release(mod_api& mod, QMouseEvent& event);
void send_key_release(mod_api& mod, QKeyEvent& event, Keymap const& keymap);
void send_key_press(mod_api& mod, QKeyEvent& event, Keymap const& keymap);

}
