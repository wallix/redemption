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

#include "mod/internal/rail_module_host_mod.hpp"

#include "utils/timebase.hpp"
#include "mod/null/null.hpp"
#include "RAIL/client_execute.hpp"
#include "keyboard/keymap.hpp"


using namespace std::chrono_literals;

RailModuleHostMod::RailModuleHostMod(
    EventContainer& events,
    gdi::GraphicApi & drawable,
    uint16_t width, uint16_t height,
    Rect const widget_rect, ClientExecute& rail_client_execute,
    Font const& font, Theme const& theme,
    const GCC::UserData::CSMonitor& cs_monitor
)
    : RailModBase(drawable, width, height, rail_client_execute, font, theme)
    , disconnection_reconnection_timer(events)
    , managed_mod(std::make_unique<null_mod>())
    , module_host(drawable, this->screen,
                  *this->managed_mod, font, cs_monitor,
                  widget_rect, width, height)
{
    this->screen.set_xy(widget_rect.x, widget_rect.y);
    this->screen.set_wh(width, height);
    this->screen.add_widget(this->module_host, WidgetComposite::HasFocus::Yes);
    this->screen.init_focus();
}

void RailModuleHostMod::set_mod(std::unique_ptr<mod_api>&& managed_mod) noexcept
{
    this->managed_mod = std::move(managed_mod);
    this->module_host.set_mod(*this->managed_mod);
}

void RailModuleHostMod::rdp_gdi_up_and_running()
{
    mod_api& mod = this->module_host.get_managed_mod();
    mod.rdp_gdi_up_and_running();
}

void RailModuleHostMod::rdp_input_synchronize(KeyLocks locks)
{
    this->managed_mod->rdp_input_synchronize(locks);
}

void RailModuleHostMod::rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y)
{
    Rect client_execute_auxiliary_window_rect = this->rail_client_execute.get_auxiliary_window_rect();

    if (!client_execute_auxiliary_window_rect.isempty()
     && client_execute_auxiliary_window_rect.contains_pt(x, y)
    ) {
        mod_api& mod = this->module_host.get_managed_mod();

        mod.rdp_input_mouse(device_flags, x, y);
    }
    else {
        RailModBase::rdp_input_mouse(device_flags, x, y);
    }
}

// Callback

void RailModuleHostMod::send_to_mod_channel(
    CHANNELS::ChannelNameId front_channel_name,
    InStream& chunk, size_t length, uint32_t flags)
{
    if (front_channel_name != CHANNELS::channel_names::rail) {
        mod_api& mod = this->module_host.get_managed_mod();
        mod.send_to_mod_channel(front_channel_name, chunk, length, flags);
    }
    else {
        RailModBase::send_to_mod_channel(front_channel_name, chunk, length, flags);
    }
}

// mod_api

bool RailModuleHostMod::is_up_and_running() const
{
    return this->module_host.get_managed_mod().is_up_and_running();
}

bool RailModuleHostMod::is_auto_reconnectable() const
{
    return this->module_host.get_managed_mod().is_auto_reconnectable();
}

bool RailModuleHostMod::server_error_encountered() const
{
    return this->module_host.get_managed_mod().server_error_encountered();
}

void RailModuleHostMod::move_size_widget(int16_t left, int16_t top, uint16_t width,
                        uint16_t height)
{
    Dimension dim = this->get_dim();

    this->module_host.set_xy(left, top);
    this->module_host.set_wh(width, height);

    if (dim.w && dim.h && ((dim.w != width) || (dim.h != height)) &&
        this->rail_client_execute.is_resizing_hosted_desktop_enabled()
    ) {
        this->disconnection_reconnection_timer.set_timeout_or_create_event(
            1s, "RAIL Module Host Disconnection Reconnection Timeout",
            [this](Event&)
            {
                // TODO there is a channel to resize the session, we should use it
                if (this->module_host.get_managed_mod().is_auto_reconnectable()){
                    throw Error(ERR_RAIL_RESIZING_REQUIRED);
                }
            }
        );
    }
}

Dimension RailModuleHostMod::get_dim() const
{
    const mod_api& mod = this->module_host.get_managed_mod();

    return mod.get_dim();
}

void RailModuleHostMod::acl_update(AclFieldMask const& acl_fields)
{
    return this->module_host.get_managed_mod().acl_update(acl_fields);
}
