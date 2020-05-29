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
#include "RAIL/client_execute.hpp"
#include "core/front_api.hpp"
#include "core/RDP/slowpath.hpp"
#include "configs/config.hpp"


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

void RailModuleHostMod::cancel_double_click_detection()
{
    assert(this->rail_enabled);

    this->first_click_down_timer.reset();

    this->dc_state = DCState::Wait;
}

RailModuleHostMod::RailModuleHostMod(
    RailModuleHostModVariables vars,
    TimeBase& time_base,
    TimerContainer& timer_events_,
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
    , dc_state(DCState::Wait)
    , rail_enabled(rail_client_execute.is_rail_enabled())
    , current_mouse_owner(MouseOwner::WidgetModule)
    , time_base(time_base)
    , timer_events_(timer_events_)
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
    if (this->rail_enabled && !this->rail_client_execute) {
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
void RailModuleHostMod::rdp_gdi_up_and_running(ScreenInfo & screen_info)
{
    mod_api& mod = this->rail_module_host.get_managed_mod();

    mod.rdp_gdi_up_and_running(screen_info);
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
        }
        else {
            bool out_mouse_captured = false;
            if (!this->rail_client_execute.input_mouse(device_flags, x, y, out_mouse_captured)) {
                switch (this->dc_state) {
                    case DCState::Wait:
                        if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                            this->dc_state = DCState::FirstClickDown;

                            if (this->first_click_down_timer) {
                                this->first_click_down_timer->set_delay(std::chrono::seconds(1));
                            }
                            else {
                                this->first_click_down_timer = timer_events_
                                .create_timer_executor(this->time_base)
                                .set_delay(std::chrono::seconds(1))
                                .on_action(jln::one_shot([this]{
                                    this->dc_state = DCState::Wait;
                                }));
                            }
                        }
                    break;

                    case DCState::FirstClickDown:
                        if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                            this->dc_state = DCState::FirstClickRelease;
                        }
                        else if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                        }
                        else {
                            this->cancel_double_click_detection();
                        }
                    break;

                    case DCState::FirstClickRelease:
                        if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                            this->dc_state = DCState::SecondClickDown;
                        }
                        else {
                            this->cancel_double_click_detection();
                        }
                    break;

                    case DCState::SecondClickDown:
                        if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                            this->dc_state = DCState::Wait;

                            bool out_mouse_captured_2 = false;

                            this->rail_client_execute.input_mouse(PTRFLAGS_EX_DOUBLE_CLICK, x, y, out_mouse_captured_2);

                            this->cancel_double_click_detection();
                        }
                        else if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                        }
                        else {
                            this->cancel_double_click_detection();
                        }
                    break;

                    default:
                        assert(false);

                        this->cancel_double_click_detection();
                    break;
                }

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
}

// Callback

void RailModuleHostMod::send_to_mod_channel(
    CHANNELS::ChannelNameId front_channel_name,
    InStream& chunk, size_t length, uint32_t flags)
{
    if (this->rail_enabled && this->rail_client_execute){
        if (front_channel_name == CHANNELS::channel_names::rail) {
            this->rail_client_execute.send_to_mod_rail_channel(length, chunk, flags);
        }
        else if (front_channel_name == CHANNELS::channel_names::drdynvc) {
            this->dvc_manager.send_to_mod_drdynvc_channel(length, chunk, flags);
        }
    }

    if (front_channel_name == channel_names::rail) { return; }

    mod_api& mod = this->rail_module_host.get_managed_mod();

    mod.send_to_mod_channel(front_channel_name, chunk, length, flags);
}

void RailModuleHostMod::send_auth_channel_data(const char * string_data)
{
    mod_api& mod = this->rail_module_host.get_managed_mod();

    mod.send_auth_channel_data(string_data);
}

void RailModuleHostMod::send_checkout_channel_data(const char * string_data)
{
    mod_api& mod = this->rail_module_host.get_managed_mod();

    mod.send_checkout_channel_data(string_data);
}

// mod_api

bool RailModuleHostMod::is_up_and_running() const
{
    return this->rail_module_host.get_managed_mod().is_up_and_running();
}

void RailModuleHostMod::move_size_widget(int16_t left, int16_t top, uint16_t width,
                        uint16_t height)
{
    Dimension dim = this->get_dim();

    this->rail_module_host.move_size_widget(left, top, width, height);

    if (dim.w && dim.h && ((dim.w != width) || (dim.h != height)) &&
        this->rail_client_execute.is_resizing_hosted_desktop_enabled()) {
        if (this->disconnection_reconnection_timer) {
            this->disconnection_reconnection_timer->set_delay(std::chrono::seconds(1));
        }
        else {
            this->disconnection_reconnection_timer = this->timer_events_
                .create_timer_executor(time_base, std::ref(*this))
                .set_delay(std::chrono::seconds(1))
                .on_action([](auto ctx, RailModuleHostMod& self){
                if (self.rail_module_host.get_managed_mod().is_auto_reconnectable()) {
                    throw Error(ERR_AUTOMATIC_RECONNECTION_REQUIRED);
                }
//                return ctx.terminate();
                return ctx.ready();
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
