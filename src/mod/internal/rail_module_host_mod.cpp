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

#include "core/session_reactor.hpp"
#include "mod/internal/rail_module_host_mod.hpp"
#include "mod/internal/client_execute.hpp"
#include "configs/config.hpp"
#include "core/front_api.hpp"

RailModuleHostMod::RailModuleHostMod(
    RailModuleHostModVariables vars,
    SessionReactor& session_reactor,
    FrontAPI& front, uint16_t width, uint16_t height,
    Rect const widget_rect, std::unique_ptr<mod_api> managed_mod,
    ClientExecute& client_execute,
    const GCC::UserData::CSMonitor& cs_monitor,
    bool can_resize_hosted_desktop)
: LocallyIntegrableMod(session_reactor, front, width, height, vars.get<cfg::font>(),
                       client_execute, vars.get<cfg::theme>())
, rail_module_host(session_reactor, front, widget_rect.x, widget_rect.y,
                   widget_rect.cx, widget_rect.cy,
                   this->screen, this, std::move(managed_mod),
                   vars.get<cfg::font>(), cs_monitor, width, height)
, vars(vars)
, can_resize_hosted_desktop(can_resize_hosted_desktop)
, client_execute(client_execute)
{
    this->screen.add_widget(&this->rail_module_host);

    this->screen.set_widget_focus(&this->rail_module_host,
        Widget::focus_reason_tabkey);

    this->screen.rdp_input_invalidate(this->screen.get_rect());
}

RailModuleHost * RailModuleHostMod::get_module_host() {
    return &(this->rail_module_host);
}

// RdpInput

void RailModuleHostMod::rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
{
    Rect client_execute_auxiliary_window_rect = this->client_execute.get_auxiliary_window_rect();

    if (!client_execute_auxiliary_window_rect.isempty() &&
        client_execute_auxiliary_window_rect.contains_pt(x, y)) {
        mod_api& mod = this->rail_module_host.get_managed_mod();

        mod.rdp_input_mouse(device_flags, x, y, keymap);
    }
    else {
        LocallyIntegrableMod::rdp_input_mouse(device_flags, x, y, keymap);
    }
}

void RailModuleHostMod::rdp_input_up_and_running()
{
    mod_api& mod = this->rail_module_host.get_managed_mod();

    mod.rdp_input_up_and_running();
}

// Callback

void RailModuleHostMod::send_to_mod_channel(
    CHANNELS::ChannelNameId front_channel_name,
    InStream& chunk, size_t length, uint32_t flags)
{
    LocallyIntegrableMod::send_to_mod_channel(front_channel_name, chunk,
        length, flags);

    if (front_channel_name == channel_names::rail) { return; }

    mod_api& mod = this->rail_module_host.get_managed_mod();

    mod.send_to_mod_channel(front_channel_name, chunk, length, flags);
}

void RailModuleHostMod::send_auth_channel_data(const char * string_data)
{
    mod_api& mod = this->rail_module_host.get_managed_mod();

    mod.send_auth_channel_data(string_data);
}

// mod_api

void RailModuleHostMod::draw_event(time_t now, gdi::GraphicApi& gapi)
{
    LocallyIntegrableMod::draw_event(now, gapi);
}

bool RailModuleHostMod::is_up_and_running()
{
    mod_api& mod = this->rail_module_host.get_managed_mod();

    return mod.is_up_and_running();
}

void RailModuleHostMod::move_size_widget(int16_t left, int16_t top, uint16_t width,
                        uint16_t height)
{
    Dimension dim = this->get_dim();

    this->rail_module_host.move_size_widget(left, top, width, height);

    if (dim.w && dim.h && ((dim.w != width) || (dim.h != height)) &&
        this->client_execute.is_resizing_hosted_desktop_enabled()) {
        if (this->disconnection_reconnection_timer) {
            this->disconnection_reconnection_timer->set_time(std::chrono::seconds(1));
        }
        else {
            this->disconnection_reconnection_timer = this->session_reactor
            .create_timer(std::ref(*this))
            .set_delay(std::chrono::seconds(1))
            .on_action([](auto ctx, RailModuleHostMod& self){
                if (self.rail_module_host.get_managed_mod().is_auto_reconnectable()) {
                    throw Error(ERR_AUTOMATIC_RECONNECTION_REQUIRED);
                }
                return ctx.terminate();
            });
        }
    }
}

Dimension RailModuleHostMod::get_dim() const
{
    const mod_api& mod = this->rail_module_host.get_managed_mod();

    return mod.get_dim();
}

bool RailModuleHostMod::is_resizing_hosted_desktop_allowed() const
{
    return (vars.get<cfg::remote_program::allow_resize_hosted_desktop>() &&
        this->can_resize_hosted_desktop);
}
