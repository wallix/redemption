/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Martin Potier, Jonathan Poelen,
              Meng Tan, Raphaël Zhou

   Bouncer test, high level API
*/

#pragma once

#include "mod/mod_api.hpp"
#include "core/events.hpp"
#include "utils/timebase.hpp"

class FrontAPI;
namespace gdi
{
    class GraphicApi;
}

class Bouncer2Mod : public mod_api
{
    uint16_t front_width;
    uint16_t front_height;

    int speedx = 2;
    int speedy = 2;

    Rect dancing_rect;

    bool draw_green_carpet = true;

    int mouse_x = 0;
    int mouse_y = 0;

    EventsGuard events_guard;
    gdi::GraphicApi & gd;

    [[nodiscard]] Rect get_screen_rect() const;

public:
    Bouncer2Mod(
         gdi::GraphicApi & gd,
         EventContainer & events,
         uint16_t width, uint16_t height);

    ~Bouncer2Mod();

    void rdp_gdi_up_and_running() override {}

    void rdp_gdi_down() override {}

    void rdp_input_invalidate(Rect /*rect*/) override
    {
        this->draw_green_carpet = true;
    }

    void rdp_input_mouse(int /*device_flags*/, int x, int y, Keymap2 * /*keymap*/) override
    {
        this->mouse_x = x;
        this->mouse_y = y;
    }

    void rdp_input_scancode(long /*param1*/, long /*param2*/, long /*param3*/,
                            long /*param4*/, Keymap2 * keymap) override;

    void rdp_input_unicode(uint16_t /*unicode*/, uint16_t /*flag*/) override
    {}

    void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                               int16_t /*param1*/, int16_t /*param2*/) override
    {}

    void refresh(Rect clip) override;

    [[nodiscard]] Dimension get_dim() const override;

public:
    // This should come from BACK!
    void draw_event(gdi::GraphicApi & gd);

    [[nodiscard]] bool is_up_and_running() const override
    {
        return true;
    }

    bool server_error_encountered() const override { return false; }

    void send_to_mod_channel(CHANNELS::ChannelNameId /*front_channel_name*/, InStream & /*chunk*/, std::size_t /*length*/, uint32_t /*flags*/) override {}

private:
    int interaction();
};
