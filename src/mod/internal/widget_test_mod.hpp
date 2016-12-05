/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Meng Tan, Jonathan Poelen, Raphael Zhou
*/


#pragma once

#include "internal_mod.hpp"

#include "widget2/flat_button.hpp"

class WidgetTestMod : public InternalMod, public NotifyApi {
public:
    WidgetTestMod(FrontAPI & front, uint16_t width, uint16_t height, Font const & font, Theme const & theme)
    : InternalMod(front, width, height, font, theme) {
        this->screen.refresh(this->screen.get_rect());
    }

    ~WidgetTestMod() override {
        this->screen.clear();
    }

    void notify(Widget2 *, notify_event_t) override {}

public:
    void draw_event(time_t now, gdi::GraphicApi &) override {
        (void)now;
        this->event.reset();
    }

    bool is_up_and_running() override { return true; }
};
