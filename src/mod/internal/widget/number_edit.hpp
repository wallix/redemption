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

#include "edit.hpp"
#include "gdi/graphic_api.hpp"

class WidgetNumberEdit : public WidgetEdit
{
public:
    WidgetNumberEdit(gdi::GraphicApi & drawable, Widget & parent,
                     NotifyApi* notifier, const char* text, int group_id,
                     BGRColor fgcolor, BGRColor bgcolor, BGRColor focus_color,
                     Font const & font, size_t edit_position = -1,
                     int xtext = 0, int ytext = 0)
    : WidgetEdit(drawable, parent, notifier, text, group_id, fgcolor, bgcolor,
                 focus_color, font, edit_position, xtext, ytext)
    {}

    void set_text(const char * text) override {
        this->label.x_text = this->label.initial_x_text;
        this->WidgetEdit::set_text(text);
    }

    void insert_text(const char* text) override {
        for (const char * s = text; *s; ++s) {
            if (*s < '0' || '9' < *s) {
                return ;
            }
        }
        WidgetEdit::insert_text(text);
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
        if (keymap->nb_kevent_available() && keymap->top_kevent() == Keymap2::KEVENT_KEY){
            uint32_t c = keymap->top_char();
            if (c < '0' || '9' < c) {
                keymap->get_char();
                keymap->get_kevent();
                return ;
            }
        }
        WidgetEdit::rdp_input_scancode(param1, param2, param3, param4, keymap);
    }
};

