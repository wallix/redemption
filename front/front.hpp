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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   header file. Front object, used to communicate with RDP client

*/

#if !defined(__FRONT_HPP__)
#define __FRONT_HPP__

#include "log.hpp"
#include <stdlib.h>
#include <stdint.h>
#include "stream.hpp"
#include "constants.hpp"
#include "ssl_calls.hpp"
#include "file_loc.hpp"
#include <string.h>
#include <sys/time.h>
#include "orders.hpp"
#include "altoco.hpp"

#include "font.hpp"
#include "cache.hpp"
#include "font.hpp"
#include "region.hpp"
#include "capture.hpp"
#include "bitmap.hpp"
#include "bitmap_cache.hpp"
#include "capture.hpp"
#include <deque>
#include "NewRDPOrders.hpp"

class Front {
public:
    struct BitmapCache *bmp_cache;
    struct Capture * capture;
    struct RDP::Orders *orders;
    struct Font * font;
    struct Cache* cache;
    int mouse_x;
    int mouse_y;
    struct timeval start;
    bool nomouse;
    bool notimestamp;
    int timezone;

#warning mouse_x, mouse_y, start not initialized

    Front(struct RDP::Orders *orders, struct Cache* cache, Font *font, bool nomouse, bool notimestamp, int timezone)
    :
    bmp_cache(0),
    capture(0),
    orders(orders),
    font(font),
    cache(cache),
    nomouse(nomouse),
    notimestamp(notimestamp),
    timezone(timezone)
    {
        ;
    }

    ~Front(){
        if (this->capture){
            delete this->capture;
        }
        if (this->bmp_cache){
            delete this->bmp_cache;
        }
    }

    void reset(struct RDP::Orders *orders, struct Cache* cache, Font *font){
        this->font = font;
        this->cache = cache;
        this->orders = orders;
        if (this->bmp_cache){
            delete this->bmp_cache;
        }
        this->bmp_cache = new BitmapCache(&(orders->rdp_layer->client_info));
    }

    void start_capture(int width, int height, bool flag, char * path, const char * codec_id, const char * quality, int timezone)
    {
        this->timezone = timezone;
        if (flag){
            this->stop_capture();
            gettimeofday(&this->start, NULL);
            this->capture = new Capture(width, height, path, codec_id, quality);
        }
    }

    void stop_capture()
    {
        if (this->capture){
            delete this->capture;
            this->capture = 0;
        }
    }

    uint64_t difftimeval(const struct timeval endtime, const struct timeval starttime)
    {
      uint64_t sec = (endtime.tv_sec  - starttime.tv_sec ) * 1000000
                   + (endtime.tv_usec - starttime.tv_usec);
      return sec;
    }

    void periodic_snapshot(bool pointer_is_displayed)
    {
        if (this->capture){
            const uint64_t inter_frame_interval = this->capture->inter_frame_interval;
            struct timeval now;
            gettimeofday(&now, NULL);
            if (difftimeval(now, this->start) > inter_frame_interval)
            {
                this->start = now;
                this->capture->snapshot(
                    this->mouse_x, this->mouse_y,
                    pointer_is_displayed|this->nomouse,
                    this->notimestamp, this->timezone);
            }
        }
    }

    void begin_update()
    {
        this->orders->init();
    }

    void end_update()
    {
        this->orders->send();
    }


// rop values:
// 0x0 : 0
// 0x1 : ~(src | dst)
// 0x2 : (~src) & dst
// 0x3 : ~src
// 0x4 : src & (~dst)
// 0x5 : ~(dst)
// 0x6 : src ^ dst
// 0x7 : ~(src & dst)
// 0x8 : src & dst
// 0x9 : ~(src) ^ dst
// 0xA : dst
// 0xB : (~src) | dst
// 0xC : src
// 0xD : src | (~dst)
// 0xE : src | dst
// 0xF : ~0

    void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error)
    {
//        LOG(LOG_INFO, "front::send_pointer\n");
        this->orders->rdp_layer->server_rdp_send_pointer(cache_idx, data, mask, x, y);
//        LOG(LOG_INFO, "front::send_pointer done\n");
    }

    void set_pointer(int cache_idx) throw (Error)
    {
//        LOG(LOG_INFO, "front::set_pointer\n");
        this->orders->rdp_layer->server_rdp_set_pointer(cache_idx);
//        LOG(LOG_INFO, "front::set_pointer done\n");
    }


    int get_channel_id(char* name)
    {
//        LOG(LOG_INFO, "front::get_channel_id\n");
        return this->orders->rdp_layer->sec_layer.mcs_layer.server_mcs_get_channel_id(name);
    }


    /* fill in an area of the screen with one color */
    void opaque_rect(const Rect & r, int fgcolor, const Rect & clip)
    {
//      LOG(LOG_INFO, "this->front.orders->opaque_rect(Rect(%d, %d, %d, %d), 0x%.6x, Rect(%d, %d, %d, %d));", r.x, r.y, r.cx, r.cy, fgcolor, clip.x, clip.y, clip.cx, clip.cy);

        if (!clip.isempty() && !clip.intersect(r).isempty()){
            this->orders->opaque_rect(r, fgcolor, clip);
            if (this->capture){
                this->capture->rect(r, fgcolor, this->orders->rdp_layer->client_info.bpp, clip);
            }
        }
    }

    #warning unify API with orders
    void screen_blt(int rop, const Rect & r, int srcx, int srcy, const Rect &clip)
    {
//      LOG(LOG_INFO, "this->front.orders->screen_blt(Rect(%d, %d, %d, %d), %d, %d, 0x.2x, Rect(%d, %d, %d, %d));", r.x, r.y, r.cx, r.cy, srcx, srcy, rop, clip.x, clip.y, clip.cx, clip.cy);

//        LOG(LOG_INFO, "front::screen_blt\n");
        if (!clip.isempty() && !clip.intersect(r).isempty()){
            // this one is used when dragging a visible window on screen
            this->orders->screen_blt(r, srcx, srcy, rop, clip);
            if (this->capture){
                this->capture->screen_blt(r, srcx, srcy, rop, clip);
            }
        }
    }

    void dest_blt(const Rect & r, int rop, const Rect &clip)
    {
//      LOG(LOG_INFO, "this->front.orders->dest_blt(Rect(%d, %d, %d, %d), 0x%.2x, Rect(%d, %d, %d, %d));", r.x, r.y, r.cx, r.cy, rop, clip.x, clip.y, clip.cx, clip.cy);

        if (!clip.intersect(r).isempty()){
            this->orders->dest_blt(r, rop, clip);
            if (this->capture){
                #warning missing code in capture, apply some logical operator inplace
                this->capture->rect(r, WHITE, this->orders->rdp_layer->client_info.bpp, clip);
            }
        }
    }


    void pat_blt(const Rect & r, int rop, uint32_t bg_color,  uint32_t fg_color, const RDPBrush & brush, const Rect &clip)
    {
//      LOG(LOG_INFO, "this->front.orders->pat_blt(Rect(%d, %d, %d, %d), 0x%.2x, 0x%.6x, 0x%.6x, this->brush, Rect(%d, %d, %d, %d));", r.x, r.y, r.cx, r.cy, rop, bg_color, fg_color, clip.x, clip.y, clip.cx, clip.cy);

        if (!clip.intersect(r).isempty()){
            this->orders->pat_blt(r, rop, bg_color, fg_color, brush, clip);
            if (this->capture){
                this->capture->rect(r, fg_color, this->orders->rdp_layer->client_info.bpp, clip);
            }
        }
    }


    void mem_blt(int cache_id,
                 int color_table, const Rect & r,
                 int rop,
                 int srcx, int srcy,
                 int cache_idx, const Rect & clip)
    {
//      LOG(LOG_INFO, "this->front.orders->mem_blt(%d, %d, Rect(%d, %d, %d, %d), 0x%.2x, %d, %d, %d, Rect(%d, %d, %d, %d));", cache_id, color_table, r.x, r.y, r.cx, r.cy, rop, srcx, srcy, cache_idx, clip.x, clip.y, clip.cx, clip.cy);

        if (!clip.intersect(r).isempty()){
            this->orders->mem_blt(cache_id, color_table, r, rop, srcx, srcy, cache_idx, clip);
            if (this->capture){
                BitmapCacheItem * entry =  this->bmp_cache->get_item(cache_id, cache_idx);
                this->capture->mem_blt(cache_id, color_table, r, rop, entry->pbmp->bpp, entry->pbmp->data_co, srcx, srcy, cache_idx, clip);
            }
        }
    }

    #warning harmonize name of function -> line_to
    void line(int rop, int x1, int y1, int x2, int y2, int bgcolor, const RDPPen & pen, const Rect & clip)
    {
//        LOG(LOG_INFO, "front::line\n");
        #warning if direction of line is inverted, put it back in the right order, for now just ignore lines in wrong direction
        if (x1 >= x2 && y1 >= y2
        && !clip.intersect(Rect(x1, y1, (x2 - x1) +1, (y2 - y1)+1)).isempty()){
            if (!clip.intersect(Rect(x1, y1, (x2 - x1) +1, (y2 - y1)+1)).isempty()){
                uint32_t rop2 = rop;
                if ((rop < 1) || (rop > 0x10)) {
                    rop2 = 0x0d; /* R2_COPYPEN */
                }
                this->orders->line_to(1, x1, y1, x2, y2, rop2, bgcolor, pen, clip);
                if (this->capture){
                    this->capture->line(1, x1, y1, x2, y2, rop2, bgcolor, pen, this->orders->rdp_layer->client_info.bpp, clip);
                }
            }

        }
    }

    #warning harmonize name of function -> glyph_index
    void draw_text2(int font, int flags, int mixmode,
                const Rect & box, const Rect & clip_rect,
                int x, int y, uint8_t* data, int data_len,
                int fgcolor, int bgcolor, const Rect & draw_rect)
    {
//        LOG(LOG_INFO, "front::draw_text2\n");
        if (draw_rect.intersect(clip_rect).isempty()){
            return;
        }

        this->orders->glyph_index(font, flags, mixmode,
                            fgcolor, bgcolor,
                            clip_rect, box,
                            x, y, data, data_len, draw_rect);
        if (this->capture){
            this->capture->text(font, flags, mixmode,
                            fgcolor, bgcolor,
                            clip_rect, box,
                            x, y, data, data_len, draw_rect);
        }
    }

    void send_palette(const RGBPalette & palette)
    {
//        LOG(LOG_INFO, "front::send_palette\n");
        if (this->orders->rdp_layer->client_info.bpp <= 8) {
            this->orders->rdp_layer->server_send_palette(palette);
            this->orders->init();
            this->orders->send_palette(palette, 0);
            this->orders->send();
        }
//        LOG(LOG_INFO, "front::send_palette done\n");
    }

    void send_brush(const int index)
    {
        this->orders->send_brush(8, 8, 1, 0x81, 8, this->cache->brush_items[index].pattern, index);

    }


    void send_bitmap_common(const uint8_t cache_id, const uint16_t cache_idx)
    {
        BitmapCacheItem * entry =  this->bmp_cache->get_item(cache_id, cache_idx);
        this->orders->send_bitmap_common(*entry->pbmp, cache_id, cache_idx);
    }

    // draw bitmap from src_data (image rect contained in src_r) to x, y
    // clip_region is the list of visible rectangles that should be sent
    void send_bitmap_front(const Rect & dst, const Rect & src_r, const uint8_t rop, const uint8_t * src_data,
                     int palette_id,
                     const Rect & clip)
    {
//        LOG(LOG_INFO, "front::send_bitmap_front bpp=%d\n", this->orders->rdp_layer->client_info.bpp);
        for (int y = 0; y < dst.cy ; y += 64) {
            int cy = std::min(64, dst.cy - y);
            for (int x = 0; x < dst.cx ; x += 64) {
                int cx = std::min(64, dst.cx - x);
                const Rect tile(x, y, cx, cy);
                if (!clip.intersect(tile.offset(dst.x, dst.y)).isempty()){
                     uint32_t cache_ref = this->bmp_cache->add_bitmap(
                                                src_r.cx, src_r.cy,
                                                src_data,
                                                tile.offset(src_r.x, src_r.y),
                                                this->orders->rdp_layer->client_info.bpp);

                    uint8_t send_type = (cache_ref >> 24);
                    uint8_t cache_id  = (cache_ref >> 16);
                    uint16_t cache_idx = (cache_ref & 0xFFFF);

                    if (send_type == BITMAP_ADDED_TO_CACHE){
                        this->send_bitmap_common(cache_id, cache_idx);
                    };


                    this->mem_blt(cache_id, palette_id,
                                  tile.offset(dst.x, dst.y), rop,
                                  0, 0, cache_idx, clip);
                }
            }
        }
    }

    void send_bitmap_front2(const Rect & dst, const Rect & src_r, const uint8_t rop, const uint8_t * src_data,
                     int palette_id,
                     const Rect & clip)
    {
        this->send_bitmap_front(dst, src_r, rop, src_data, palette_id, clip);
    }

    #warning harmonize names with orders send_glyph or send_font
    void send_glyph(int font, int character,
                    int offset, int baseline,
                    int width, int height, const uint8_t* data)
    {
//        LOG(LOG_INFO, "front::send_glyph\n");
        struct FontChar fi(offset, baseline, width, height, 0);

        memcpy(fi.data, data, fi.datasize());
        this->send_glyph(&fi, font, character);
    }

    void send_glyph(FontChar* font_char, int font_index, int char_index)
    {
//        LOG(LOG_INFO, "front::send_glyph 2\n");
        this->orders->send_font(font_char, font_index, char_index);
    }

};

#endif
