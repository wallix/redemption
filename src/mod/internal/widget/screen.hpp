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
class NotifyApi;

class WidgetScreen : public WidgetParent
{
public:
    Theme theme;
    std::unique_ptr<WidgetTooltip> tooltip;
    Widget * current_over;

    CompositeArray composite_array;

    Font const & font;

    WidgetScreen(gdi::GraphicApi & drawable, uint16_t width, uint16_t height,
                 Font const & font, NotifyApi * notifier, Theme theme);

    ~WidgetScreen() override;

    void show_tooltip(Widget * widget, const char * text, int x, int y,
                      Rect const preferred_display_rect) override;

    bool next_focus() override;
    bool previous_focus() override;

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override;

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override;

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override;

    void allow_mouse_pointer_change(bool allow);

    void redo_mouse_pointer_change(int x, int y);

private:
    bool allow_mouse_pointer_change_ = true;
};
