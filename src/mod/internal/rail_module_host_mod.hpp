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

#include "mod/internal/locally_integrable_mod.hpp"
#include "mod/internal/widget/rail_module_host.hpp"

#include "configs/config_access.hpp"

#include <memory>

class SessionReactor;

using RailModuleHostModVariables = vcfg::variables<
    vcfg::var<cfg::translation::language,                        vcfg::accessmode::get>,
    vcfg::var<cfg::remote_program::allow_resize_hosted_desktop,  vcfg::accessmode::get>
>;


class RailModuleHostMod : public LocallyIntegrableMod, public NotifyApi
{
public:
    RailModuleHostMod(
        RailModuleHostModVariables vars, SessionReactor& session_reactor,
        FrontAPI& front, uint16_t width, uint16_t height,
        Rect const widget_rect, std::unique_ptr<mod_api> managed_mod,
        ClientExecute& client_execute, Font const& font, Theme const& theme,
        const GCC::UserData::CSMonitor& cs_monitor, bool can_resize_hosted_desktop);

    ~RailModuleHostMod() override
    {
        this->screen.clear();
    }

    void notify(Widget* /*sender*/, notify_event_t /*event*/) override
    {}

    RailModuleHost& get_module_host();

    // RdpInput

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override;

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        (void)time;
        (void)device_flags;
        (void)param1;
        (void)param2;
    }

    void rdp_input_up_and_running() override;

    // Callback

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name,
                             InStream& chunk, size_t length,
                             uint32_t flags) override;

    void send_auth_channel_data(const char * string_data) override;

    void send_checkout_channel_data(const char * string_data) override;

    // mod_api

    void draw_event(time_t now, gdi::GraphicApi& gapi) override;

    bool is_up_and_running() const override;

    void move_size_widget(int16_t left, int16_t top, uint16_t width,
                          uint16_t height) override;

    Dimension get_dim() const override;

    bool is_resizing_hosted_desktop_allowed() const override;

    gdi::GraphicApi& proxy_gd(gdi::GraphicApi& gd);

private:
    RailModuleHost rail_module_host;

    RailModuleHostModVariables vars;

    bool can_resize_hosted_desktop = false;

    SessionReactor::TimerPtr disconnection_reconnection_timer; // Window resize

    ClientExecute& client_execute;
};
