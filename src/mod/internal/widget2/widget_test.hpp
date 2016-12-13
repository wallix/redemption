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

#include "mod/internal/widget2/composite.hpp"
#include "mod/internal/widget2/remote_desktop.hpp"
#include "gdi/graphic_api.hpp"

class WidgetTest : public WidgetParent
{
private:
    CompositeArray composite_array;

    WidgetRemoteDesktop remote_desktop;

public:
    WidgetTest(gdi::GraphicApi& drawable, int16_t left, int16_t top, int16_t width, int16_t height,
             Widget2& parent, NotifyApi* notifier)
        : WidgetParent(drawable, parent, notifier)
        , remote_desktop(drawable, *this, this)
    {
        this->impl = &composite_array;

        this->add_widget(&this->remote_desktop);

        this->move_size_widget(left, top, width, height);
    }

    ~WidgetTest() override {
        this->clear();
    }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) {
        this->set_xy(left, top);
        this->set_wh(width, height);

        this->remote_desktop.set_xy(left, top);
        this->remote_desktop.set_wh(width, height);
    }
};
