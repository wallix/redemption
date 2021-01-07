/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2019
 *   Author(s): Meng Tan
 */

#include "mod/internal/transition_mod.hpp"
#include "keyboard/keymap2.hpp"

TransitionMod::TransitionMod(
    char const * message,
    EventContainer& events,
    gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, ClientExecute & rail_client_execute, Font const& font,
    Theme const& theme
)
    : RailModBase(
        events, drawable, front,
        width, height, rail_client_execute, font, theme)
    , ttmessage(drawable, this->screen, nullptr, message,
                theme.tooltip.fgcolor, theme.tooltip.bgcolor,
                theme.tooltip.border_color, font)
{
    Dimension dim = this->ttmessage.get_optimal_dim();
    this->ttmessage.set_wh(dim);
    this->ttmessage.set_xy(widget_rect.x + (widget_rect.cx - dim.w) / 2,
                           widget_rect.y + (widget_rect.cy - dim.h) / 2);
    this->ttmessage.rdp_input_invalidate(this->ttmessage.get_rect());
    this->set_mod_signal(BACK_EVENT_NONE);
}

TransitionMod::~TransitionMod() = default;

void TransitionMod::rdp_input_scancode(long int param1, long int param2,
                                       long int param3, long int param4,
                                       Keymap2* keymap)
{
    RailModBase::rdp_input_scancode(param1, param2, param3, param4, keymap);

    if (keymap->nb_kevent_available() > 0){
        switch (keymap->top_kevent()){
        case Keymap2::KEVENT_ESC:
            keymap->get_kevent();
            this->set_mod_signal(BACK_EVENT_STOP);
            break;
        default:;
        }
    }
}
