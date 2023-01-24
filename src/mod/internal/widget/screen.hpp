/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#pragma once

#include "mod/internal/widget/composite.hpp"
#include "utils/theme.hpp"

#include <memory>


class WidgetTooltip;
class Font;

class WidgetScreen : public WidgetComposite, public WidgetTooltipShower
{
public:
    WidgetScreen(gdi::GraphicApi & drawable, uint16_t width, uint16_t height,
                 Font const & font, Theme theme);

    ~WidgetScreen() override;

    void show_tooltip(const char * text, int x, int y,
                      Rect preferred_display_rect,
                      Rect mouse_area) override;

    bool next_focus() override;
    bool previous_focus() override;

    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override;

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override;

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override;

    void allow_mouse_pointer_change(bool allow);

    void redo_mouse_pointer_change(int x, int y);

public:
    Theme theme;

    std::unique_ptr<WidgetTooltip> tooltip;
private:
    Rect tooltip_mouse_area;

    Widget * current_over;

public:
    Font const & font;

private:
    bool allow_mouse_pointer_change_ = true;
};
