/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni, JOnathan Poelen, Raphael Zhou

   Use (implemented) basic RDP orders to draw some known test pattern
*/

#ifndef _REDEMPTION_MOD_INTERNAL_TEST_CARD_MOD_HPP_
#define _REDEMPTION_MOD_INTERNAL_TEST_CARD_MOD_HPP_

#include "internal_mod.hpp"
#include "RDP/bitmapupdate.hpp"

class TestCardMod : public InternalMod
{
    BGRPalette palette332;

public:
    TestCardMod(FrontAPI & front, uint16_t width, uint16_t height)
    : InternalMod(front, width, height)
    {
        init_palette332(this->palette332);
    }

    virtual ~TestCardMod()
    {
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
        }
    }

    virtual void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                                       int16_t /*param1*/, int16_t /*param2*/)
    {
    }

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual void draw_event()
    {
        this->draw();
        this->event.reset();
    }

    void draw()
    {
        this->front.begin_update();

        const Rect & clip = this->get_screen_rect();

        this->front.draw(RDPOpaqueRect(this->get_screen_rect(), WHITE), clip);
        this->front.draw(RDPOpaqueRect(this->get_screen_rect().shrink(5), RED), clip);
        this->front.draw(RDPOpaqueRect(this->get_screen_rect().shrink(10), GREEN), clip);
        this->front.draw(RDPOpaqueRect(this->get_screen_rect().shrink(15), BLUE), clip);
        this->front.draw(RDPOpaqueRect(this->get_screen_rect().shrink(20), BLACK), clip);

        Rect winrect = this->get_screen_rect().shrink(30);
        this->front.draw(RDPOpaqueRect(winrect, WINBLUE), clip);

        //Bitmap bitmap(SHARE_PATH "/" "Philips_PM5544_640.png");
        Bitmap bitmap(SHARE_PATH "/" "Philips_PM5544_640.bmp");
        this->front.draw(RDPMemBlt(0,
            Rect(winrect.x + (winrect.cx - bitmap.cx)/2,
                 winrect.y + (winrect.cy - bitmap.cy)/2,
                 bitmap.cx, bitmap.cy),
                 0xCC,
             0, 0, 0), clip, bitmap);

        //  lineTo mix_mode=1 startx=200 starty=1198 endx=200 endy=145 bg_color=0 rop2=13 clip=(200, 145, 1, 110)
        this->front.draw(
            RDPLineTo(1, 200, 1198, 200, 145, 0, 13, RDPPen(0, 1, 0x0000FF)),
            Rect(200, 145, 1, 110));

        this->front.draw(
            RDPLineTo(1, 200, 145, 200, 1198, 0, 13, RDPPen(0, 1, 0x0000FF)),
            Rect(200, 145, 1, 110));

        this->front.draw(
            RDPLineTo(1, 201, 1198, 200, 145, 0, 13, RDPPen(0, 1, 0x0000FF)),
            Rect(200, 145, 1, 110));

        this->front.draw(
            RDPLineTo(1, 200, 145, 201, 1198, 0, 13, RDPPen(0, 1, 0x0000FF)),
            Rect(200, 145, 1, 110));

        this->front.draw(
            RDPLineTo(1, 1198, 200, 145, 200, 0, 13, RDPPen(0, 1, 0x0000FF)),
            Rect(145, 200, 110, 1));

        this->front.draw(
            RDPLineTo(1, 145, 200, 1198, 200, 0, 13, RDPPen(0, 1, 0x0000FF)),
            Rect(145, 200, 110, 1));

        this->front.draw(
            RDPLineTo(1, 1198, 201, 145, 200, 0, 13, RDPPen(0, 1, 0x0000FF)),
            Rect(145, 200, 110, 1));

        this->front.draw(
            RDPLineTo(1, 145, 200, 1198, 201, 0, 13, RDPPen(0, 1, 0x0000FF)),
            Rect(145, 200, 110, 1));

        this->front.server_draw_text(30, 30, "White", BLACK, WHITE, clip);
        this->front.server_draw_text(30, 50, "Red  ", BLACK, RED, clip);
        this->front.server_draw_text(30, 70, "Green", BLACK, GREEN, clip);
        this->front.server_draw_text(30, 90, "Blue ", BLACK, BLUE, clip);
        this->front.server_draw_text(30, 110, "Black", WHITE, BLACK, clip);

        Bitmap card(SHARE_PATH "/" REDEMPTION_LOGO24);
        this->front.draw(RDPMemBlt(0,
            Rect(this->get_screen_rect().cx - card.cx - 30,
                 this->get_screen_rect().cy - card.cy - 30, card.cx, card.cy),
                 0xCC,
             0, 0, 0), clip, card);

        // Bogus square generating zero width/height tiles if not properly guarded
        uint8_t comp64x64RED[] = {
            0xc0, 0x30, 0x00, 0x00, 0xFF,
            0xf0, 0xc0, 0x0f,
        };

        Bitmap bloc64x64(24, &this->palette332, 64, 64, comp64x64RED, sizeof(comp64x64RED), true);
        this->front.draw(RDPMemBlt(0,
            Rect(0, this->get_screen_rect().cy - 64, bloc64x64.cx, bloc64x64.cy), 0xCC,
             32, 32, 0), clip, bloc64x64);

        Bitmap logo(SHARE_PATH "/ad8b.bmp");
        this->front.draw(RDPMemBlt(0,
            Rect(100, 100, 26, 32),
            0xCC,
            80, 50, 0), clip, logo);

        this->front.end_update();
    }
};

#endif
