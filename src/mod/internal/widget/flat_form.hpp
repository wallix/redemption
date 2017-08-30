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
#include "mod/internal/widget/flat_button.hpp"
#include "mod/internal/widget/edit.hpp"
#include "utils/translation.hpp"

class Theme;

class FlatForm : public WidgetParent
{
public:
    CompositeArray composite_array;

    WidgetLabel      warning_msg;
    WidgetLabel      duration_label;
    WidgetEdit       duration_edit;
    WidgetLabel      duration_format;
    WidgetLabel      ticket_label;
    WidgetEdit       ticket_edit;
    WidgetLabel      comment_label;
    WidgetEdit       comment_edit;
    WidgetLabel      notes;
    WidgetFlatButton confirm;

    int flags;
    int duration_max;

    enum {
        NONE               = 0x00,
        COMMENT_DISPLAY    = 0x01,
        COMMENT_MANDATORY  = 0x02,
        TICKET_DISPLAY     = 0x04,
        TICKET_MANDATORY   = 0x08,
        DURATION_DISPLAY   = 0x10,
        DURATION_MANDATORY = 0x20,
    };

    const char * generic_warning;
    const char * format_warning;
    const char * toohigh_warning;
    const char * field_comment;
    const char * field_ticket;
    const char * field_duration;

    char warning_buffer[512];

    FlatForm(gdi::GraphicApi& drawable, int16_t left, int16_t top, int16_t width, int16_t height,
             Widget & parent, NotifyApi* notifier, int group_id,
             Font const & font, Theme const & theme, Translation::language_t lang,
             int flags = 0, int duration_max = 0);

    FlatForm(gdi::GraphicApi& drawable,
             Widget & parent, NotifyApi* notifier, int group_id,
             Font const & font, Theme const & theme, Translation::language_t lang,
             int flags = 0, int duration_max = 0);

    ~FlatForm() override;

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height);

    void notify(Widget* widget, NotifyApi::notify_event_t event) override;

    void set_warning_buffer(const char * field, const char * format);

    void set_warning_buffer_duration(const char * field, int duration,
                                     const char * format);


    unsigned long check_duration(const char * duration);

    void check_confirmation();

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override;
};
