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

#include "mod/internal/widget/composite.hpp"
#include "mod/internal/widget/module_host.hpp"
#include "core/RDP/gcc/userdata/cs_monitor.hpp"

#include <memory>

class mod_api;

class RailModuleHost : public WidgetParent
{
private:
    CompositeArray composite_array;

    WidgetModuleHost module_host;

public:
    RailModuleHost(
        SessionReactor& session_reactor, gdi::GraphicApi& drawable,
        int16_t left, int16_t top, int16_t width, int16_t height,
        Widget& parent, NotifyApi* notifier,
        std::unique_ptr<mod_api> managed_mod, Font const & font,
        const GCC::UserData::CSMonitor& cs_monitor,
        uint16_t front_width, uint16_t front_height);

    ~RailModuleHost() override;

    mod_api& get_managed_mod();

    const mod_api& get_managed_mod() const;

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height);

    // Widget

    void focus(int reason) override;

    void blur() override;
};
