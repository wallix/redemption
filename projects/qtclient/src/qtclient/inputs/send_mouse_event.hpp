/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QtCore/Qt>

class mod_api;
class QMouseEvent;

namespace qtclient
{

void send_mouse_button(mod_api& mod, uint16_t flags, Qt::MouseButton button, uint16_t x, uint16_t y);

}
