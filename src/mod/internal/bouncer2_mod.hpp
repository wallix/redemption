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

#include "mod/internal/internal_mod.hpp"
#include "core/session_reactor.hpp"

class Bouncer2Mod : public InternalMod
{
    int speedx = 2;
    int speedy = 2;

    Rect dancing_rect;

    bool draw_green_carpet = true;

    int mouse_x = 0;
    int mouse_y = 0;

    SessionReactor& session_reactor;
    SessionReactor::GraphicTimerPtr timer;

public:
    Bouncer2Mod(
         SessionReactor& session_reactor,
         FrontAPI & front, uint16_t width, uint16_t height,
         Font const & font);

    ~Bouncer2Mod() override;

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

    void refresh(Rect clip) override;

public:
    // This should come from BACK!
    void draw_event(time_t /*now*/, gdi::GraphicApi & gd) override;

    bool is_up_and_running() const override
    {
        return true;
    }

private:
    int interaction();
    void wipe(Rect const oldrect, Rect newrect, RDPColor color, const Rect clip, gdi::GraphicApi & gd);
};
