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

#include "mod/internal/widget/widget.hpp"
#include "gdi/graphic_api.hpp"

class WidgetFlatButton : public Widget
{
private:
    static const size_t buffer_size = 256;

    char buffer[buffer_size];

    bool auto_resize_;

    int x_text;
    int y_text;

    unsigned border_width;

public:
    int state;
    notify_event_t event;
    BGRColor fg_color;
    BGRColor bg_color;
    BGRColor focus_color;
    bool logo;

    Font const & font;

    Rect label_rect;

    WidgetFlatButton(gdi::GraphicApi & drawable, Widget& parent,
                     NotifyApi* notifier, const char * text,
                     int group_id, BGRColor fgcolor, BGRColor bgcolor, BGRColor focuscolor,
                     unsigned border_width, Font const & font, int xtext = 0, int ytext = 0,
                     bool logo = false/*, notify_event_t notify_event = NOTIFY_SUBMIT*/);

    ~WidgetFlatButton() override;

    void set_xy(int16_t x, int16_t y) override;

    void set_wh(uint16_t w, uint16_t h) override;

    using Widget::set_wh;

    void set_text(char const* text);

    void rdp_input_invalidate(Rect clip) override;

    static void draw(Rect const clip, Rect const rect, gdi::GraphicApi& drawable,
                     bool logo, bool has_focus, char const* text,
                     RDPColor fg_color, RDPColor bg_color, RDPColor focuscolor, gdi::ColorCtx color_ctx,
                     Rect label_rect, int state, unsigned border_width, Font const& font, int xtext, int ytext);

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override;

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override;

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override;

    Dimension get_optimal_dim() override;

    static Dimension get_optimal_dim(unsigned border_width, Font const& font, char const* text, int xtext = 0, int ytext = 0);
};
