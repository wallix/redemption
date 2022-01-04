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

#include "mod/mod_api.hpp"
#include "mod/internal/dvc_manager.hpp"
#include "mod/internal/widget/screen.hpp"
#include "mod/internal/widget/module_host.hpp"
#include "core/events.hpp"


class ClientExecute;

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

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override;

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override
    {
        this->screen.rdp_input_unicode(flag, unicode);
    }

    void refresh(Rect r) override;

private:
    FrontAPI & front;

    WidgetScreen screen;
    ClientExecute& rail_client_execute;

    DVCManager dvc_manager;

    EventRef2 disconnection_reconnection_timer;

    const bool rail_enabled;

    enum class MouseOwner : bool
    {
        ClientExecute,
        WidgetModule,
    };

    MouseOwner current_mouse_owner;

    int old_mouse_x = 0;
    int old_mouse_y = 0;

public:
    RailModuleHostMod(
        EventContainer& events,
        gdi::GraphicApi & drawable,
        FrontAPI& front, uint16_t width, uint16_t height,
        Rect const widget_rect, ClientExecute& rail_client_execute,
        Font const& font, Theme const& theme,
        const GCC::UserData::CSMonitor& cs_monitor, bool can_resize_hosted_desktop);

    void set_mod(std::unique_ptr<mod_api>&& managed_mod) noexcept;

    ~RailModuleHostMod();

    gdi::GraphicApi& proxy_gd() { return this->module_host; }

    void init() override;

    void notify(Widget& /*widget*/, notify_event_t /*event*/) override
    {}

    // RdpInput

    void rdp_input_mouse(int device_flags, int x, int y) override;

    void rdp_gdi_down() override {}

    void rdp_gdi_up_and_running() override;

    void rdp_input_synchronize(KeyLocks locks) override
    {
        (void)locks;
    }

    // Callback

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name,
                             InStream& chunk, size_t length,
                             uint32_t flags) override;

    // mod_api

    [[nodiscard]] bool is_up_and_running() const override;

    bool is_auto_reconnectable() const override;

    bool server_error_encountered() const override;

    void move_size_widget(int16_t left, int16_t top, uint16_t width,
                          uint16_t height) override;

    [[nodiscard]] Dimension get_dim() const override;

    [[nodiscard]] bool is_resizing_hosted_desktop_allowed() const;

    void acl_update(AclFieldMask const& acl_fields) override;

private:
    std::unique_ptr<mod_api> managed_mod;
    WidgetModuleHost module_host;

    bool can_resize_hosted_desktop = false;
};
