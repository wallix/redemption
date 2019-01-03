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
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen
 */

#pragma once

#include "mod/mod_api.hpp"
#include "mod/internal/widget/screen.hpp"

class FrontAPI;

struct InternalMod : public mod_api
{
protected:
    uint16_t front_width;
    uint16_t front_height;

    FrontAPI & front;

    WidgetScreen screen;

public:
    InternalMod(
        FrontAPI & front, uint16_t front_width, uint16_t front_height,
        Font const & font, Theme const & theme);

    Font const & font() const
    {
        return this->screen.font;
    }

    Theme const & theme() const
    {
        return this->screen.theme;
    }

    Rect get_screen_rect() const
    {
        return this->screen.get_rect();
    }

    void rdp_input_invalidate(Rect r) override
    {
        this->screen.rdp_input_invalidate(r);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override
    {
        this->screen.rdp_input_mouse(device_flags, x, y, keymap);
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override
    {
        this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override
    {
        this->screen.rdp_input_unicode(unicode, flag);
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        (void)time;
        (void)device_flags;
        (void)param1;
        (void)param2;
    }

    void refresh(Rect clip) override
    {
        this->screen.refresh(clip);
    }

    Dimension get_dim() const override
    { return Dimension(this->front_width, this->front_height); }

    void allow_mouse_pointer_change(bool allow) {
        this->screen.allow_mouse_pointer_change(allow);
    }

    void redo_mouse_pointer_change(int x, int y) {
        this->screen.redo_mouse_pointer_change(x, y);
    }
};

