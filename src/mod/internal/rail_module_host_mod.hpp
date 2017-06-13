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

#include "core/RDP/gcc/userdata/cs_monitor.hpp"
#include "mod/internal/internal_mod.hpp"
#include "mod/internal/locally_integrable_mod.hpp"
#include "mod/internal/widget2/notify_api.hpp"
#include "mod/internal/widget2/rail_module_host.hpp"
#include "mod/mod_api.hpp"

#include "configs/config_access.hpp"

using RailModuleHostModVariables = vcfg::variables<
    vcfg::var<cfg::translation::language,                        vcfg::accessmode::get>,
    vcfg::var<cfg::font,                                         vcfg::accessmode::get>,
    vcfg::var<cfg::theme,                                        vcfg::accessmode::get>,
    vcfg::var<cfg::remote_program::allow_resize_hosted_desktop,  vcfg::accessmode::get>
>;

class RailModuleHostMod : public LocallyIntegrableMod, public NotifyApi {
    RailModuleHost rail_module_host;

    RailModuleHostModVariables vars;

    class ManagedModEventHandler : public EventHandler::CB {
        RailModuleHostMod& mod_;

    public:
        ManagedModEventHandler(RailModuleHostMod& mod)
        : mod_(mod)
        {}

        void operator()(time_t now, wait_obj& event, gdi::GraphicApi& drawable) override {
            this->mod_.process_managed_mod_event(now, event, drawable);
        }
    } managed_mod_event_handler;

    wait_obj disconnection_reconnection_event;

    bool disconnection_reconnection_required = false;   // Window resize

    class DisconnectionReconnectionEventHandler : public EventHandler::CB {
    public:
        void operator()(time_t/* now*/, wait_obj&/* event*/, gdi::GraphicApi&/* drawable*/) override {
            throw Error(ERR_AUTOMATIC_RECONNECTION_REQUIRED);
        }
    } disconnection_reconnection_event_handler;

    ClientExecute& client_execute;

public:
    RailModuleHostMod(
        RailModuleHostModVariables vars,
        FrontAPI& front, uint16_t width, uint16_t height,
        Rect const widget_rect, std::unique_ptr<mod_api> managed_mod,
        ClientExecute& client_execute,
        const GCC::UserData::CSMonitor& cs_monitor)
    : LocallyIntegrableMod(front, width, height, vars.get<cfg::font>(),
                           client_execute, vars.get<cfg::theme>())
    , rail_module_host(front, widget_rect.x, widget_rect.y,
                       widget_rect.cx, widget_rect.cy,
                       this->screen, this, std::move(managed_mod),
                       vars.get<cfg::font>(), vars.get<cfg::theme>(),
                       cs_monitor, width, height)
    , vars(vars)
    , managed_mod_event_handler(*this)
    , client_execute(client_execute)
    {
        this->screen.add_widget(&this->rail_module_host);

        this->screen.set_widget_focus(&this->rail_module_host,
            Widget2::focus_reason_tabkey);

        this->screen.rdp_input_invalidate(this->screen.get_rect());
    }

    ~RailModuleHostMod() override
    {
        this->screen.clear();
    }

    void notify(Widget2*, notify_event_t) override {}

    void process_managed_mod_event(time_t now, wait_obj& /*event*/, gdi::GraphicApi& gapi) {
        mod_api& mod = this->rail_module_host.get_managed_mod();

        mod.draw_event(now, gapi);
    }

public:
    // RdpInput

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {
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

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override {
        (void)time;
        (void)device_flags;
        (void)param1;
        (void)param2;
    }

    void rdp_input_up_and_running() override {
        mod_api& mod = this->rail_module_host.get_managed_mod();

        mod.rdp_input_up_and_running();
    }

    // Callback

    void send_to_mod_channel(const char* front_channel_name,
                             InStream& chunk, size_t length,
                             uint32_t flags) override
    {
        LocallyIntegrableMod::send_to_mod_channel(front_channel_name, chunk,
            length, flags);

        if (!::strncmp(front_channel_name, "rail", 4)) { return; }

        mod_api& mod = this->rail_module_host.get_managed_mod();

        mod.send_to_mod_channel(front_channel_name, chunk, length, flags);
    }

    // mod_api

    void draw_event(time_t now, gdi::GraphicApi& gapi) override
    {
        LocallyIntegrableMod::draw_event(now, gapi);

        this->event.reset();
    }

    void get_event_handlers(std::vector<EventHandler>& out_event_handlers) override {
        mod_api& mod = this->rail_module_host.get_managed_mod();

        mod.get_event_handlers(out_event_handlers);

        out_event_handlers.emplace_back(
            &mod.get_event(),
            &this->managed_mod_event_handler,
            mod.get_fd()
        );

        if (this->disconnection_reconnection_required &&
            mod.is_auto_reconnectable()) {
            out_event_handlers.emplace_back(
                &this->disconnection_reconnection_event,
                &this->disconnection_reconnection_event_handler,
                INVALID_SOCKET
            );
        }

        LocallyIntegrableMod::get_event_handlers(out_event_handlers);
    }

    bool is_up_and_running() override
    {
        mod_api& mod = this->rail_module_host.get_managed_mod();

        return mod.is_up_and_running();
    }

    void move_size_widget(int16_t left, int16_t top, uint16_t width,
                          uint16_t height) override
    {
        Dimension dim = this->get_dim();

        this->rail_module_host.move_size_widget(left, top, width, height);

        if (dim.w && dim.h && ((dim.w != width) || (dim.h != height)) &&
            this->client_execute.is_resizing_hosted_desktop_enabled()) {
            this->disconnection_reconnection_required = true;

            this->disconnection_reconnection_event.set(1000000);
        }
    }

    Dimension get_dim() const override
    {
        const mod_api& mod = this->rail_module_host.get_managed_mod();

        return mod.get_dim();
    }

    bool is_resizing_hosted_desktop_allowed() const override {
        return vars.get<cfg::remote_program::allow_resize_hosted_desktop>();
    }
};
