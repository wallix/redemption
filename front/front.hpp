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
#include "widget.hpp"
#include "region.hpp"
#include "capture.hpp"
#include "bitmap.hpp"
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
    Colors & colors;
    RGBPalette & palette;
    int mouse_x;
    int mouse_y;
    struct timeval start;
    bool nomouse;
    bool notimestamp;
    int timezone;

#warning mouse_x, mouse_y, start not initialized

    Front(struct RDP::Orders *orders, struct Cache* cache, Font *font, Colors & colors, RGBPalette & palette, bool nomouse, bool notimestamp, int timezone)
    :
    bmp_cache(0),
    capture(0),
    orders(orders),
    font(font),
    cache(cache),
    colors(colors),
    palette(palette),
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

    void start_capture(int width, int height, bool flag, char * path, const char * codec_id, const char * quality)
    {
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

    void periodic_snapshot(bool pointer_is_displayed)
    {
        if (this->capture){
            const long inter_frame_interval = this->capture->inter_frame_interval;
            struct timeval now;
            gettimeofday(&now, NULL);
            if ((now.tv_sec > start.tv_sec)
            || ((now.tv_sec == start.tv_sec) && (now.tv_usec - start.tv_usec > inter_frame_interval))){
                // increment usec by 200ms
                start.tv_usec += inter_frame_interval;
                if (start.tv_usec > 1000000){
                    start.tv_sec++;
                    start.tv_usec -= 1000000;
                }
                // if we are still late, drop frames
                if ((now.tv_sec > start.tv_sec)
                || ((now.tv_sec  == start.tv_sec) && (now.tv_usec > start.tv_usec + inter_frame_interval))){
                    start = now;
                }
                else {
                    // ok, we are not late emit frame
                    this->capture->snapshot(
                        this->mouse_x, this->mouse_y,
                        pointer_is_displayed|this->nomouse,
                        this->notimestamp, this->timezone);
                }

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
        this->orders->rdp_layer->server_rdp_send_pointer(cache_idx, data, mask, x, y);
    }

    void set_pointer(int cache_idx) throw (Error)
    {
        this->orders->rdp_layer->server_rdp_set_pointer(cache_idx);
    }

    void screen_blt(int rop, const Rect & r, int srcx, int srcy, const Rect &clip)
    {
        if (!clip.isempty() && !clip.intersect(r).isempty()){
            // this one is used when dragging a visible window on screen
            this->orders->screen_blt(r, srcx, srcy, rop, clip);
            if (this->capture){
                this->capture->screen_blt(r, srcx, srcy, rop, clip);
            }
        }
    }

    void send_palette()
    {
        if (this->orders->rdp_layer->client_info.bpp <= 8) {
            if (this->orders->order_count > 0){
                this->orders->force_send();
            }
            RGBPalette palette;
            this->colors.get_palette(palette);
            this->orders->rdp_layer->server_send_palette(palette);
            this->orders->init();
            this->orders->send_palette(palette, 0);
            this->orders->send();
        }
    }


    void send_palette(const RGBPalette & palette)
    {
        if (this->orders->rdp_layer->client_info.bpp <= 8
        && memcmp(this->palette, palette, 255 * sizeof(RGBcolor)) != 0) {
           memcpy(this->palette, palette, 256 * sizeof(RGBcolor));
            if (this->orders->rdp_layer->client_info.bpp <= 8) {
                if (this->orders->order_count > 0){
                    this->orders->force_send();
                }
                this->orders->rdp_layer->server_send_palette(palette);
                this->orders->init();
                this->orders->send_palette(palette, 0);
                this->orders->send();
            }
        }
    }


    void send_glyph(int font, int character,
                    int offset, int baseline,
                    int width, int height, const uint8_t* data)
    {
        struct FontChar fi(offset, baseline, width, height, 0);

        memcpy(fi.data, data, fi.datasize());
        this->send_glyph(&fi, font, character);
    }

    int send_glyph(FontChar* font_char, int font_index, int char_index)
    {
        this->orders->send_font(font_char, font_index, char_index);
        return 0;
    }

    int get_channel_id(char* name)
    {
        return this->orders->rdp_layer->sec_layer.mcs_layer.server_mcs_get_channel_id(name);
    }

    void mem_blt(int cache_id,
                 int color_table, const Rect & r,
                 int rop,
                 int bpp, uint8_t * data,
                 int srcx, int srcy,
                 int cache_idx, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(r).isempty()){
            this->orders->mem_blt(cache_id, color_table, r, rop, srcx, srcy, cache_idx, clip);
            if (this->capture){
                this->capture->mem_blt(cache_id, color_table, r, rop, bpp,
                          data, srcx, srcy, cache_idx, clip);
            }
        }
    }


    // draw bitmap from src_data (image rect contained in src_r) to x, y
    // clip_region is the list of visible rectangles that should be sent
    void send_bitmap_front(const Rect & dst, const Rect & src_r, const uint8_t * src_data,
                     const Colors & colors,
                     int palette_id,
                     const Rect & clip)
    {
        for (int j = 0; j < dst.cy ; j += 64) {
            int h = std::min(64, dst.cy - j);
            for (int i = 0; i < dst.cx ; i+= 64) {
                int w = std::min(64, dst.cx - i);
                const Rect rect1(dst.x + i, dst.y + j, w, h);
                const Rect & draw_rect = clip.intersect(rect1);
                if (!draw_rect.isempty()){
                    uint8_t cache_id;
                    uint16_t cache_idx;

                    uint8_t send_type = this->bmp_cache->add_bitmap(
                                                src_r.cx, src_r.cy,
                                                src_data,
                                                i + src_r.x,
                                                j + src_r.y,
                                                w, h,
                                                colors.bpp,
                                                cache_id, cache_idx);

                    BitmapCacheItem * entry =  this->bmp_cache->get_item(cache_id, cache_idx);

                    if (send_type == BITMAP_ADDED_TO_CACHE){
                        this->orders->send_bitmap_common(entry->bmp, cache_id, cache_idx);
                    };

                    this->mem_blt(cache_id, palette_id,
                                  rect1, 0xcc,
                                  entry->bmp.bpp,
                                  entry->bmp.data_co,
                                  0, 0, cache_idx, clip);
                }
            }
        }
    }

    void pat_blt(const Rect & r, int rop, uint32_t bg_color,
                uint32_t fg_color, const RDPBrush & brush,
                const Rect &clip)
    {
        if (!clip.intersect(r).isempty()){
            this->orders->pat_blt(r, rop, bg_color, fg_color, brush, clip);
        }
    }

    /*****************************************************************************/
    /* fill in an area of the screen with one color */
    void opaque_rect(const Rect & r, int fgcolor, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(r).isempty()){
            this->orders->opaque_rect(r, fgcolor, clip);
            if (this->capture){
                this->capture->rect(r, fgcolor, this->colors.bpp, clip);
            }
        }
    }

    /*****************************************************************************/
    /* fill in an area of the screen with one color and operator rop*/
    void fill_rect_rop(int rop, const Rect & r, int fgcolor, int bgcolor, const RDPBrush & brush, const Rect & clip)
    {
        this->orders->pat_blt(r, rop, bgcolor, fgcolor, brush, clip);
        if (this->capture){
            this->capture->rect(r, fgcolor, this->colors.bpp, clip);
        }
    }


    /*****************************************************************************/
    void draw_text2(int font, int flags, int mixmode,
                const Rect & box, const Rect & clip_rect,
                int x, int y, uint8_t* data, int data_len,
                int fgcolor, int bgcolor, const Rect & draw_rect)
    {
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

    /*****************************************************************************/
    void line(int rop, int x1, int y1, int x2, int y2, int bgcolor, const RDPPen & pen, const Rect & clip)
    {
        #warning if direction of line is inverted, put it back in the right order, for now just ignore lines in wrong direction
        if (x1 >= x2 && y1 >= y2
        && !clip.intersect(Rect(x1, y1, (x2 - x1) +1, (y2 - y1)+1)).isempty()){
            this->orders->line(1, x1, y1, x2, y2, rop, bgcolor, pen, clip);
            if (this->capture){
                this->capture->line(1, x1, y1, x2, y2, rop, bgcolor, pen, this->colors.bpp, clip);
            }
        }
    }

};

#endif
