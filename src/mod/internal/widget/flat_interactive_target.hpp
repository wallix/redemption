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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
 *              Jennifer Inthavong
 */

#pragma once

#include "mod/internal/widget/composite.hpp"
#include "mod/internal/widget/label.hpp"
#include "mod/internal/widget/edit.hpp"
#include "mod/internal/widget/edit_valid.hpp"
#include "mod/internal/widget/widget_rect.hpp"

class Theme;
class WidgetFlatButton;

class FlatInteractiveTarget : public WidgetParent
{
public:
    // ASK DEVICE YES/NO
    // ASK CRED : LOGIN+PASSWORD/PASSWORD/NO

    FlatInteractiveTarget(gdi::GraphicApi & drawable, int16_t left, int16_t top, uint16_t width, uint16_t height,
                          Widget & parent, NotifyApi* notifier,
                          bool ask_device, bool ask_login, bool ask_password,
                          Theme const & theme, const char* caption,
                          const char * text_device,
                          const char * device_str,
                          const char * text_login,
                          const char * login_str,
                          const char * text_password,
                          Font const & font,
                          WidgetFlatButton * extra_button);

    ~FlatInteractiveTarget() override;

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height);

    BGRColor get_bg_color() const override;

    void notify(Widget* widget, NotifyApi::notify_event_t event) override;

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override;

private:
    CompositeArray composite_array;

public:
    WidgetLabel        caption_label;
    WidgetRect         separator;
    WidgetLabel        device_label;
    WidgetLabel        device;
    WidgetEditValid    device_edit;
    WidgetLabel        login_label;
    WidgetLabel        login;
    WidgetEditValid    login_edit;
    WidgetLabel        password_label;
    WidgetEditValid    password_edit;
    WidgetFlatButton * extra_button;

    Widget * last_interactive;

    BGRColor fgcolor;
    BGRColor bgcolor;

    bool               ask_device;
    bool               ask_login;
    bool               ask_password;
};
