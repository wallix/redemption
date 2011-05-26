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

   Bouncer test

*/

#if !defined(__BOUNCER_HPP__)
#define __BOUNCER_HPP__

#define DONT_CAPTURE_MOVIE true
#define COLOR_IMAGE FIXTURES_PATH "/color_image2.bmp"
#define BACKGROUND true

typedef enum {
    STATE_RSA_KEY_HANDSHAKE,
    STATE_ENTRY,
    STATE_RUNNING,
    STATE_STOP
} t_internal_state;


typedef enum {
    NOT_STATE,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    LEFT_WALLIX,
    RIGHT_WALLIX,
    TOP_WALLIX,
    BOTTOM_WALLIX
} bouncing_state;

struct bouncer_mod : public internal_mod {
    Session * session;
    Front & front;
    Bitmap bmp_background;
    Bitmap bmp;
    Bitmap bmp_wallix;
    bouncing_state state;
    bouncing_state wallix_state;
    int index;
    int src_x;
    int src_y;
    int src_x_wallix;
    int src_y_wallix;

    bouncer_mod(
        wait_obj * event,
        int (& keys)[256], int & key_flags, Keymap * &keymap,
        ModContext & context, Front & front, Session * session):
            internal_mod(keys, key_flags, keymap, front),
            front(front),
            bmp_background(COLOR_IMAGE, front.rdp_layer->client_info.bpp),
            bmp(SHARE_PATH "/ad24b.bmp", front.rdp_layer->client_info.bpp),
            bmp_wallix(SHARE_PATH "/ad24b.bmp", front.rdp_layer->client_info.bpp),
            state(LEFT),
            wallix_state(LEFT_WALLIX),
            index(0),
            src_x(100),
            src_y(100),
            src_x_wallix(200),
            src_y_wallix(300)
    {
        this->event = event;
        front.begin_update();
        front.send_bitmap_front(
            Rect(0, 0, this->bmp_background.cx, this->bmp_background.cy),
            Rect(0, 0, this->bmp_background.cx, this->bmp_background.cy),
            0xCC,
            (uint8_t*)this->bmp_background.data_co,
            0, Rect(0, 0, 800, 600));
        front.screen_blt(0xcc, Rect(0,0,800,600), 0, 0, Rect(0,0,800,600));
        front.end_update();
        event->set();
    }

    virtual ~bouncer_mod()
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
        #warning simplify bouncer below, it bounce but is much too complicated
        //LOG(LOG_DEBUG, "Index value is : %d\n", index);
        const Rect dst_r = Rect(src_x + 10, src_y + 10, 140, 140);
        Rect rect(src_x, src_y, 140, 140);
        Rect rectang = rect.intersect(dst_r);
        Rect fill_rect1 = Rect(src_x, src_y, 10, 140);
        Rect fill_rect2 = Rect(src_x, src_y, 140, 10);

        const Rect dst_r_wallix = Rect(src_x_wallix + 10, src_y_wallix + 10, 140, 140);
        Rect fill_rect_wallix = Rect(src_x_wallix, src_y_wallix, 10, 140);
        Rect fill_rect_wallix_h = Rect(src_x_wallix, src_y_wallix, 140, 10);

        uint32_t cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x - 10, src_y, 10, 140), this->front.rdp_layer->client_info.bpp);

        uint8_t cache_b_id = (cache_ref >> 16);
        uint16_t cache_b_idx = (cache_ref & 0xFFFF);

        cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x, src_y - 10, 140, 10), this->front.rdp_layer->client_info.bpp);

        uint8_t cache_b_id_h = (cache_ref >> 16);
        uint16_t cache_b_idx_h = (cache_ref & 0xFFFF);

        cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x_wallix, src_y_wallix, 10, 140), this->front.rdp_layer->client_info.bpp);

        uint8_t cache_b_id_wallix = (cache_ref >> 16);
        uint16_t cache_b_idx_wallix = (cache_ref & 0xFFFF);

        cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x_wallix, src_y_wallix - 10, 140, 10), this->front.rdp_layer->client_info.bpp);

        uint8_t cache_b_id_h_wallix = (cache_ref >> 16);
        uint16_t cache_b_idx_h_wallix = (cache_ref & 0xFFFF);

        switch (state){
            case BOTTOM:
                fill_rect1 = Rect(src_x , src_y, 10, 140);
                fill_rect2 = Rect(src_x , src_y + 140, 140, 10);
                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x , src_y, 10, 140), this->front.rdp_layer->client_info.bpp);
                cache_b_id = (cache_ref >> 16);
                cache_b_idx = (cache_ref & 0xFFFF);

                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x , src_y + 140, 140, 10), this->front.rdp_layer->client_info.bpp);

                cache_b_id_h = (cache_ref >> 16);
                cache_b_idx_h = (cache_ref & 0xFFFF);

            break;
            case RIGHT:
                fill_rect1 = Rect(src_x + 140, src_y, 10, 140);
                fill_rect2 = Rect(src_x + 10, src_y + 140, 140, 10);
                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x + 140, src_y, 10, 140), this->front.rdp_layer->client_info.bpp);

                cache_b_id = (cache_ref >> 16);
                cache_b_idx = (cache_ref & 0xFFFF);

                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x + 10, src_y + 140, 140, 10), this->front.rdp_layer->client_info.bpp);
                cache_b_id_h = (cache_ref >> 16);
                cache_b_idx_h = (cache_ref & 0xFFFF);
            break;
            default:
            break;
            case TOP:
                fill_rect1 = Rect(src_x, src_y, 140, 10);
                fill_rect2 = Rect(src_x + 140, src_y, 20, 140);

                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x, src_y, 140, 10), this->front.rdp_layer->client_info.bpp);
                cache_b_id = (cache_ref >> 16);
                cache_b_idx = (cache_ref & 0xFFFF);

                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x + 140, src_y, 20, 140), this->front.rdp_layer->client_info.bpp);
                cache_b_id_h = (cache_ref >> 16);
                cache_b_idx_h = (cache_ref & 0xFFFF);
            break;
        }
        switch (wallix_state){
            case BOTTOM_WALLIX:
                fill_rect_wallix = Rect(src_x_wallix, src_y_wallix, 10, 140);
                fill_rect_wallix_h = Rect(src_x_wallix, src_y_wallix + 140, 140, 10);

                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x_wallix , src_y_wallix, 10, 140), this->front.rdp_layer->client_info.bpp);
                cache_b_id_wallix = (cache_ref >> 16);
                cache_b_idx_wallix = (cache_ref & 0xFFFF);
                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x_wallix , src_y_wallix + 140, 140, 10), this->front.rdp_layer->client_info.bpp);
                cache_b_id_h_wallix = (cache_ref >> 16);
                cache_b_idx_h_wallix = (cache_ref & 0xFFFF);
            break;
            case RIGHT_WALLIX:
                fill_rect_wallix = Rect(src_x_wallix + 140, src_y_wallix, 10, 140);
                fill_rect_wallix_h = Rect(src_x_wallix + 10, src_y_wallix + 140, 140, 10);

                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x_wallix + 140 , src_y_wallix, 10, 140), this->front.rdp_layer->client_info.bpp);
                cache_b_id_wallix = (cache_ref >> 16);
                cache_b_idx_wallix = (cache_ref & 0xFFFF);
                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x_wallix + 10, src_y_wallix + 140, 140, 10), this->front.rdp_layer->client_info.bpp);
                cache_b_id_h_wallix = (cache_ref >> 16);
                cache_b_idx_h_wallix = (cache_ref & 0xFFFF);
            break;
            default:
            break;
            case TOP_WALLIX:
                fill_rect_wallix = Rect(src_x_wallix, src_y_wallix, 140, 10);
                fill_rect_wallix_h = Rect(src_x_wallix + 140, src_y_wallix, 20, 140);
                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x_wallix , src_y_wallix, 140, 10), this->front.rdp_layer->client_info.bpp);
                cache_b_id_wallix = (cache_ref >> 16);
                cache_b_idx_wallix = (cache_ref & 0xFFFF);
                cache_ref = front.bmp_cache->add_bitmap(800, 600, this->bmp_background.data_co, Rect(src_x_wallix + 140, src_y_wallix, 20, 140), this->front.rdp_layer->client_info.bpp);
                cache_b_id_h_wallix = (cache_ref >> 16);
                cache_b_idx_h_wallix = (cache_ref & 0xFFFF);
            break;
        }

        front.begin_update();
        front.send_bitmap_front(
            Rect(src_x, src_y, bmp.cx, bmp.cy),
            Rect(0, 0, bmp.cx, bmp.cy),
            0xCC,
            bmp.data_co, 0, Rect(0, 0, 800, 600));
        front.send_bitmap_front(
            Rect(src_x_wallix , src_y_wallix, bmp_wallix.cx, bmp_wallix.cy),
            Rect(0, 0, bmp_wallix.cx, bmp_wallix.cy),
            0xCC,
            bmp_wallix.data_co, 0, Rect(0, 0, 800, 600));
        front.end_update();

        this->front.orders.init();

        BitmapCacheItem * entry_b = front.bmp_cache->get_item(cache_b_id, cache_b_idx);
        {
            RDPBmpCache bmp(entry_b->pbmp, cache_b_id, cache_b_idx, &this->front.rdp_layer->client_info);
        // check reserved size depending on version
            this->front.reserve_order(align4(entry_b->pbmp->cx * nbbytes(entry_b->pbmp->bpp)) * entry_b->pbmp->cy + 16);
            bmp.emit(this->front.orders.out_stream);
        }

        BitmapCacheItem * entry_b_h = front.bmp_cache->get_item(cache_b_id_h, cache_b_idx_h);
        {
            RDPBmpCache bmp(entry_b_h->pbmp, cache_b_id_h, cache_b_idx_h, &this->front.rdp_layer->client_info);
        // check reserved size depending on version
            this->front.reserve_order(align4(entry_b_h->pbmp->cx * nbbytes(entry_b_h->pbmp->bpp)) * entry_b_h->pbmp->cy + 16);
            bmp.emit(this->front.orders.out_stream);
        }

        BitmapCacheItem * entry_b_wallix = front.bmp_cache->get_item(cache_b_id_wallix, cache_b_idx_wallix);
        {
            RDPBmpCache bmp(entry_b_wallix->pbmp, cache_b_id_wallix, cache_b_idx_wallix, &this->front.rdp_layer->client_info);
           // check reserved size depending on version
            front.reserve_order(align4(entry_b_wallix->pbmp->cx * nbbytes(entry_b_wallix->pbmp->bpp)) * entry_b_wallix->pbmp->cy + 16);
            bmp.emit(this->front.orders.out_stream);
        }

        BitmapCacheItem * entry_b_h_wallix = front.bmp_cache->get_item(cache_b_id_h_wallix, cache_b_idx_h_wallix);
        {
            RDPBmpCache bmp(entry_b_h_wallix->pbmp, cache_b_id_h_wallix, cache_b_idx_h_wallix, &this->front.rdp_layer->client_info);
        // check reserved size depending on version
            this->front.reserve_order(align4(entry_b_h_wallix->pbmp->cx * nbbytes(entry_b_h_wallix->pbmp->bpp)) * entry_b_h_wallix->pbmp->cy + 16);
            bmp.emit(this->front.orders.out_stream);
        }
        this->front.send();

        front.begin_update();
        front.mem_blt(cache_b_id, 0, fill_rect1, 0xcc, 0, 0, cache_b_idx,fill_rect1);
        front.mem_blt(cache_b_id_h, 0, fill_rect2, 0xcc, 0, 0, cache_b_idx_h, fill_rect2);
        front.mem_blt(cache_b_id_wallix, 0, fill_rect_wallix, 0xcc, 0, 0, cache_b_idx_wallix,fill_rect_wallix);
        front.mem_blt(cache_b_id_h_wallix, 0, fill_rect_wallix_h, 0xcc, 0, 0, cache_b_idx_h_wallix, fill_rect_wallix_h);
        front.end_update();

        src_x = src_x + 10;
        src_y = src_y + 10;
        src_x_wallix = src_x_wallix + 10;
        src_y_wallix = src_y_wallix + 10;

        if (((((src_y_wallix + 140) >= 600) && ((src_x_wallix + 140) < 800)) && (src_x_wallix > 0)) || (wallix_state == BOTTOM_WALLIX)){
            src_y_wallix = src_y_wallix - 20;
            wallix_state = BOTTOM_WALLIX;
        }
        if (((src_y_wallix > 0) && ((src_y_wallix + 140) < 600) && ((src_x_wallix + 140) >= 800)) || (wallix_state == RIGHT_WALLIX)){
            src_x_wallix = src_x_wallix - 20;
            src_y_wallix = src_y_wallix - 20;
            wallix_state = RIGHT_WALLIX;
        }

        if (((src_y_wallix  <= 10) && ((src_x_wallix + 140) < 800) && (src_x_wallix > 0)) || (wallix_state == TOP_WALLIX)){
            src_x_wallix = src_x_wallix - 20;
            wallix_state = TOP_WALLIX;
        }
        if (((src_y_wallix  > 0) && ((src_y_wallix + 140) < 600) && (src_x_wallix <= 10)) || (wallix_state == LEFT_WALLIX)){
            wallix_state = LEFT_WALLIX;
        }
        if (((((src_y + 140) >= 600) && ((src_x + 140) < 800)) && (src_x > 0)) || (state == BOTTOM)){
            src_y = src_y - 20;
            state = BOTTOM;
        }
        if (((src_y > 0) && ((src_y + 140) < 600) && ((src_x + 140) >= 800)) ||  (state == RIGHT)){
            src_x = src_x - 20;
            src_y = src_y - 20;
            state = RIGHT;
        }
        if (((src_y  <= 10) && ((src_x + 140) < 800) && (src_x > 0)) ||  (state == TOP)){
            src_x = src_x - 20;
            state = TOP;
        }

        if (((src_y  > 0) && ((src_y + 140) < 600) && (src_x <= 10)) ||(state == LEFT)){
            state = LEFT;
        }
        LOG(LOG_INFO, "out of mod_signal");
        this->event->set();
        return 0;
    }

};

#endif
