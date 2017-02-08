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
    vcfg::var<cfg::translation::language,   vcfg::accessmode::get>,
    vcfg::var<cfg::font,                    vcfg::accessmode::get>,
    vcfg::var<cfg::theme,                   vcfg::accessmode::get>
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

        void operator()(time_t now, wait_obj* event, gdi::GraphicApi& drawable) override {
            this->mod_.process_managed_mod_event(now, event, drawable);
        }
    } managed_mod_event_handler;

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

    void process_managed_mod_event(time_t now, wait_obj* /*event*/, gdi::GraphicApi& gapi) {
        mod_api& mod = this->rail_module_host.get_managed_mod();

        mod.draw_event(now, gapi);
    }

public:
    // RdpInput

    void rdp_input_scancode(long param1, long param2, long param3,
                            long param4, Keymap2 * keymap) override {
        LocallyIntegrableMod::rdp_input_scancode(param1, param2, param3,
            param4, keymap);
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
        this->rail_module_host.move_size_widget(left, top, width, height);
    }

    Dimension get_dim() const override
    {
        const mod_api& mod = this->rail_module_host.get_managed_mod();

        return mod.get_dim();
    }
};
