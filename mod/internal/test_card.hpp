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
    #warning only works with 24 bits color mode, fix that
    test_card_mod(
        wait_obj * event,
        int (& keys)[256], int & key_flags, Keymap * &keymap,
        ModContext & context, Front & front, Session * session):
            internal_mod(keys, key_flags, keymap, front)
    {
        this->event = event;

        front.orders->init();
        front.orders->opaque_rect(Rect(50, 50, 150, 150), PINK, Rect(50, 50, 150, 150));
        front.orders->opaque_rect(Rect(520, 360, 60, 25), DARK_GREY, Rect(520, 360, 60, 25));

        // add text to glyph cache
        const char * text = "Hello";
        int len = mbstowcs(0, text, 0);
        wchar_t* wstr = new wchar_t[len + 2];
        mbstowcs(wstr, text, len + 1);
        int cx = 0;
        int cy = 0;
        uint8_t *data = new uint8_t[len * 4];
        memset(data, 0, len * 4);
        int f = 0;
        int c = 0;
        int k = 0;
        for (int index = 0; index < len; index++) {
            FontChar* font_item = front.font->font_items[wstr[index]];
            switch (front.cache->add_glyph(font_item, f, c))
            {
                case Cache::GLYPH_ADDED_TO_CACHE:
                    front.orders->send_font(font_item, f, c);
                break;
                default:
                break;
            }
            data[index * 2] = c;
            data[index * 2 + 1] = k;
            k = font_item->incby;
            cx += k;
            cy = std::max(cy, font_item->height);
        }

        front.orders->glyph_index(7, 3, BLACK, YELLOW, 0, Rect(535, 363, cx+1, cy+1), Rect(0, 0, 0, 0), 536, 364+cy, data, len * 2, Rect(0, 0, 800, 600));

        //front.orders->screen_blt(Rect(190, 190, 50, 50), 50, 50, 0xCC, Rect(190, 190, 50, 50));

        front.orders->dest_blt(Rect(60, 60, 50, 50), 0, Rect(60, 60, 50, 50));

        front.orders->send();

        front.begin_update();
        RDPBrush brush;
        brush.org_x = 0;
        brush.org_y = 0;
        brush.style = 3;

        LOG(LOG_INFO, "6666");

        if (front.orders->rdp_layer->client_info.brush_cache_code == 1) {
            uint8_t pattern[8] = {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55};
            int cache_idx = 0;
            if (BRUSH_TO_SEND == front.cache->add_brush(pattern, cache_idx)){
                front.send_brush(cache_idx);
            }
            brush.hatch = cache_idx;
            brush.style = 0x81;
        }
        front.end_update();

        LOG(LOG_INFO, "7777");

        front.orders->init();

        front.orders->pat_blt(
            Rect(50, 50, 150, 150),
            0x5A,
            PINK,
            WHITE,
            brush,
            Rect(100, 100, 50, 50));

        front.orders->pat_blt(
            Rect(63, 78, 10, 13),
            0x5A,
            BLACK,
            WHITE,
            brush,
            Rect(63, 78, 10, 13));

        front.orders->send();

        uint32_t picture24[100][100];

        for(int i = 0; i < 100; i++){
            for (int j = 0; j < 100; j++){
                picture24[i][j] = WHITE;
            }
        }

        LOG(LOG_INFO, "BBBB");

        front.orders->init();

        RDPPen pen;
        pen.style = 1;
        pen.width = 10;
        pen.color = BLUE;

        front.orders->line_to(1, 50, 50, 150, 150, 0x0D, PINK, pen, Rect(100, 100, 50, 50));

        front.orders->send();

        LOG(LOG_INFO, "BBBB");

        cout << "testing front primitives\n";

        /* Draw a big white rect at right-bottom side of screen */
        front.begin_update();
        Region region;
        Rect r1(0, 0, 800, 600);
        region.rects.push_back(r1);
        front.opaque_rect(Rect(500, 400, 200, 150), DARK_GREY, Rect(500, 400, 200, 150));
        front.end_update();

        /* Draw black line crossing right-bottom white rect of screen */

        front.begin_update();

        pen.style = 1;
        pen.width = 10;
        pen.color = PINK;

        LOG(LOG_INFO, "CCCC");

        /* Line clipped 500, 400, 201, 151*/
        front.line(0xCC, 700, 400, 500, 550, 0, pen, Rect(600, 400, 150, 150));

        /*Normal line */
        front.line(0xCC, 500, 400, 700, 550, 0, pen, Rect(500, 400, 200, 150));

        front.end_update();

        LOG(LOG_INFO, "DDDDD");

        /* Write "hello" at right-bottom white rect of screen */

        front.begin_update();

        front.draw_text2(7, 3, 0,
                    Rect(0, 0, 0, 0), Rect(500, 530, cx+1, cy+1),
                    501, 531+cy, data, len * 2,
                    BLACK, DARK_GREY, Rect(0, 0, 800, 600));
        front.end_update();

        /* Draw a little grey rect at left-bottom side of screen */

        front.begin_update();
        front.pat_blt(Rect(0, 450, 50, 50), 0x5A,
                               DARK_GREY, DARK_GREY,
                               brush,
                               Rect(0, 450, 50, 50));
        front.end_update();

        LOG(LOG_INFO, "EEEEE");

        /* Draw a little rect at bottom side of screen */
        uint32_t picture16[100][100];

        for(int i = 0; i < 100; i++){
            for (int j = 0; j < 100; j++){
                picture16[i][j] = 0xFFFF00;
            }
        }

        LOG(LOG_INFO, "FFFFF");

        front.begin_update();

        uint32_t cache_ref = front.bmp_cache->add_bitmap(100, 100, (uint8_t*)picture16, 0, 0, 32, 32,
                                    front.orders->rdp_layer->client_info.bpp);

        uint8_t send_type2 = (cache_ref >> 24);
        uint8_t cache_b_id = (cache_ref >> 16);
        uint16_t cache_b_idx = (cache_ref & 0xFFFF);

        BitmapCacheItem * entry =  front.bmp_cache->get_item(cache_b_id, cache_b_idx);

        int e = entry->pbmp->cx % 4;
        if (e != 0) {
            e = 4 - e;
        }

        LOG(LOG_INFO, "GGGGG");

        LOG(LOG_INFO, "new_b_bpp=%d new_b_cx=%d new_b_cy=%d", entry->pbmp->bpp, entry->pbmp->cx, entry->pbmp->cy);
        for (int x = 0 ; x < 100 ; x+=20){
                unsigned char * tmp = (unsigned char*)entry->pbmp->data_co + x;
            LOG(LOG_INFO, "%0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x",
                tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6], tmp[7], tmp[8], tmp[9],
                tmp[10], tmp[11], tmp[12], tmp[13], tmp[14], tmp[15], tmp[16], tmp[17], tmp[18], tmp[19]);
        };

        using namespace RDP;

        if (send_type2 == BITMAP_ADDED_TO_CACHE){
            LOG(LOG_INFO, "Sending bitmap\n");
            RDPBmpCache bmp(entry->pbmp, cache_b_id, cache_b_idx, &front.orders->rdp_layer->client_info);
            // check reserved size depending on version
            front.orders->reserve_order(align4(entry->pbmp->cx * nbbytes(entry->pbmp->bpp)) * entry->pbmp->cy + 16);
            bmp.emit(front.orders->out_stream);
        }
        front.mem_blt(cache_b_id, 0, Rect(100, 450, 32, 32), 0xcc, 0, 0, cache_b_idx, Rect(100, 450, 32, 32));
        front.end_update();

        /* Draw a little pink rect at bottom-right side of screen bottom-right rectangle */
        front.begin_update();
        front.screen_blt(0xcc, Rect(690, 540, 50, 50), 190, 190, Rect(690, 540, 50, 50));
        front.end_update();
    }

    virtual ~test_card_mod()
    {
    }

    // event from front (keyboard or mouse event)
    virtual int mod_event(int msg, long x, long y, long param4, long param5)
    {
        return 0;
    }

    // event from back en (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual int mod_signal()
    {
        return 0;
    }

};

#endif
