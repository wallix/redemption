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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Use (implemented) basic RDP orders to draw some known test pattern

*/

#if !defined(__TEST_CARD_HPP__)
#define __TEST_CARD_HPP__

struct test_card_mod : public internal_mod {
    test_card_mod(
        wait_obj * event,
        int (& keys)[256], int & key_flags, Keymap * &keymap,
        ModContext & context, Front & front, Session * session):
            internal_mod(keys, key_flags, keymap, front)
    {
        this->event = event;
        this->event->set();
    }

    virtual ~test_card_mod()
    {
    }

    // event from front (keyboard or mouse event)
    virtual int mod_event(int msg, long x, long y, long param4, long param5)
    {
        return 0;
    }

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual int mod_signal()
    {
        this->draw();
        this->event->reset();
        return 0;
    }


    void draw()
    {
        this->server_begin_update();

        this->server_set_clip(this->screen.rect);

        this->opaque_rect(RDPOpaqueRect(this->screen.rect, WHITE));
        this->opaque_rect(RDPOpaqueRect(this->screen.rect.shrink(5), RED));
        this->opaque_rect(RDPOpaqueRect(this->screen.rect.shrink(10), GREEN));
        this->opaque_rect(RDPOpaqueRect(this->screen.rect.shrink(15), BLUE));
        this->opaque_rect(RDPOpaqueRect(this->screen.rect.shrink(20), BLACK));

        Rect winrect = this->screen.rect.shrink(30);
        this->opaque_rect(RDPOpaqueRect(winrect, WINBLUE));

        Bitmap bitmap(SHARE_PATH "/" "Philips_PM5544_640.bmp");
        this->bitmap_update(bitmap,
            Rect(winrect.x + (winrect.cx - bitmap.cx)/2,
                 winrect.y + (winrect.cy - bitmap.cy)/2,
                 bitmap.cx, bitmap.cy),
             0, 0);


        this->server_draw_text(30, 30, "White", BLACK, WHITE);
        this->server_draw_text(30, 50, "Red  ", BLACK, RED);
        this->server_draw_text(30, 70, "Green", BLACK, GREEN);
        this->server_draw_text(30, 90, "Blue ", BLACK, BLUE);
        this->server_draw_text(30, 110, "Black", WHITE, BLACK);


        Bitmap card(SHARE_PATH "/" REDEMPTION_LOGO24);
        this->bitmap_update(card,
            Rect(this->screen.rect.cx - card.cx - 30,
                 this->screen.rect.cy - card.cy - 30, card.cx, card.cy),
             0, 0);

        this->server_end_update();
    }

};

#endif
