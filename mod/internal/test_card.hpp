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
        this->mod_bpp = this->get_front_bpp();

        this->server_begin_update();
        this->server_set_clip(this->screen.rect);
        this->opaque_rect(RDPOpaqueRect(this->screen.rect, WHITE));
        const Rect inner_red = Rect(this->screen.rect.x+5, this->screen.rect.y+5,
                              this->screen.rect.cx-10, this->screen.rect.cy-10);
        this->opaque_rect(RDPOpaqueRect(inner_red, RED));
        const Rect inner_green = Rect(this->screen.rect.x+10, this->screen.rect.y+10,
                              this->screen.rect.cx-20, this->screen.rect.cy-20);
        this->opaque_rect(RDPOpaqueRect(inner_green, GREEN));
        const Rect inner_blue = Rect(this->screen.rect.x+15, this->screen.rect.y+15,
                              this->screen.rect.cx-30, this->screen.rect.cy-30);
        this->opaque_rect(RDPOpaqueRect(inner_blue, BLUE));
        const Rect inner_black = Rect(this->screen.rect.x+20, this->screen.rect.y+20,
                              this->screen.rect.cx-40, this->screen.rect.cy-40);
        this->opaque_rect(RDPOpaqueRect(inner_black, BLACK));

//        this->server_set_clip(Rect(50, 50, 150, 150));
//        this->opaque_rect(RDPOpaqueRect(Rect(50, 50, 150, 150), PINK));
//        this->server_set_clip(Rect(520, 360, 60, 25));
//        this->opaque_rect(RDPOpaqueRect(Rect(520, 360, 60, 25), DARK_GREY));

        this->server_draw_text(535, 363, "Hello", BLACK, YELLOW);

        //front.orders.screen_blt(Rect(190, 190, 50, 50), 50, 50, 0xCC, Rect(190, 190, 50, 50));

//        front.orders.dest_blt(Rect(60, 60, 50, 50), 0, Rect(60, 60, 50, 50));

        this->server_end_update();

//        front.begin_update();
//        RDPBrush brush;
//        brush.org_x = 0;
//        brush.org_y = 0;
//        brush.style = 3;

//        if (front.rdp_layer.client_info.brush_cache_code == 1) {
//            uint8_t pattern[8] = {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55};
//            int cache_idx = 0;
//            if (BRUSH_TO_SEND == front.cache.add_brush(pattern, cache_idx)){
//                front.brush_cache(cache_idx);
//            }
//            brush.hatch = cache_idx;
//            brush.style = 0x81;
//        }
//        front.end_update();

//        front.orders.init();

//        front.orders.pat_blt(
//            Rect(50, 50, 150, 150),
//            0x5A,
//            PINK,
//            WHITE,
//            brush,
//            Rect(100, 100, 50, 50));

//        front.orders.pat_blt(
//            Rect(63, 78, 10, 13),
//            0x5A,
//            BLACK,
//            WHITE,
//            brush,
//            Rect(63, 78, 10, 13));

//        front.send();

//        uint32_t picture24[100][100];

//        for(int i = 0; i < 100; i++){
//            for (int j = 0; j < 100; j++){
//                picture24[i][j] = WHITE;
//            }
//        }

//        front.orders.init();

//        RDPPen pen;
//        pen.style = 1;
//        pen.width = 10;
//        pen.color = BLUE;

//        front.orders.line_to(1, 50, 50, 150, 150, 0x0D, PINK, pen, Rect(100, 100, 50, 50));

//        front.send();

//        cout << "testing front primitives\n";

//        /* Draw a big white rect at right-bottom side of screen */
//        front.begin_update();
//        Region region;
//        Rect r1(0, 0, 800, 600);
//        region.rects.push_back(r1);
//        front.opaque_rect(Rect(500, 400, 200, 150), DARK_GREY, Rect(500, 400, 200, 150));
//        front.end_update();

//        /* Draw black line crossing right-bottom white rect of screen */

//        front.begin_update();

//        pen.style = 1;
//        pen.width = 10;
//        pen.color = PINK;

//        /* Line clipped 500, 400, 201, 151*/
//        front.line(0xCC, 700, 400, 500, 550, 0, pen, Rect(600, 400, 150, 150));

//        /*Normal line */
//        front.line(0xCC, 500, 400, 700, 550, 0, pen, Rect(500, 400, 200, 150));

//        front.end_update();

//        /* Write "hello" at right-bottom white rect of screen */

//        front.begin_update();
//        this->server_set_clip(Rect(0, 0, 800, 600));
//        this->server_draw_text(500, 530, "Hello", BLACK, DARK_GREY);
//        front.end_update();

//        /* Draw a little grey rect at left-bottom side of screen */

//        front.begin_update();
//        front.pat_blt(Rect(0, 450, 50, 50), 0x5A,
//                               DARK_GREY, DARK_GREY,
//                               brush,
//                               Rect(0, 450, 50, 50));
//        front.end_update();

//        /* Draw a little rect at bottom side of screen */
//        uint32_t picture16[100][100];

//        for(int i = 0; i < 100; i++){
//            for (int j = 0; j < 100; j++){
//                picture16[i][j] = 0xFFFF00;
//            }
//        }

//        front.begin_update();

//        uint32_t cache_ref = front.bmp_cache->add_bitmap(100, 100, (uint8_t*)picture16, Rect(0, 0, 32, 32),
//                                    front.rdp_layer.client_info.bpp);

//        uint8_t send_type2 = (cache_ref >> 24);
//        uint8_t cache_b_id = (cache_ref >> 16);
//        uint16_t cache_b_idx = (cache_ref & 0xFFFF);

//        BitmapCacheItem * entry =  front.bmp_cache->get_item(cache_b_id, cache_b_idx);

//        int e = entry->pbmp->cx % 4;
//        if (e != 0) {
//            e = 4 - e;
//        }

//        LOG(LOG_INFO, "new_b_bpp=%d new_b_cx=%d new_b_cy=%d", entry->pbmp->bpp, entry->pbmp->cx, entry->pbmp->cy);
//        for (int x = 0 ; x < 100 ; x+=20){
//                unsigned char * tmp = (unsigned char*)entry->pbmp->data_co + x;
//            LOG(LOG_INFO, "%0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x",
//                tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6], tmp[7], tmp[8], tmp[9],
//                tmp[10], tmp[11], tmp[12], tmp[13], tmp[14], tmp[15], tmp[16], tmp[17], tmp[18], tmp[19]);
//        };

//        using namespace RDP;

//        if (send_type2 == BITMAP_ADDED_TO_CACHE){
//            LOG(LOG_INFO, "Sending bitmap\n");
//            RDPBmpCache bmp(entry->pbmp, cache_b_id, cache_b_idx, &front.rdp_layer.client_info);
//            // check reserved size depending on version
//            front.reserve_order(align4(entry->pbmp->cx * nbbytes(entry->pbmp->bpp)) * entry->pbmp->cy + 16);
//            bmp.emit(front.orders.out_stream);
//        }
//        front.mem_blt(cache_b_id, 0, Rect(100, 450, 32, 32), 0xcc, 0, 0, cache_b_idx, Rect(100, 450, 32, 32));
//        front.end_update();

//        /* Draw a little pink rect at bottom-right side of screen bottom-right rectangle */
//        front.begin_update();
//        front.screen_blt(0xcc, Rect(690, 540, 50, 50), 190, 190, Rect(690, 540, 50, 50));
//        front.end_update();
    }

};

#endif
