/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen, Meng Tan
 *
 */


#pragma once

#include "mod/internal/rail_mod_base.hpp"
#include "mod/internal/widget/flat_wab_close.hpp"
#include "configs/config_access.hpp"
#include "core/events.hpp"

namespace gdi
{
    class GraphicApi;
}

using CloseModVariables = vcfg::variables<
    vcfg::var<cfg::globals::auth_user, vcfg::accessmode::get | vcfg::accessmode::is_asked>,
    vcfg::var<cfg::globals::target_device, vcfg::accessmode::get | vcfg::accessmode::ask | vcfg::accessmode::is_asked>,
    vcfg::var<cfg::globals::target_user, vcfg::accessmode::get | vcfg::accessmode::ask>,
    vcfg::var<cfg::context::selector, vcfg::accessmode::ask>,
    vcfg::var<cfg::context::target_protocol, vcfg::accessmode::ask>,
    vcfg::var<cfg::globals::close_timeout, vcfg::accessmode::get>,
    vcfg::var<cfg::globals::target_application, vcfg::accessmode::get>,
    vcfg::var<cfg::context::module, vcfg::accessmode::get>,
    vcfg::var<cfg::translation::language, vcfg::accessmode::get>,
    vcfg::var<cfg::context::close_box_extra_message, vcfg::accessmode::get | vcfg::accessmode::set>
>;

class CloseMod : public RailModBase, public NotifyApi
{
    FlatWabClose close_widget;
    CloseModVariables vars;

public:
    CloseMod(
        char const* auth_error_message,
        CloseModVariables vars,
        EventContainer& events,
        gdi::GraphicApi & gd, FrontAPI & front, uint16_t width, uint16_t height,
        Rect const widget_rect, ClientExecute & rail_client_execute, Font const& font,
        Theme const& theme, bool back_selector);

    ~CloseMod() override;

    void notify(Widget* sender, notify_event_t event) override;

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override
    {
        this->close_widget.move_size_widget(left, top, width, height);
    }

private:
    EventsGuard events_guard;
};
