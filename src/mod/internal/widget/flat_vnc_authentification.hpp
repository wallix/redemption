/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou
 */

#pragma once

#include "mod/internal/widget/label.hpp"
#include "mod/internal/widget/edit_valid.hpp"
#include "mod/internal/widget/image.hpp"
#include "mod/internal/widget/composite.hpp"

class Theme;

class FlatVNCAuthentification : public WidgetParent
{
public:
    WidgetLabel     message_label;
    WidgetLabel     password_label;
    WidgetEditValid password_edit;
    WidgetImage     img;

    BGRColor fgcolor;
    BGRColor bgcolor;

    CompositeArray composite_array;

    FlatVNCAuthentification(gdi::GraphicApi & drawable, uint16_t width, uint16_t height,
                            Widget & parent, NotifyApi* notifier, const char * password,
                            Theme const & theme, const char * label_text_message,
                            const char * label_text_password, Font const & font);

    ~FlatVNCAuthentification() override;

    BGRColor get_bg_color() const override;

    void notify(Widget* widget, NotifyApi::notify_event_t event) override;

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override;
};
