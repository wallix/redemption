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
#include <deque>
#include "server_rdp.hpp"
#include "NewRDPOrders.hpp"
#include "orders.hpp"
#include "transport.hpp"
#include "config.hpp"

class Front {
public:
    struct BitmapCache *bmp_cache;
    struct Capture * capture;
    struct Font font;
    int mouse_x;
    int mouse_y;
    struct timeval start;
    bool nomouse;
    bool notimestamp;
    int timezone;
    struct server_rdp rdp_layer;
    struct RDP::Orders orders;
    Cache cache;

#warning mouse_x, mouse_y, start not initialized

    Front(SocketTransport * trans, Inifile * ini)
    :
    bmp_cache(0),
    capture(0),
    font(SHARE_PATH "/" DEFAULT_FONT_NAME),
    nomouse(ini->globals.nomouse),
    notimestamp(ini->globals.notimestamp),
    timezone(0),
    rdp_layer(trans, ini),
    cache() {}

    ~Front(){
        if (this->capture){
            delete this->capture;
        }
        if (this->bmp_cache){
            delete this->bmp_cache;
        }
    }

    void reset(){
        this->cache = cache;
        #warning is it necessary (or even usefull) to send remaining drawing orders before resetting ?
        if (this->orders.order_count > 0){
            this->force_send();
        }
        this->orders.reset_xx();

        if (this->bmp_cache){
            delete this->bmp_cache;
        }
        this->bmp_cache = new BitmapCache(&(this->rdp_layer.client_info));
        this->cache.reset(this->rdp_layer.client_info);

        LOG(LOG_INFO, "width=%d height=%d bpp=%d "
                  "cache1_entries=%d cache1_size=%d "
                  "cache2_entries=%d cache2_size=%d "
                  "cache2_entries=%d cache2_size=%d ",
        this->rdp_layer.client_info.width, this->rdp_layer.client_info.height, this->rdp_layer.client_info.bpp,
        this->rdp_layer.client_info.cache1_entries, this->rdp_layer.client_info.cache1_size,
        this->rdp_layer.client_info.cache2_entries, this->rdp_layer.client_info.cache2_size,
        this->rdp_layer.client_info.cache3_entries, this->rdp_layer.client_info.cache3_size);

    }

    void send()
    {
        if (this->orders.order_level > 0) {
            this->orders.order_level--;
            if (this->orders.order_level == 0){
                if (this->orders.order_count > 0){
                    this->force_send();
                }
            }
        }
    }

    /*****************************************************************************/
    // check if the next order will fit in available packet size
    // if not send previous orders we got and init a new packet
    void reserve_order(size_t asked_size)
    {
        if (this->orders.order_count > 0) {
            size_t max_packet_size = std::min(this->orders.out_stream.capacity, (size_t)16384);
            size_t used_size = (size_t)(this->orders.out_stream.p - this->orders.order_count_ptr);

            if ((used_size + asked_size + 100) > max_packet_size) {
                this->force_send();
            }
        }
        if (0 == this->orders.order_count){
            // we initialize only when **sure** there will be orders to send,
            // (ie: when reserve_order(xx) is called),
            // RDP does not support empty orders batch
            // at this point we know at least one order will be emited
            this->force_init();
        }
        this->orders.order_count++;
    }

    void force_init()
    {
//        LOG(LOG_INFO, "Orders::force_init()");
        #warning see with order limit : is this big enough ?
        this->orders.out_stream.init(16384);

        this->rdp_layer.sec_layer.server_sec_init(this->orders.out_stream);
        this->orders.out_stream.rdp_hdr = this->orders.out_stream.p;
        this->orders.out_stream.p += 18;

        this->orders.out_stream.out_uint16_le(RDP_UPDATE_ORDERS);
        this->orders.out_stream.out_clear_bytes(2); /* pad */
        this->orders.order_count_ptr = this->orders.out_stream.p;
        this->orders.out_stream.out_clear_bytes(2); /* number of orders, set later */
        this->orders.out_stream.out_clear_bytes(2); /* pad */
    }

    void force_send()
    {
//        LOG(LOG_ERR, "force_send: level=%d order_count=%d", this->orders.order_level, this->orders.order_count);
        this->orders.out_stream.mark_end();
        this->orders.out_stream.p = this->orders.order_count_ptr;
        this->orders.out_stream.out_uint16_le(this->orders.order_count);

        this->orders.out_stream.p = this->orders.out_stream.rdp_hdr;
        int len = this->orders.out_stream.end - this->orders.out_stream.p;
        this->orders.out_stream.out_uint16_le(len);
        this->orders.out_stream.out_uint16_le(0x10 | RDP_PDU_DATA);
        this->orders.out_stream.out_uint16_le(this->rdp_layer.mcs_channel);
        this->orders.out_stream.out_uint32_le(this->rdp_layer.share_id);
        this->orders.out_stream.out_uint8(0);
        this->orders.out_stream.out_uint8(1);
        this->orders.out_stream.out_uint16_le(len - 14);
        this->orders.out_stream.out_uint8(RDP_DATA_PDU_UPDATE);
        this->orders.out_stream.out_uint8(0);
        this->orders.out_stream.out_uint16_le(0);

        this->rdp_layer.sec_layer.server_sec_send(this->orders.out_stream, MCS_GLOBAL_CHANNEL);

        this->orders.order_count = 0;
    }


    void start_capture(int width, int height, bool flag, char * path, const char * codec_id, const char * quality, int timezone)
    {
        this->timezone = timezone;
        if (flag){
            this->stop_capture();
            gettimeofday(&this->start, NULL);
            this->capture = new Capture(width, height, this->rdp_layer.client_info.bpp, path, codec_id, quality);
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
        this->orders.init();
    }

    void end_update()
    {
        this->send();
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
        this->rdp_layer.server_rdp_send_pointer(cache_idx, data, mask, x, y);
//        LOG(LOG_INFO, "front::send_pointer done\n");
    }

    void set_pointer(int cache_idx) throw (Error)
    {
//        LOG(LOG_INFO, "front::set_pointer\n");
        this->rdp_layer.server_rdp_set_pointer(cache_idx);
//        LOG(LOG_INFO, "front::set_pointer done\n");
    }


    int get_channel_id(char* name)
    {
//        LOG(LOG_INFO, "front::get_channel_id\n");
        return this->rdp_layer.sec_layer.mcs_layer.server_mcs_get_channel_id(name);
    }


    /* fill in an area of the screen with one color */
    void opaque_rect(const RDPOpaqueRect & cmd, const Rect & clip)
    {
//      LOG(LOG_INFO, "this->front.orders->opaque_rect(Rect(%d, %d, %d, %d), 0x%.6x, Rect(%d, %d, %d, %d));", r.x, r.y, r.cx, r.cy, fgcolor, clip.x, clip.y, clip.cx, clip.cy);

        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->reserve_order(23);
            this->orders.opaque_rect(cmd, clip);
            if (this->capture){
                this->capture->opaque_rect(cmd, clip);
            }
        }
    }

    void scr_blt(const RDPScrBlt & scrblt, const Rect &clip)
    {
        if (!clip.isempty() && !clip.intersect(scrblt.rect).isempty()){
            // this one is used when dragging a visible window on screen
            this->reserve_order(25);
            this->orders.scr_blt(scrblt, clip);
            if (this->capture){
                this->capture->scr_blt(scrblt, clip);
            }
        }
    }

    void dest_blt(const RDPDestBlt & cmd, const Rect &clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->reserve_order(21);
            this->orders.dest_blt(cmd, clip);
            if (this->capture){
                #warning missing code in capture, apply some logical operator inplace
                this->capture->opaque_rect(RDPOpaqueRect(cmd.rect, WHITE), clip);
            }
        }
    }


    void pat_blt(const RDPPatBlt & cmd, const Rect &clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->reserve_order(29);
            this->orders.pat_blt(cmd, clip);
            if (this->capture){
                #warning missing code in capture, apply full pat_blt
                this->capture->opaque_rect(RDPOpaqueRect(cmd.rect, cmd.fore_color), clip);
            }
        }
    }


    void mem_blt(const RDPMemBlt & memblt, const Rect & clip)
    {
        this->reserve_order(30);

        if (!clip.isempty() && !clip.intersect(memblt.rect).isempty()){
            this->orders.mem_blt(memblt, clip);
            if (this->capture){
                this->capture->mem_blt(memblt, *this->bmp_cache, clip);
            }
        }
    }

    void line_to(const RDPLineTo& lineto, const Rect & clip)
    {
        const Rect rect(
            (lineto.startx <= lineto.endx)?lineto.startx:lineto.endx,
            (lineto.starty <= lineto.endy)?lineto.starty:lineto.endy,
            (lineto.startx <= lineto.endx)?(lineto.endx-lineto.startx+1):(lineto.startx-lineto.endx+1),
            (lineto.starty <= lineto.endy)?(lineto.endy-lineto.starty+1):(lineto.starty-lineto.endy+1));

        if (!clip.isempty() && !clip.intersect(rect).isempty()){
            this->reserve_order(32);
            this->orders.line_to(lineto, clip);
            if (this->capture){
                this->capture->line_to(lineto, clip);
            }
        }

    }

    void glyph_index(const RDPGlyphIndex & glyph_index, const Rect & clip)
    {
//        LOG(LOG_INFO, "front::glyph_index\n");
        if (glyph_index.bk.intersect(clip).isempty()){
            return;
        }

        this->reserve_order(297);
        this->orders.glyph_index(glyph_index, clip);
        if (this->capture){
            this->capture->glyph_index(glyph_index, clip);
        }
    }

    void color_cache(const RGBPalette & palette)
    {
//        LOG(LOG_INFO, "front::color_cache\n");
        if (this->rdp_layer.client_info.bpp <= 8) {
            this->rdp_layer.send_global_palette(palette);
            this->orders.init();
            this->reserve_order(2000);
            this->orders.color_cache(palette, 0);
            this->send();
        }
//        LOG(LOG_INFO, "front::color_cache done\n");
    }

    void brush_cache(const int index)
    {
        const int size = 8;
        this->reserve_order(size + 12);

        this->orders.brush_cache(8, 8, 1, 0x81, size, this->cache.brush_items[index].pattern, index);

    }


    void send_bitmap_common(const uint8_t cache_id, const uint16_t cache_idx)
    {
        BitmapCacheItem * entry =  this->bmp_cache->get_item(cache_id, cache_idx);

        #warning really when using compression we'll use less space
        this->reserve_order(entry->pbmp->bmp_size + 16);

        this->orders.send_bitmap_common(&this->rdp_layer.client_info, *entry->pbmp, cache_id, cache_idx);
    }

    // draw bitmap from src_data (image rect contained in src_r) to x, y
    // clip_region is the list of visible rectangles that should be sent
    void send_bitmap_front(const Rect & dst, const Rect & src_r, const uint8_t rop, const uint8_t * src_data,
                     int palette_id,
                     const Rect & clip)
    {
        LOG(LOG_INFO, "front::send_bitmap_front bpp=%d\n", this->rdp_layer.client_info.bpp);
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
                                                this->rdp_layer.client_info.bpp);

                    uint8_t send_type = (cache_ref >> 24);
                    uint8_t cache_id  = (cache_ref >> 16);
                    uint16_t cache_idx = (cache_ref & 0xFFFF);

                    if (send_type == BITMAP_ADDED_TO_CACHE){
                        this->send_bitmap_common(cache_id, cache_idx);
                    };

                    const RDPMemBlt cmd(cache_id + palette_id * 256, tile.offset(dst.x, dst.y), rop, 0, 0, cache_idx);
                    this->mem_blt(cmd, clip);
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

//    #warning harmonize names with orders send_glyph or send_font
//    void send_glyph(int font, int character,
//                    int offset, int baseline,
//                    int width, int height, const uint8_t* data)
//    {
////        LOG(LOG_INFO, "front::send_glyph\n");
//        struct FontChar fi(offset, baseline, width, height, 0);

//        memcpy(fi.data, data, fi.datasize());
//        this->send_glyph(&fi, font, character);
//    }

    void send_glyph(const FontChar & font_char, int font_index, int char_index)
    {
//        LOG(LOG_INFO, "front::send_glyph 2\n");

        this->reserve_order(font_char.datasize() + 18);
        this->orders.send_font(font_char, font_index, char_index);
    }

};

#endif
