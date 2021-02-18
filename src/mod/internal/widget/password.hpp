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

class WidgetPassword : public WidgetEdit
{
public:
    WidgetLabel masked_text;

    int w_char;
    int h_char;

    WidgetPassword(gdi::GraphicApi & drawable,
                   Widget& parent, NotifyApi* notifier, const char * text,
                   int group_id, BGRColor fgcolor, BGRColor bgcolor, BGRColor focus_color, Font const & font,
                   std::size_t edit_position = -1, int xtext = 0, int ytext = 0); /*NOLINT*/

    Dimension get_optimal_dim() override;

    void set_masked_text();

    void set_xy(int16_t x, int16_t y) override;

    void set_wh(uint16_t w, uint16_t h) override;

    using WidgetEdit::set_wh;

    void set_text(const char * text) override;

    void insert_text(const char* text) override;

    void rdp_input_invalidate(Rect clip) override;
    void update_draw_cursor(Rect old_cursor) override;


    [[nodiscard]] Rect get_cursor_rect() const override;

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override;

    void rdp_input_scancode(long int param1, long int param2, long int param3,
                                    long int param4, Keymap2* keymap) override;

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override;

    void clipboard_copy(CopyPaste& copy_paste) override;
    void clipboard_cut(CopyPaste& copy_paste) override;
};
