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
class WidgetButton;

class WidgetInteractiveTarget : public WidgetComposite
{
public:
    // ASK DEVICE YES/NO
    // ASK CRED : LOGIN+PASSWORD/PASSWORD/NO

    struct Events
    {
        WidgetEventNotifier onsubmit;
        WidgetEventNotifier oncancel;
        WidgetEventNotifier onctrl_shift;
    };

    WidgetInteractiveTarget(
        gdi::GraphicApi & drawable, CopyPaste & copy_paste,
        int16_t left, int16_t top, uint16_t width, uint16_t height,
        Events events,
        bool ask_device, bool ask_login, bool ask_password,
        Theme const & theme, const char * caption,
        const char * text_device,
        const char * device_str,
        const char * text_login,
        const char * login_str,
        const char * text_password,
        Font const & font,
        WidgetButton * extra_button);

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height);

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override;

private:
    WidgetEventNotifier oncancel;
    WidgetEventNotifier onctrl_shift;

    WidgetLabel        caption_label;
    WidgetRect         separator;
    WidgetLabel        device_label;
    WidgetLabel        device;
public:
    WidgetEditValid    device_edit;
private:
    WidgetLabel        login_label;
    WidgetLabel        login;
public:
    WidgetEditValid    login_edit;
private:
    WidgetLabel        password_label;
public:
    WidgetEditValid    password_edit;
private:
    WidgetButton * extra_button;

    Color fgcolor;

    bool ask_device;
    bool ask_login;
    bool ask_password;
};
