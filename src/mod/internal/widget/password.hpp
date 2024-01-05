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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#pragma once

#include "mod/internal/widget/label.hpp"
#include "mod/internal/widget/edit.hpp"
#include "core/font.hpp"

struct WidgetPasswordFont
{
    Font shadow_font;

    WidgetPasswordFont(Font const& font);
};

class WidgetPassword : WidgetPasswordFont, public WidgetEdit
{
public:
    WidgetPassword(gdi::GraphicApi & drawable, CopyPaste & copy_paste,
                   const char * text, WidgetEventNotifier onsubmit,
                   Color fgcolor, Color bgcolor, Color focus_color,
                   Font const & font,
                   std::size_t edit_position = -1, int xtext = 0, int ytext = 0); /*NOLINT*/

    void toggle_password_visibility();

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override;

    bool password_is_visible() const
    {
        return is_password_visible;
    }

private:
    bool is_password_visible = false;
    Font const& font;
};
