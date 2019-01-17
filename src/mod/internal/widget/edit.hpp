/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1 of the License, or
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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#pragma once

#include "mod/internal/widget/widget.hpp"
#include "mod/internal/widget/label.hpp"

class WidgetEdit : public Widget
{
public:
    WidgetLabel label;
    size_t buffer_size;
    size_t num_chars;
    size_t edit_buffer_pos;
    size_t edit_pos;
    size_t cursor_px_pos;
    int w_text;
    int h_text;
    BGRColor cursor_color;
    BGRColor focus_color;
    bool drawall;
    bool draw_border_focus;
    Font const & font;

    WidgetEdit(gdi::GraphicApi & drawable,
               Widget & parent, NotifyApi* notifier, const char * text,
               int group_id, BGRColor fgcolor, BGRColor bgcolor, BGRColor focus_color,
               Font const & font, std::size_t edit_position = -1, int xtext = 0, int ytext = 0);

    WidgetEdit(WidgetEdit const & other);

    ~WidgetEdit() override;

    Dimension get_optimal_dim() override;

    virtual void set_text(const char * text/*, int position = 0*/);

    virtual void insert_text(const char * text/*, int position = 0*/);

    const char * get_text() const;

    void set_xy(int16_t x, int16_t y) override;

    void set_wh(uint16_t w, uint16_t h) override;

    using Widget::set_wh;

    void rdp_input_invalidate(Rect clip) override;

    void draw_border(Rect clip, BGRColor color);

    virtual Rect get_cursor_rect() const;

    void draw_current_cursor();
    void draw_cursor(const Rect clip);

    void increment_edit_pos();

    size_t utf8len_current_char();

    void decrement_edit_pos();

    virtual void update_draw_cursor(Rect old_cursor);

    void move_to_last_character();

    void move_to_first_character();

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override;

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override;

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override;
};
