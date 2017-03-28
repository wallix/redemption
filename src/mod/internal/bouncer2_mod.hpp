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

#include "core/front_api.hpp"
#include "mod/internal/internal_mod.hpp"

class Bouncer2Mod : public InternalMod
{
    int speedx;
    int speedy;

    Rect dancing_rect;

    bool draw_green_carpet = true;

    int mouse_x = 0;
    int mouse_y = 0;

public:
    Bouncer2Mod(FrontAPI & front, uint16_t width, uint16_t height,
                Font const & font, bool dont_resize)
    : InternalMod(front, width, height, font, Theme{}, dont_resize)
    , speedx(10)
    , speedy(10)
    , dancing_rect(0,0,100,100)
    , mouse_x(front.mouse_x)
    , mouse_y(front.mouse_y)
    {}

    ~Bouncer2Mod() override {
        this->screen.clear();
    }

    void rdp_input_invalidate(Rect /*rect*/) override {
        this->draw_green_carpet = true;
    }

    void rdp_input_mouse(int /*device_flags*/, int x, int y,
                         Keymap2 * /*keymap*/) override {
        this->mouse_x = x;
        this->mouse_y = y;
    }

    void rdp_input_scancode(long /*param1*/, long /*param2*/, long /*param3*/,
                            long /*param4*/, Keymap2 * keymap) override {
        if (keymap->nb_kevent_available() > 0
         && keymap->get_kevent() == Keymap2::KEVENT_ESC) {
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
            return ;
        }

        this->interaction();
    }

    void refresh(Rect clip) override {
        this->rdp_input_invalidate(clip);
    }

private:
    int interaction()
    {
        // Get x% of the screen cx and cy
        long x = this->mouse_x;
        long y = this->mouse_y;
        int scarex = this->get_screen_rect().cx / 5;
        int scarey = this->get_screen_rect().cx / 5;
        Rect scareZone(this->dancing_rect.getCenteredX() - (scarex / 2),this->dancing_rect.getCenteredY() - (scarey / 2),scarex,scarey);

        // Calculating new speedx and speedy, if cube encounters a moving mouse pointer, it flees
        if (scareZone.contains_pt(x,y)) {
            if (((this->dancing_rect.getCenteredX() - x) < scarex)
            && this->dancing_rect.getCenteredX() > x) {
                this->speedx = 2;
            } else if (((x - this->dancing_rect.getCenteredX()) < scarex)
            && x > this->dancing_rect.getCenteredX()) {
                this->speedx = -2;
            }
            if (((this->dancing_rect.getCenteredY() - y) < scarey)
            && this->dancing_rect.getCenteredY() > y) {
                this->speedy = 2;
            } else if (((y - this->dancing_rect.getCenteredY()) < scarey) && y > this->dancing_rect.getCenteredY()) {
                this->speedy = -2;
            }
        }
        return 0;
    }

public:
    // This should come from BACK!
    void draw_event(time_t /*now*/, gdi::GraphicApi & drawable) override
    {
        auto const color_ctx = gdi::ColorCtx::depth24();

        if (this->draw_green_carpet) {
            drawable.begin_update();
            drawable.draw(RDPOpaqueRect(this->screen.get_rect(), RDPColor{0x00FF00}), this->screen.get_rect(), color_ctx);
            drawable.end_update();

            this->draw_green_carpet = false;
        }

        this->interaction();

        // Calculating new speedx and speedy
        if (this->dancing_rect.x <= 0 && this->speedx < 0) {
            this->speedx = -this->speedx;
        } else if (this->dancing_rect.x + this->dancing_rect.cx >= this->get_screen_rect().cx && this->speedx > 0) {
            this->speedx = -this->speedx;
        }
        if (this->dancing_rect.y <= 0 && this->speedy < 0) {
            this->speedy = -this->speedy;
        } else if (this->dancing_rect.y + this->dancing_rect.cy >= this->get_screen_rect().cy && this->speedy > 0) {
            this->speedy = -this->speedy;
        }

        // Saving old rect position
        Rect oldrect = this->dancing_rect.offset(0,0);

        // Setting the new position
        this->dancing_rect.x += this->speedx;
        this->dancing_rect.y += this->speedy;

        drawable.begin_update();
        // Drawing the RECT
        drawable.draw(RDPOpaqueRect(this->dancing_rect, RDPColor(0x0000FF)), this->screen.get_rect(), color_ctx);

        // And erase
        this->wipe(oldrect, this->dancing_rect, RDPColor{0x00FF00}, this->screen.get_rect(), drawable);
        drawable.end_update();

        // Final with setting next idle time
        this->event.set(33333); // 0.03s is 30fps
    }

    bool is_up_and_running() override { return true; }

private:
    void wipe(Rect const oldrect, Rect newrect, RDPColor color, const Rect clip, gdi::GraphicApi & drawable) {
        oldrect.difference(newrect, [&](const Rect & a) {
            drawable.draw(RDPOpaqueRect(a, color), clip, gdi::ColorCtx::depth24());
        });
    }
};
