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

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"
#include "keyboard/keymap2.hpp"
#include "mod/internal/bouncer2_mod.hpp"
#include "utils/sugar/update_lock.hpp"

Bouncer2Mod::Bouncer2Mod(
    SessionReactor& session_reactor,
    FrontAPI & front, uint16_t width, uint16_t height,
    Font const & font)
: InternalMod(front, width, height, font, Theme{})
, dancing_rect(0,0,100,100)
, session_reactor(session_reactor)
, timer(session_reactor.create_graphic_timer()
    .set_delay(std::chrono::milliseconds(33))
    .on_action(jln::always_ready([this](gdi::GraphicApi& gd){
        this->draw_event(0, gd);
    })))
{}

Bouncer2Mod::~Bouncer2Mod()
{
    this->screen.clear();
}

void Bouncer2Mod::rdp_input_scancode(
    long /*param1*/, long /*param2*/, long /*param3*/, long /*param4*/, Keymap2 * keymap)
{
    if (keymap->nb_kevent_available() > 0 && keymap->get_kevent() == Keymap2::KEVENT_ESC) {
        this->session_reactor.set_event_next(BACK_EVENT_STOP);
        return ;
    }

    this->interaction();
}

void Bouncer2Mod::refresh(Rect clip)
{
    this->rdp_input_invalidate(clip);
}

int Bouncer2Mod::interaction()
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

// This should come from BACK!
void Bouncer2Mod::draw_event(time_t /*now*/, gdi::GraphicApi & gd)
{
    auto const color_ctx = gdi::ColorCtx::depth24();

    if (this->draw_green_carpet) {
        update_lock lock{gd};
        gd.draw(RDPOpaqueRect(this->screen.get_rect(), encode_color24()(GREEN)), this->screen.get_rect(), color_ctx);
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

    gd.begin_update();
    // Drawing the RECT
    gd.draw(RDPOpaqueRect(this->dancing_rect, encode_color24()(RED)), this->screen.get_rect(), color_ctx);

    // And erase
    this->wipe(oldrect, this->dancing_rect, encode_color24()(GREEN), this->screen.get_rect(), gd);
    gd.end_update();
}

void Bouncer2Mod::wipe(
    Rect const oldrect, Rect newrect, RDPColor color,
    const Rect clip, gdi::GraphicApi & gd)
{
    oldrect.difference(newrect, [&](const Rect & a) {
        gd.draw(RDPOpaqueRect(a, color), clip, gdi::ColorCtx::depth24());
    });
}
