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

#include "configs/config.hpp"
#include "utils/timebase.hpp"
#include "mod/internal/rail_module_host_mod.hpp"
#include "RAIL/client_execute.hpp"
#include "core/front_api.hpp"
#include "core/RDP/slowpath.hpp"


void RailModuleHostMod::rdp_input_invalidate(Rect r)
{
    this->screen.rdp_input_invalidate(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

void RailModuleHostMod::rdp_input_scancode(
    long param1, long param2, long param3, long param4, Keymap2 * keymap)
{
    this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);

    if (this->rail_enabled) {
        if (!this->alt_key_pressed) {
            if ((param1 == 56) && !(param3 & SlowPath::KBDFLAGS_RELEASE)) {
                this->alt_key_pressed = true;
            }
        }
        else {
//            if ((param1 == 56) && (param3 == (SlowPath::KBDFLAGS_DOWN | SlowPath::KBDFLAGS_RELEASE))) {
            if ((param1 == 56) && (param3 & SlowPath::KBDFLAGS_RELEASE)) {
                this->alt_key_pressed = false;
            }
            else if ((param1 == 62) && !param3) {
                LOG(LOG_INFO, "RailModuleHostMod::rdp_input_scancode: Close by user (Alt+F4)");
                throw Error(ERR_WIDGET);    // F4 key pressed
            }
        }
    }
}

void RailModuleHostMod::refresh(Rect r)
{
    this->screen.refresh(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

RailModuleHostMod::RailModuleHostMod(
    RailModuleHostModVariables vars,
    TimeBase& time_base,
    EventContainer& events,
    gdi::GraphicApi & drawable, FrontAPI& front, uint16_t width, uint16_t height,
    Rect const widget_rect, std::unique_ptr<mod_api> managed_mod,
    ClientExecute& rail_client_execute, Font const& font, Theme const& theme,
    const GCC::UserData::CSMonitor& cs_monitor, bool can_resize_hosted_desktop)
    : front_width(width)
    , front_height(height)
    , front(front)
    , screen(drawable, width, height, font, nullptr, theme)
    , rail_client_execute(rail_client_execute)
    , dvc_manager(false)
    , mouse_state(time_base, events)
    , rail_enabled(rail_client_execute.is_rail_enabled())
    , current_mouse_owner(MouseOwner::WidgetModule)
    , time_base(time_base)
    , events(events)
    , rail_module_host(drawable, widget_rect.x, widget_rect.y,
                       widget_rect.cx, widget_rect.cy,
                       this->screen, this, std::move(managed_mod),
                       font, cs_monitor, width, height)
    , vars(vars)
    , can_resize_hosted_desktop(can_resize_hosted_desktop)
{
    this->screen.set_wh(width, height);
    this->screen.move_xy(widget_rect.x, widget_rect.y);
    this->screen.add_widget(&this->rail_module_host);
    this->screen.set_widget_focus(&this->rail_module_host, Widget::focus_reason_tabkey);
    this->screen.rdp_input_invalidate(this->screen.get_rect());

    this->vars.set<cfg::context::rail_module_host_mod_is_active>(true);
}

void RailModuleHostMod::init()
{
    if (this->rail_enabled && !this->rail_client_execute.is_ready()) {
        this->rail_client_execute.ready(
                    *this, this->front_width, this->front_height,
                    this->font(), this->is_resizing_hosted_desktop_allowed());
        this->dvc_manager.ready(this->front);
    }
}


RailModuleHost& RailModuleHostMod::get_module_host()
{
    return this->rail_module_host;
}

// RdpInput
void RailModuleHostMod::rdp_gdi_up_and_running()
{
    mod_api& mod = this->rail_module_host.get_managed_mod();
    mod.rdp_gdi_up_and_running();
}

void RailModuleHostMod::rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
{
    Rect client_execute_auxiliary_window_rect = this->rail_client_execute.get_auxiliary_window_rect();

    if (!client_execute_auxiliary_window_rect.isempty() &&
        client_execute_auxiliary_window_rect.contains_pt(x, y)) {
        mod_api& mod = this->rail_module_host.get_managed_mod();

        mod.rdp_input_mouse(device_flags, x, y, keymap);
    }
    else {
        if (device_flags & (MOUSE_FLAG_WHEEL | MOUSE_FLAG_HWHEEL)) {
            x = this->old_mouse_x;
            y = this->old_mouse_y;
        }
        else {
            this->old_mouse_x = x;
            this->old_mouse_y = y;
        }

        if (!this->rail_enabled) {
            this->screen.rdp_input_mouse(device_flags, x, y, keymap);
            return;
        }

        bool out_mouse_captured = false;
        if (!this->rail_client_execute.input_mouse(device_flags, x, y, keymap, out_mouse_captured)) {
            this->mouse_state.chained_input_mouse = [this] (int device_flags, int x, int y, Keymap2 * keymap, bool & out_mouse_captured){
                return this->rail_client_execute.input_mouse(device_flags, x, y, keymap, out_mouse_captured);
            };
            this->mouse_state.input_mouse(device_flags, x, y, keymap);

            if (out_mouse_captured) {
                this->allow_mouse_pointer_change(false);
                this->current_mouse_owner = MouseOwner::ClientExecute;
            }
            else {
                if (MouseOwner::WidgetModule != this->current_mouse_owner) {
                    this->redo_mouse_pointer_change(x, y);
                }

                this->current_mouse_owner = MouseOwner::WidgetModule;
            }
        }

        this->screen.rdp_input_mouse(device_flags, x, y, keymap);

        if (out_mouse_captured) {
            this->allow_mouse_pointer_change(true);
        }
    }
}

// Callback

void RailModuleHostMod::send_to_mod_channel(
    CHANNELS::ChannelNameId front_channel_name,
    InStream& chunk, size_t length, uint32_t flags)
{

    if (front_channel_name == CHANNELS::channel_names::rail){
        if (this->rail_enabled
        && this->rail_client_execute.is_ready()){
            this->rail_client_execute.send_to_mod_rail_channel(length, chunk, flags);
        }
        return;
    }

    if (this->rail_enabled
    && this->rail_client_execute.is_ready()
    && front_channel_name == CHANNELS::channel_names::drdynvc)
    {
        this->dvc_manager.send_to_mod_drdynvc_channel(length, chunk, flags);
    }

    mod_api& mod = this->rail_module_host.get_managed_mod();
    mod.send_to_mod_channel(front_channel_name, chunk, length, flags);
}

void RailModuleHostMod::create_shadow_session(const char * userdata, const char * type)
{
    mod_api& mod = this->rail_module_host.get_managed_mod();
    mod.create_shadow_session(userdata, type);
}

void RailModuleHostMod::send_auth_channel_data(const char * data)
{
    mod_api& mod = this->rail_module_host.get_managed_mod();
    mod.send_auth_channel_data(data);
}

void RailModuleHostMod::send_checkout_channel_data(const char * data)
{
    mod_api& mod = this->rail_module_host.get_managed_mod();
    mod.send_checkout_channel_data(data);
}

// mod_api

bool RailModuleHostMod::is_up_and_running() const
{
    return this->rail_module_host.get_managed_mod().is_up_and_running();
}

bool RailModuleHostMod::server_error_encountered() const
{
    return this->rail_module_host.get_managed_mod().server_error_encountered();
}

void RailModuleHostMod::move_size_widget(int16_t left, int16_t top, uint16_t width,
                        uint16_t height)
{
    Dimension dim = this->get_dim();

    this->rail_module_host.move_size_widget(left, top, width, height);

    if (dim.w && dim.h && ((dim.w != width) || (dim.h != height)) &&
        this->rail_client_execute.is_resizing_hosted_desktop_enabled()) {

        if (this->disconnection_reconnection_timer) {
            this->events.reset_timeout(this->time_base.get_current_time()+std::chrono::seconds{1},
                          this->disconnection_reconnection_timer);

        }
        else {
            this->disconnection_reconnection_timer = this->events.create_event_timeout(
                "RAIL Module Host Disconnection Reconnection Timeout", this,
                this->time_base.get_current_time()+std::chrono::seconds(1),
                [this](Event&)
                {
                    if (this->rail_module_host.get_managed_mod().is_auto_reconnectable()){
                        throw Error(ERR_AUTOMATIC_RECONNECTION_REQUIRED);
                    }
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

gdi::GraphicApi & RailModuleHostMod::proxy_gd(gdi::GraphicApi& gd)
{
    return this->rail_module_host.proxy_gd(gd);
}
