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

#include "mod/internal/widget/rail_module_host.hpp"

#include "configs/config.hpp"
#include "configs/config_access.hpp"
#include "core/session_reactor.hpp"
#include "mod/mod_api.hpp"
#include "mod/internal/dvc_manager.hpp"
#include "mod/internal/widget/screen.hpp"
#include "RAIL/client_execute.hpp"

#include <memory>

class ClientExecute;
class TimeBase;

using RailModuleHostModVariables = vcfg::variables<
    vcfg::var<cfg::translation::language,                        vcfg::accessmode::get>,
    vcfg::var<cfg::remote_program::allow_resize_hosted_desktop,  vcfg::accessmode::get>,
    vcfg::var<cfg::context::rail_module_host_mod_is_active,      vcfg::accessmode::set>
>;


class RailModuleHostMod : public mod_api, public NotifyApi
{
public:
    [[nodiscard]] Font const & font() const
    {
        return this->screen.font;
    }

    [[nodiscard]] Theme const & theme() const
    {
        return this->screen.theme;
    }

    [[nodiscard]] Rect get_screen_rect() const
    {
        return this->screen.get_rect();
    }

    void rdp_input_invalidate(Rect r) override;

    void rdp_input_scancode(long param1, long param2, long param3, long param4,
            Keymap2 * keymap) override;

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override
    {
        this->screen.rdp_input_unicode(unicode, flag);
    }

    void refresh(Rect r) override;

    void allow_mouse_pointer_change(bool allow)
    {
        this->screen.allow_mouse_pointer_change(allow);
    }

    void redo_mouse_pointer_change(int x, int y)
    {
        this->screen.redo_mouse_pointer_change(x, y);
    }

protected:
    uint16_t front_width;
    uint16_t front_height;

    FrontAPI & front;

    WidgetScreen screen;
    ClientExecute& rail_client_execute;

private:
    DVCManager dvc_manager;

    bool alt_key_pressed = false;

    MouseState mouse_state;
    int disconnection_reconnection_timer = 0;

    const bool rail_enabled;

    enum class MouseOwner
    {
        ClientExecute,
        WidgetModule,
    };

    MouseOwner current_mouse_owner;

    int old_mouse_x = 0;
    int old_mouse_y = 0;

protected:
    TimeBase& time_base;
    EventContainer& events;

public:
    RailModuleHostMod(
        RailModuleHostModVariables vars,
        TimeBase& time_base,
        EventContainer& events,
        gdi::GraphicApi & drawable, FrontAPI& front, uint16_t width, uint16_t height,
        Rect const widget_rect, std::unique_ptr<mod_api> managed_mod,
        ClientExecute& rail_client_execute, Font const& font, Theme const& theme,
        const GCC::UserData::CSMonitor& cs_monitor, bool can_resize_hosted_desktop);

    ~RailModuleHostMod() override
    {
        end_of_lifespan(this->events, this);    
        this->rail_client_execute.reset(true);
        this->screen.clear();
        this->vars.set<cfg::context::rail_module_host_mod_is_active>(false);
    }

    void init() override;

    std::string module_name() override {return "Rail Module Host Mod";}

    void notify(Widget* /*sender*/, notify_event_t /*event*/) override
    {}

    RailModuleHost& get_module_host();

    // RdpInput

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override;

    void rdp_gdi_down() override {}

    void rdp_gdi_up_and_running(ScreenInfo &) override;

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        (void)time;
        (void)device_flags;
        (void)param1;
        (void)param2;
    }

    // Callback

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name,
                             InStream& chunk, size_t length,
                             uint32_t flags) override;

    void send_auth_channel_data(const char * string_data) override;

    void send_checkout_channel_data(const char * string_data) override;

    // mod_api

    [[nodiscard]] bool is_up_and_running() const override;

    void move_size_widget(int16_t left, int16_t top, uint16_t width,
                          uint16_t height) override;

    [[nodiscard]] Dimension get_dim() const override;

    [[nodiscard]] bool is_resizing_hosted_desktop_allowed() const;

    gdi::GraphicApi& proxy_gd(gdi::GraphicApi& gd);

private:
    RailModuleHost rail_module_host;

    RailModuleHostModVariables vars;

    bool can_resize_hosted_desktop = false;
};
