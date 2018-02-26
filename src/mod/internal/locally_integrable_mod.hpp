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
#include "mod/internal/internal_mod.hpp"
#include "mod/internal/dvc_manager.hpp"

class ClientExecute;

class LocallyIntegrableMod : public InternalMod
{
public:
    LocallyIntegrableMod(SessionReactor& session_reactor,
                         FrontAPI & front,
                         uint16_t front_width, uint16_t front_height,
                         Font const & font, ClientExecute & client_execute,
                         Theme const & theme);

    ~LocallyIntegrableMod() override;

    void rdp_input_invalidate(Rect r) override;

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override;

    void rdp_input_scancode(long param1, long param2, long param3, long param4,
            Keymap2 * keymap) override;

    void refresh(Rect r) override;

    void draw_event(time_t, gdi::GraphicApi &) override;

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags) override;

private:
    void cancel_double_click_detection();

    virtual bool is_resizing_hosted_desktop_allowed() const;

    ClientExecute & client_execute;
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

    SessionReactor::BasicTimerPtr first_click_down_timer;

    const bool rail_enabled;

    enum class MouseOwner
    {
        ClientExecute,
        WidgetModule,
    };

    MouseOwner current_mouse_owner;

    int old_mouse_x = 0;
    int old_mouse_y = 0;

    SessionReactor& session_reactor;
    SessionReactor::GraphicEventPtr graphic_event;
};
