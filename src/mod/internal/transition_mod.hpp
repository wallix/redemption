/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2019
 *   Author(s): Meng Tan
 *
 *   Show a waiting message in modules transitions,
 *   also allow to quit with escape key
 */


#pragma once

#include "configs/config_access.hpp"
#include "mod/internal/widget/tooltip.hpp"
#include "mod/mod_api.hpp"
#include "mod/internal/dvc_manager.hpp"
#include "mod/internal/widget/screen.hpp"
#include "core/events.hpp"
#include "keyboard/mouse.hpp"

using TransitionModVariables = vcfg::variables<
    vcfg::var<cfg::translation::language,               vcfg::accessmode::get>,
    vcfg::var<cfg::debug::mod_internal,                 vcfg::accessmode::get>
>;

class ClientExecute;
class TimeBase;


class TransitionMod : public mod_api
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

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override
    {
        this->screen.rdp_input_unicode(unicode, flag);
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        (void)time;
        (void)device_flags;
        (void)param1;
        (void)param2;
    }

    void refresh(Rect r) override;

    [[nodiscard]] Dimension get_dim() const override
    {
        return Dimension(this->front_width, this->front_height);
    }

    void allow_mouse_pointer_change(bool allow)
    {
        this->screen.allow_mouse_pointer_change(allow);
    }

    void redo_mouse_pointer_change(int x, int y)
    {
        this->screen.redo_mouse_pointer_change(x, y);
    }

private:
    [[nodiscard]] virtual bool is_resizing_hosted_desktop_allowed() const;

protected:
    uint16_t front_width;
    uint16_t front_height;

    FrontAPI & front;

    WidgetScreen screen;

private:
    ClientExecute & rail_client_execute;
    DVCManager dvc_manager;

    MouseState mouse_state;

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

private:
    WidgetTooltip ttmessage;

    TransitionModVariables vars;

public:
    TransitionMod(
        TransitionModVariables vars,
        TimeBase& time_base,
        EventContainer& events,
        gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
        Rect const widget_rect, ClientExecute & rail_client_execute, Font const& font,
        Theme const& theme
    );

    ~TransitionMod() override;

    std::string module_name() override {return "Transition Mod";}

    [[nodiscard]] bool is_up_and_running() const override { return true; }

    bool server_error_encountered() const override { return false; }

    void init() override;

    void rdp_gdi_up_and_running() override {}

    void rdp_gdi_down() override {}

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags) override;
    void create_shadow_session(const char * /*userdata*/, const char * /*type*/) override {}
    void send_auth_channel_data(const char * /*data*/) override {}
    void send_checkout_channel_data(const char * /*data*/) override {}

    void rdp_input_scancode(long int param1, long int param2, long int param3,
                            long int param4, Keymap2* keymap) override;

    void rdp_input_invalidate(Rect r) override;

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override;
};
