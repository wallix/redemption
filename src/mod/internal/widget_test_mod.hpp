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

#include "mod/internal/internal_mod.hpp"
#include "mod/internal/locally_integrable_mod.hpp"
#include "mod/internal/widget2/notify_api.hpp"
#include "mod/internal/widget2/remote_desktop.hpp"
#include "mod/internal/widget2/widget_test.hpp"

#include "configs/config_access.hpp"

using WidgetTestModVariables = vcfg::variables<
    vcfg::var<cfg::translation::language,   vcfg::accessmode::get>,
    vcfg::var<cfg::font,                    vcfg::accessmode::get>,
    vcfg::var<cfg::theme,                   vcfg::accessmode::get>
>;

class WidgetTestMod : public LocallyIntegrableMod, public NotifyApi {
    WidgetTest widget_test;

    WidgetTestModVariables vars;

public:
    WidgetTestMod(
        WidgetTestModVariables vars,
        FrontAPI& front, uint16_t width, uint16_t height, Rect const& widget_rect,
        ClientExecute& client_execute)
    : LocallyIntegrableMod(front, width, height, vars.get<cfg::font>(), client_execute, vars.get<cfg::theme>())
    , widget_test(front, widget_rect.x, widget_rect.y, widget_rect.cx + 1, widget_rect.cy + 1,
                  this->screen, this)
    , vars(vars)
    {
        this->screen.add_widget(&this->widget_test);

        this->screen.set_widget_focus(&this->widget_test, Widget2::focus_reason_tabkey);

        this->screen.refresh(this->screen.get_rect());
    }

    ~WidgetTestMod() override
    {
        this->screen.clear();
    }

    void notify(Widget2*, notify_event_t) override {}

public:
    void draw_event(time_t now, gdi::GraphicApi& gapi) override
    {
        LocallyIntegrableMod::draw_event(now, gapi);
    }

    bool is_up_and_running() override { return true; }

    void send_to_mod_channel(const char * front_channel_name, InStream& chunk, size_t length, uint32_t flags) override {
        LocallyIntegrableMod::send_to_mod_channel(front_channel_name, chunk, length, flags);
    }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override {
        this->widget_test.move_size_widget(left, top, width + 1, height + 1);
    }
};
