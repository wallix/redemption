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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_NUMBER_EDIT_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_NUMBER_EDIT_HPP

#include "widget2_edit.hpp"

class WidgetNumberEdit : public WidgetEdit
{
public:
    WidgetNumberEdit(DrawApi* drawable, int16_t x, int16_t y, uint16_t cx, Widget2 * parent,
                     NotifyApi* notifier, const char* text, int group_id = 0,
                     int fgcolor = BLACK, int bgcolor = WHITE, size_t
                     edit_position = -1, int xtext = 0, int ytext = 0)
    : WidgetEdit(drawable, x, y, cx, parent, notifier, text, group_id, fgcolor, bgcolor, edit_position, xtext, ytext)
    {}

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
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

#endif
