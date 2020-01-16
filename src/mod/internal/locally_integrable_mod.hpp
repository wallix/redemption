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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "core/session_reactor.hpp"
#include "mod/mod_api.hpp"
#include "mod/internal/dvc_manager.hpp"
#include "mod/internal/widget/screen.hpp"

class ClientExecute;

class LocallyIntegrableMod : public mod_api
{
public:
    LocallyIntegrableMod(SessionReactor& session_reactor,
                         TimerContainer& timer_events_,
                         GraphicEventContainer& graphic_events_,
                         gdi::GraphicApi & drawable, FrontAPI & front,
                         uint16_t front_width, uint16_t front_height,
                         Font const & font, ClientExecute & rail_client_execute,
                         Theme const & theme);

    ~LocallyIntegrableMod() override;

    std::string module_name() override {return "Locally Integrable Mod";}

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

    void rdp_gdi_up_and_running(ScreenInfo & ) override {}

    void rdp_gdi_down() override {}

    void rdp_input_invalidate(Rect r) override;

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override;

    void rdp_input_scancode(long param1, long param2, long param3, long param4,
            Keymap2 * keymap) override;

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

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags) override;

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
    void cancel_double_click_detection();

    [[nodiscard]] virtual bool is_resizing_hosted_desktop_allowed() const;

protected:
    uint16_t front_width;
    uint16_t front_height;

    FrontAPI & front;

    WidgetScreen screen;

private:
    ClientExecute & rail_client_execute;
    DVCManager dvc_manager;

    bool alt_key_pressed = false;

    enum class DCState
    {
        Wait,
        FirstClickDown,
        FirstClickRelease,
        SecondClickDown,
    };

    DCState dc_state;

    TimerPtr first_click_down_timer;

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
    SessionReactor& session_reactor;
    TimerContainer& timer_events_;
    GraphicEventContainer& graphic_events_;

private:
    GraphicEventPtr graphic_event;
};
