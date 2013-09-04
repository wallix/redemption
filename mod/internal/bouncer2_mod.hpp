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
              Meng Tan

   Bouncer test, high level API

*/

#ifndef REDEMPTION_MOD_INTERNAL_BOUNCER2_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_BOUNCER2_MOD_HPP

#include "front_api.hpp"
#include "internal_mod.hpp"

class Bouncer2Mod : public InternalMod
{
    int speedx;
    int speedy;
    Rect dancing_rect;

public:
    Bouncer2Mod(FrontAPI & front, uint16_t width, uint16_t height)
    : InternalMod(front, width, height)
    , speedx(10)
    , speedy(10)
    , dancing_rect(0,0,100,100)
    {
        this->front.begin_update();
        this->front.draw(RDPOpaqueRect(this->screen.rect, 0x00FF00), this->screen.rect);
        this->front.end_update();

        // Using µsec set
        this->event.set(33333);
    }

    ~Bouncer2Mod()
    {
        this->screen.child_list.clear();
    }

    virtual void rdp_input_invalidate(const Rect & /*rect*/)
    {
    }

    virtual void rdp_input_mouse(int /*device_flags*/, int /*x*/, int /*y*/, Keymap2 * /*keymap*/)
    {
    }

    virtual void rdp_input_scancode(long /*param1*/, long /*param2*/, long /*param3*/,
                                    long /*param4*/, Keymap2 * keymap)
    {
        if (keymap->nb_kevent_available() > 0
         && keymap->get_kevent() == Keymap2::KEVENT_ESC) {
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
            return ;
        }
        this->interaction();
    }

    virtual void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                                       int16_t /*param1*/, int16_t /*param2*/)
    {
    }

    int interaction()
    {
        // Get x% of the screen cx and cy
        long x = this->front.mouse_x;
        long y = this->front.mouse_y;
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

    // This should come from BACK!
    virtual void draw_event(time_t now)
    {
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

        this->front.begin_update();
        // Drawing the RECT
        this->front.draw(RDPOpaqueRect(this->dancing_rect, 0x0000FF), this->screen.rect);

        // And erase
        this->wipe(oldrect, this->dancing_rect, 0x00FF00, this->screen.rect);
        this->front.end_update();

        // Final with setting next idle time
        this->event.set(33333); // 0.3s is 30fps
    }

    void wipe(Rect oldrect, Rect newrect, int color, const Rect & clip) {
        // new RectIterator
        struct RectIt : public Rect::RectIterator {
            int color;
            Bouncer2Mod & b;
            const Rect & clip;

            RectIt(int color, Bouncer2Mod & b, const Rect & clip)
            : color(color), b(b), clip(clip)
            {}

            void callback(const Rect & a) {
                b.front.draw(RDPOpaqueRect(a, color), this->clip);
            }
        } it(color, *this, clip);

        // Use my iterator
        oldrect.difference(newrect, it);
    }
};

#endif
