/*
SPDX-FileCopyrightText: 2022 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "mod/internal/widget/widget.hpp"

struct NullTooltipShower : WidgetTooltipShower
{
    void show_tooltip(const char * text, int x, int y, Rect preferred_display_rect, Rect mouse_area) override
    {
        (void)text;
        (void)x;
        (void)y;
        (void)preferred_display_rect;
        (void)mouse_area;
    }
};
