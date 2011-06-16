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
#include <string.h>
#include <sys/time.h>
#include <deque>
#include <algorithm>

#include "stream.hpp"
#include "constants.hpp"
#include "ssl_calls.hpp"
#include "file_loc.hpp"
#include "altoco.hpp"
#include "rect.hpp"
#include "region.hpp"
#include "capture.hpp"
#include "nativecapture.hpp"
#include "font.hpp"
#include "bitmap.hpp"
#include "bitmap_cache.hpp"
#include "cache.hpp"

#include "server_rdp.hpp"

#include "RDP/orders/RDPOrdersCommon.hpp"

#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"

#include "transport.hpp"
#include "config.hpp"
#include "error.hpp"



class Front {
public:
    Stream out_stream;
    Cache cache;
    struct BitmapCache *bmp_cache;
    struct Capture * capture;
    struct NativeCapture * nativecapture;
    struct Font font;
    int mouse_x;
    int mouse_y;
    struct timeval start;
    bool nomouse;
    bool notimestamp;
    int timezone;
    struct server_rdp rdp_layer;
    // Internal state of orders
    RDPOrderCommon common;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPScrBlt scrblt;
    RDPOpaqueRect opaquerect;
    RDPMemBlt memblt;
    RDPLineTo lineto;
    RDPGlyphIndex glyphindex;
    // state variables for gathering batch of orders
    uint8_t* order_count_ptr;
    int order_count;
    int order_level;

    Front(SocketTransport * trans, Inifile * ini)
    :
    out_stream(16384),
    cache(),
    bmp_cache(0),
    capture(0),
    nativecapture(0),
    font(SHARE_PATH "/" DEFAULT_FONT_NAME),
    mouse_x(0),
    mouse_y(0),
    nomouse(ini->globals.nomouse),
    notimestamp(ini->globals.notimestamp),
    timezone(0),
    rdp_layer(trans, ini),
    // Internal state of orders
    common(0, Rect(0, 0, 1, 1)),
    destblt(Rect(), 0),
    patblt(Rect(), 0, 0, 0, RDPBrush()),
    scrblt(Rect(), 0, 0, 0),
    opaquerect(Rect(), 0),
    memblt(0, Rect(), 0, 0, 0, 0),
    lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
    glyphindex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),
    // state variables for a batch of orders
    order_count_ptr(0),
    order_count(0),
    order_level(0)
    {}

    ~Front(){
        if (this->capture){
            delete this->capture;
            delete this->nativecapture;
        }
        if (this->bmp_cache){
            delete this->bmp_cache;
        }
    }

    void reset(){
        this->cache = cache;
        #warning is it necessary (or even usefull) to send remaining drawing orders before resetting ?
        if (this->order_count > 0){
            this->force_send();
        }
        #warning some duplication of front instanciation code, it probably means that current front is part of rdp layer, not the other way around, as rdp_layer has a longer lifetime than front. On the other hand caches seems to be on the same timeline that front (means caches are parts of front) and should not be instnaciated elsewhere.
        this->common = RDPOrderCommon(0,  Rect(0, 0, 1, 1));
        this->memblt = RDPMemBlt(0, Rect(), 0, 0, 0, 0);
        this->opaquerect = RDPOpaqueRect(Rect(), 0);
        this->scrblt = RDPScrBlt(Rect(), 0, 0, 0);
        this->destblt = RDPDestBlt(Rect(), 0);
        this->patblt = RDPPatBlt(Rect(), 0, 0, 0, RDPBrush());
        this->lineto = RDPLineTo(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0));
        this->glyphindex = RDPGlyphIndex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)"");
        this->common.order = RDP::PATBLT;

        this->order_count = 0;
        this->order_level = 0;

        if (this->bmp_cache){
            delete this->bmp_cache;
        }
        this->bmp_cache = new BitmapCache(&(this->rdp_layer.client_info));
        this->cache.reset(this->rdp_layer.client_info);

        LOG(LOG_INFO, "// width=%d height=%d bpp=%d "
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
        if (this->order_level > 0) {
            this->order_level--;
            if (this->order_level == 0){
                if (this->order_count > 0){
                    this->force_send();
                }
            }
        }
    }

    /*****************************************************************************/
    // check if the next order will fit in available packet size
    // if not send previous orders we got and init a new packet
    #warning we should define some kind of OrdersStream, to buffer in orders
    void reserve_order(size_t asked_size)
    {
        if (this->order_count > 0) {
            size_t max_packet_size = std::min(this->out_stream.capacity, (size_t)16384);
            size_t used_size = (size_t)(this->out_stream.p - this->order_count_ptr);

            if ((used_size + asked_size + 100) > max_packet_size) {
                this->force_send();
            }
        }
        if (0 == this->order_count){
            // we initialize only when **sure** there will be orders to send,
            // (ie: when reserve_order(xx) is called),
            // RDP does not support empty orders batch
            // at this point we know at least one order will be emited
            this->force_init();
        }
        this->order_count++;
    }

    void force_init()
    {
//        LOG(LOG_INFO, "Orders::force_init()");
        #warning see with order limit : is this big enough ?
        this->out_stream.init(16384);

        this->rdp_layer.sec_layer.server_sec_init(this->out_stream);
        #warning we should define some kind of OrdersStream, to buffer in orders
        this->out_stream.rdp_hdr = this->out_stream.p;
        this->out_stream.p += 18;

        this->out_stream.out_uint16_le(RDP_UPDATE_ORDERS);
        this->out_stream.out_clear_bytes(2); /* pad */
        this->order_count_ptr = this->out_stream.p;
        this->out_stream.out_clear_bytes(2); /* number of orders, set later */
        this->out_stream.out_clear_bytes(2); /* pad */
    }

    void force_send()
    {
        #warning we should define some kind of OrdersStream, to buffer in orders
//        LOG(LOG_ERR, "force_send: level=%d order_count=%d", this->order_level, this->order_count);
        this->out_stream.mark_end();
        this->out_stream.p = this->order_count_ptr;
        this->out_stream.out_uint16_le(this->order_count);

        this->out_stream.p = this->out_stream.rdp_hdr;
        int len = this->out_stream.end - this->out_stream.p;
        this->out_stream.out_uint16_le(len);
        this->out_stream.out_uint16_le(0x10 | RDP_PDU_DATA);
        this->out_stream.out_uint16_le(this->rdp_layer.mcs_channel);
        this->out_stream.out_uint32_le(this->rdp_layer.share_id);
        this->out_stream.out_uint8(0);
        this->out_stream.out_uint8(1);
        this->out_stream.out_uint16_le(len - 14);
        this->out_stream.out_uint8(RDP_DATA_PDU_UPDATE);
        this->out_stream.out_uint8(0);
        this->out_stream.out_uint16_le(0);

        this->rdp_layer.sec_layer.server_sec_send(this->out_stream, MCS_GLOBAL_CHANNEL);

        this->order_count = 0;
    }


    void start_capture(int width, int height, bool flag, char * path, const char * codec_id, const char * quality, int timezone)
    {
        LOG(LOG_INFO, "------------ start capture -------------");
        this->timezone = timezone;
        if (flag){
            this->stop_capture();
            gettimeofday(&this->start, NULL);

            this->nativecapture = new NativeCapture(width, height, this->rdp_layer.client_info.bpp, path);
            this->capture = new Capture(width, height, this->rdp_layer.client_info.bpp, path, codec_id, quality);
        }
    }

    void stop_capture()
    {
        if (this->capture){
            delete this->capture;
            delete this->nativecapture;
            this->capture = 0;
            this->nativecapture = 0;
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
            #warning interframe interval management should be moved to capture (because it could be different between several capture objects)
            const uint64_t inter_frame_interval = this->capture->inter_frame_interval;
            struct timeval now;
            gettimeofday(&now, NULL);
            if (difftimeval(now, this->start) > inter_frame_interval)
            {
                LOG(LOG_INFO, "------------ periodic snapshot -------------");
                this->start = now;
                this->capture->snapshot(
                    this->mouse_x, this->mouse_y,
                    pointer_is_displayed|this->nomouse,
                    this->notimestamp, this->timezone);
                this->nativecapture->snapshot();
            }
        }
    }

    void begin_update()
    {
        this->order_level++;
        if (this->order_level == 1) {
            this->order_count = 0;
        }
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


    void send_global_palette(const RGBPalette & palette)
    {
        this->rdp_layer.send_global_palette(palette);
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

            RDPOrderCommon newcommon(RDP::RECT, clip);
            cmd.emit(this->out_stream, newcommon, this->common, this->opaquerect);
            this->common = newcommon;
            this->opaquerect = cmd;

            if (this->capture){
                this->capture->opaque_rect(cmd, clip);
                this->nativecapture->opaque_rect(cmd, clip);
            }
        }
    }

    void scr_blt(const RDPScrBlt & cmd, const Rect &clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            // this one is used when dragging a visible window on screen
            this->reserve_order(25);
            RDPOrderCommon newcommon(RDP::SCREENBLT, clip);
            cmd.emit(this->out_stream, newcommon, this->common, this->scrblt);
            this->common = newcommon;
            this->scrblt = cmd;

            if (this->capture){
                this->capture->scr_blt(cmd, clip);
                this->nativecapture->scr_blt(cmd, clip);
            }
        }
    }

    void dest_blt(const RDPDestBlt & cmd, const Rect &clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->reserve_order(21);

            RDPOrderCommon newcommon(RDP::DESTBLT, clip);
            cmd.emit(this->out_stream, newcommon, this->common, this->destblt);
            this->common = newcommon;
            this->destblt = cmd;

            if (this->capture){
                #warning missing code in capture, apply some logical operator inplace
                this->capture->opaque_rect(RDPOpaqueRect(cmd.rect, WHITE), clip);
                this->nativecapture->dest_blt(cmd, clip);
            }
        }
    }


    void pat_blt(const RDPPatBlt & cmd, const Rect &clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->reserve_order(29);

            using namespace RDP;
            RDPOrderCommon newcommon(PATBLT, clip);
            cmd.emit(this->out_stream, newcommon, this->common, this->patblt);
            this->common = newcommon;
            this->patblt = cmd;

            if (this->capture){
                #warning missing code in capture, apply full pat_blt
                this->capture->opaque_rect(RDPOpaqueRect(cmd.rect, cmd.fore_color), clip);
                this->nativecapture->pat_blt(cmd, clip);
            }
        }
    }


    void mem_blt(const RDPMemBlt & cmd, const Rect & clip)
    {
        using namespace RDP;
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->reserve_order(30);
            RDPOrderCommon newcommon(MEMBLT, clip);
            cmd.emit(this->out_stream, newcommon, this->common, this->memblt);
            this->common = newcommon;
            this->memblt = cmd;

            if (this->capture){
                this->capture->mem_blt(cmd, *this->bmp_cache, clip);
                this->nativecapture->mem_blt(cmd, *this->bmp_cache, clip);
            }
        }
    }

    void line_to(const RDPLineTo& cmd, const Rect & clip)
    {
        using namespace RDP;

        const uint16_t minx = std::min(cmd.startx, cmd.endx);
        const uint16_t miny = std::min(cmd.starty, cmd.endy);
        const Rect rect(minx, miny,
                        std::max(cmd.startx, cmd.endx)-minx+1,
                        std::max(cmd.starty, cmd.endy)-miny+1);

        if (!clip.isempty() && !clip.intersect(rect).isempty()){
            this->reserve_order(32);
            RDPOrderCommon newcommon(LINE, clip);
            cmd.emit(this->out_stream, newcommon, this->common, this->lineto);
            this->common = newcommon;
            this->lineto = cmd;
            if (this->capture){
                this->capture->line_to(cmd, clip);
                this->nativecapture->line_to(cmd, clip);
            }
        }
    }

    void glyph_index(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        if (cmd.bk.intersect(clip).isempty()){
            return;
        }

        this->reserve_order(297);
        RDPOrderCommon newcommon(RDP::GLYPHINDEX, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->glyphindex);
        this->common = newcommon;
        this->glyphindex = cmd;
        if (this->capture){
            this->capture->glyph_index(cmd, clip);
            this->nativecapture->glyph_index(cmd, clip);
        }
    }

    void color_cache(const RGBPalette & palette)
    {
        this->reserve_order(2000);
        RDPColCache newcmd;
        #warning why is it always palette 0 ?
        memcpy(newcmd.palette[0], palette, 256);
        newcmd.emit(this->out_stream, 0);
    }

    void brush_cache(const int index)
    {
        const int size = 8;
        this->reserve_order(size + 12);

        using namespace RDP;

        RDPBrushCache newcmd;
        #warning define a constructor with emit parameters
        newcmd.bpp = 1;
        newcmd.width = 8;
        newcmd.height = 8;
        newcmd.type = 0x81;
        newcmd.size = size;
        newcmd.data = this->cache.brush_items[index].pattern;
        newcmd.emit(this->out_stream, index);
        newcmd.data = 0;
    }


    void bitmap_cache(const uint8_t cache_id, const uint16_t cache_idx)
    {
        BitmapCacheItem * entry =  this->bmp_cache->get_item(cache_id, cache_idx);

        #warning really when using compression we'll use less space
        this->reserve_order(entry->pbmp->bmp_size(this->rdp_layer.client_info.bpp) + 16);

        using namespace RDP;

        RDPBmpCache bmp_order(this->rdp_layer.client_info.bpp, entry->pbmp, cache_id, cache_idx, &this->rdp_layer.client_info);
        bmp_order.emit(this->out_stream);

        if (this->capture){
            this->nativecapture->bitmap_cache(cache_id, cache_idx, entry);
        }

    }

    // draw bitmap from src_data (image rect contained in src_r) to x, y
    // clip_region is the list of visible rectangles that should be sent
    void send_bitmap_front(const Rect & dst, const Rect & src_r, const uint8_t rop, const uint8_t * src_data,
                     int palette_id,
                     const Rect & clip)
    {
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
                        this->bitmap_cache(cache_id, cache_idx);
                    };

                    const RDPMemBlt cmd(cache_id + palette_id * 256, tile.offset(dst.x, dst.y), rop, 0, 0, cache_idx);
                    this->mem_blt(cmd, clip);
                }
            }
        }
    }

    #warning create add_bitmap in front API (instead of send_bitmap_common ?)

    void send_bitmap_front2(const Rect & dst, const Rect & src_r, const uint8_t rop, const uint8_t * src_data,
                     int palette_id,
                     const Rect & clip)
    {
        this->send_bitmap_front(dst, src_r, rop, src_data, palette_id, clip);
    }

    void glyph_cache(const FontChar & font_char, int font_index, int char_index)
    {
        this->reserve_order(font_char.datasize() + 18);

        using namespace RDP;

        #warning define a constructor with emit parameters

        RDPGlyphCache newcmd;
        newcmd.size = font_char.datasize();
        newcmd.cacheId = font_index;
        newcmd.glyphData_cacheIndex = char_index;
        newcmd.glyphData_x = font_char.offset;
        newcmd.glyphData_y = font_char.baseline;
        newcmd.glyphData_cx = font_char.width;
        newcmd.glyphData_cy = font_char.height;
        newcmd.glyphData_aj = font_char.data;
        newcmd.emit(this->out_stream);
        newcmd.glyphData_aj = 0;
    }

};

#endif
