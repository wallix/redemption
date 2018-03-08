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

#include "mod/internal/widget/rail_module_host.hpp"

RailModuleHost::RailModuleHost(
    gdi::GraphicApi& drawable,
    int16_t left, int16_t top, int16_t width, int16_t height,
    Widget& parent, NotifyApi* notifier,
    std::unique_ptr<mod_api> managed_mod, Font const & font,
    const GCC::UserData::CSMonitor& cs_monitor,
    uint16_t front_width, uint16_t front_height
)
: WidgetParent(drawable, parent, notifier)
, module_host(drawable, *this, this, std::move(managed_mod),
              font, cs_monitor, front_width, front_height)
{
    this->impl = &composite_array;

    this->add_widget(&this->module_host);

    this->move_size_widget(left, top, width, height);
}

RailModuleHost::~RailModuleHost()
{
    this->clear();
}

mod_api& RailModuleHost::get_managed_mod()
{
    return this->module_host.get_managed_mod();
}

const mod_api& RailModuleHost::get_managed_mod() const
{
    return this->module_host.get_managed_mod();
}

void RailModuleHost::move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height)
{
    this->set_xy(left, top);
    this->set_wh(width, height);

    this->module_host.set_xy(left, top);
    this->module_host.set_wh(width, height);
}

// Widget

void RailModuleHost::focus(int reason)
{
    this->module_host.focus(reason);
    Widget::focus(reason);
}

void RailModuleHost::blur()
{
    this->module_host.blur();
    Widget::blur();
}
