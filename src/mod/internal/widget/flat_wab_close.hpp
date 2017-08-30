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

#include "mod/internal/widget/composite.hpp"
#include "mod/internal/widget/widget_rect.hpp"
#include "mod/internal/widget/label.hpp"
#include "mod/internal/widget/multiline.hpp"
#include "mod/internal/widget/image.hpp"
#include "mod/internal/widget/flat_button.hpp"
#include "utils/translation.hpp"

class Theme;

class FlatWabClose : public WidgetParent
{
public:
    CompositeArray composite_array;

    WidgetLabel        connection_closed_label;
    WidgetRect         separator;

    WidgetLabel        username_label;
    WidgetLabel        username_value;
    WidgetLabel        target_label;
    WidgetLabel        target_value;
    WidgetLabel        diagnostic_label;
    WidgetMultiLine    diagnostic_value;
    WidgetLabel        timeleft_label;
    WidgetLabel        timeleft_value;

    WidgetFlatButton   cancel;
    WidgetFlatButton * back;

    WidgetImage        img;

private:
    BGRColor bg_color;

    long prev_time;

    Translation::language_t lang;

    bool showtimer;

public:
    FlatWabClose(gdi::GraphicApi & drawable,
                 int16_t left, int16_t top, int16_t width, int16_t height, Widget& parent,
                 NotifyApi* notifier, const char * diagnostic_text,
                 const char * username, const char * target,
                 bool showtimer, Font const & font, Theme const & theme,
                 Translation::language_t lang, bool back_selector = false);

    ~FlatWabClose() override;

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height);

    BGRColor get_bg_color() const override;

    void refresh_timeleft(long tl);

    void notify(Widget * widget, NotifyApi::notify_event_t event) override;

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override;
};
