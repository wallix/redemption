/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QtCore/Qt>

class RdpInput;
class QMouseEvent;

namespace qtclient
{

void send_mouse_button(RdpInput& mod, uint16_t flags, Qt::MouseButton button, int x, int y);
void send_mouse_wheel(RdpInput& mod, uint16_t whell_flag, int delta);

}
