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
#include "widget2/screen.hpp"
#include "core/front_api.hpp"
#include "core/channel_list.hpp"

enum {
    MODINTERNAL_LOGLEVEL_CLIENTEXECUTE = 0x00000001
};

struct InternalMod : public mod_api {
protected:
    uint16_t front_width;
    uint16_t front_height;

    FrontAPI & front;

    WidgetScreen screen;

public:
    InternalMod(FrontAPI & front, uint16_t front_width, uint16_t front_height, Font const & font,
                Theme const & theme, bool dont_resize)
        : front_width(front_width)
        , front_height(front_height)
        , front(front)
        , screen(front, font, nullptr, theme)
    {
        this->screen.set_wh(front_width, front_height);

        if (!dont_resize) {
            this->front.server_resize(front_width, front_height, 24);
        }
    }

    Font const & font() const {
        return this->screen.font;
    }

    Theme const & theme() const {
        return this->screen.theme;
    }

    ~InternalMod() override {}

    Rect get_screen_rect() const
    {
        return this->screen.get_rect();
    }

    void send_to_front_channel(CHANNELS::ChannelNameId mod_channel_name, uint8_t const * data, size_t length, size_t chunk_size,
        int flags) override {
        const CHANNELS::ChannelDef * front_channel =
            this->front.get_channel_list().get_by_name(mod_channel_name);
            LOG(LOG_INFO, "Channel \"%s\"", mod_channel_name);
        if (front_channel) {
            this->front.send_to_channel(*front_channel, data, length, chunk_size, flags);
        }
        else {
            LOG(LOG_ERR, "Channel \"%s\" is not fonud!", mod_channel_name);
        }
    }

    void rdp_input_invalidate(Rect r) override {
        this->screen.rdp_input_invalidate(r);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {
        this->screen.rdp_input_mouse(device_flags, x, y, keymap);
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
        this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override {
        (void)time;
        (void)device_flags;
        (void)param1;
        (void)param2;
    }

    void refresh(Rect clip) override {
        this->screen.refresh(clip);
    }

    Dimension get_dim() const override
    { return Dimension(this->front_width, this->front_height); }

    void allow_mouse_pointer_change(bool allow) {
        this->screen.allow_mouse_pointer_change(allow);
    }

    void redo_mouse_pointer_change() {
        this->screen.redo_mouse_pointer_change();
    }
};

