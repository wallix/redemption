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

*/

#if !defined(__ORDERS_HPP__)
#define __ORDERS_HPP__

#include "server_rdp.hpp"
#include "font.hpp"
#include "constants.hpp"
#include "error.hpp"
#include "rect.hpp"
#include "NewRDPOrders.hpp"
#include <algorithm>
#include "altoco.hpp"
#include "bitmap.hpp"

namespace RDP {

struct Orders
{
    // State
    RDPOrderCommon common;
    RDPMemBlt memblt;
    RDPOpaqueRect opaquerect;
    RDPScrBlt scrblt;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPLineTo lineto;
    RDPGlyphIndex text;

    Stream* out_s;
    struct server_rdp* rdp_layer;

    uint8_t* order_count_ptr; /* pointer to count, set when sending */
    int order_count;
    int order_level; /* inc for every call to init */

    Orders(struct server_rdp* rdp_layer) :
        common(0, Rect(0, 0, 1, 1)),
        memblt(0, Rect(), 0, 0, 0, 0),
        opaquerect(Rect(), 0),
        scrblt(Rect(), 0, 0, 0),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        text(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)"")
    {
        this->order_count = 0;
        this->order_level = 0; /* inc for every call to init */
        this->rdp_layer = rdp_layer;
        this->out_s = new Stream(16384); // allocate 64k buffer
    }

    ~Orders()
    {
        delete this->out_s;
    }

    /*****************************************************************************/
    /* set all values to zero */
    void reset() throw (Error)
    {
//        LOG(LOG_INFO, "Orders::reset()");
#warning is it necessary (or even usefull) to send remaining drawing orders before resetting ?
        if (this->order_count > 0){
            this->force_send();
        }

        common = RDPOrderCommon(0,  Rect(0, 0, 1, 1));
        memblt = RDPMemBlt(0, Rect(), 0, 0, 0, 0);
        opaquerect = RDPOpaqueRect(Rect(), 0);
        scrblt = RDPScrBlt(Rect(), 0, 0, 0);
        destblt = RDPDestBlt(Rect(), 0);
        patblt = RDPPatBlt(Rect(), 0, 0, 0, RDPBrush());
        lineto = RDPLineTo(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0));
        text = RDPGlyphIndex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)"");
        common.order = PATBLT;

        this->order_count = 0;
        this->order_level = 0;
    }

    /*****************************************************************************/
    /* returns error */
    int init()
    {
//        LOG(LOG_INFO, "Orders::init() level=%d count=%d", this->order_level, this->order_count);
        this->order_level++;
        if (this->order_level == 1) {
            this->order_count = 0;
        }
        return 0;
    }

    /*****************************************************************************/
    /* returns error */
    void send()
    {
//        LOG(LOG_ERR, "Orders::send() level=%d order_count=%d", this->order_level, this->order_count);
        if (this->order_level > 0) {
            this->order_level--; // every init should have a send
            if (this->order_level == 0){
                if (this->order_count > 0){
                    this->force_send();
                }
            }
        }
    }

    void force_init()
    {
//        LOG(LOG_INFO, "Orders::force_init()");
        #warning see with order limit is this big enough ?
        this->out_s->init(16384);
        this->rdp_layer->server_rdp_init_data(*this->out_s);
        this->out_s->out_uint16_le(RDP_UPDATE_ORDERS);
        this->out_s->out_clear_bytes(2); /* pad */
        this->order_count_ptr = this->out_s->p;
        this->out_s->out_clear_bytes(2); /* number of orders, set later */
        this->out_s->out_clear_bytes(2); /* pad */
    }

    void force_send()
    {
//        LOG(LOG_ERR, "send: level=%d order_count=%d", this->order_level, this->order_count);
        this->out_s->mark_end();
        this->out_s->p = this->order_count_ptr;
        this->out_s->out_uint16_le(this->order_count);
        this->rdp_layer->server_rdp_send_data(*this->out_s, RDP_DATA_PDU_UPDATE);
        this->order_count = 0;
    }

    /*****************************************************************************/
    /* returns error */
    /* send a brush cache entry */
    int send_brush(int width, int height, int bpp, int type, int size, uint8_t* data, int cache_id)
    {
//        LOG(LOG_INFO, "order send brush(width=%d, height=%d, bpp=%d, type=%d, size=%d, cache_id=%d\n",
//            width, height, bpp, type, size, cache_id);
        using namespace RDP;

        this->reserve_order(size + 12);

        int order_flags = STANDARD | SECONDARY;
        this->out_s->out_uint8(order_flags);
        int len = (size + 6) - 7; /* length after type minus 7 */
        this->out_s->out_uint16_le(len);
        this->out_s->out_uint16_le(0); /* flags */
        this->out_s->out_uint8(BRUSHCACHE); /* type */
        this->out_s->out_uint8(cache_id);
        this->out_s->out_uint8(bpp);
        this->out_s->out_uint8(width);
        this->out_s->out_uint8(height);
        this->out_s->out_uint8(type);
        this->out_s->out_uint8(size);
        this->out_s->out_copy_bytes(data, size);

//        LOG(LOG_INFO, "order send brush done\n");

        return 0;
    }

    inline static bool is_1_byte(int16_t value){
        return (value >= -128) && (value <= 127);
    }

    inline static uint8_t pounder_bound(int16_t delta, uint8_t pound)
    {
        return ((pound * (delta != 0)) << (4 * is_1_byte(delta)));
    }

    int opaque_rect(const Rect & r, int color, const Rect & clip)
    {
        this->reserve_order(23);

//        char buffer[1000];
//        this->opaquerect.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "RECT OLD SEND: %s", buffer);

        RDPOpaqueRect cmd(r, color);
        RDPOrderCommon newcommon(RECT, clip);
        cmd.emit(*this->out_s, newcommon, this->common, this->opaquerect);
        this->common = newcommon;
        this->opaquerect = cmd;

//        this->opaquerect.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "RECT NEW SEND: %s", buffer);

        return 0;
    }

    int screen_blt(const Rect & r, int16_t srcx, int16_t srcy, uint8_t rop, const Rect &clip)
    {
        this->reserve_order(25);

//        char buffer[1000];
//        this->scrblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "SCREENBLT OLD SEND: %s", buffer);


        RDPScrBlt cmd(r, rop, srcx, srcy);
        RDPOrderCommon newcommon(SCREENBLT, clip);
        cmd.emit(*this->out_s, newcommon, this->common, this->scrblt);
        this->common = newcommon;
        this->scrblt = cmd;

//        this->scrblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "SCREENBLT NEW SEND: %s", buffer);

        return 0;
    }

    int dest_blt(const Rect & r, uint8_t rop, const Rect &clip)
    {
        this->reserve_order(21);

//        char buffer[1000];
//        this->destblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "DESTBLT OLD SEND: %s", buffer);


        RDPDestBlt cmd(r, rop);
        RDPOrderCommon newcommon(DESTBLT, clip);
        cmd.emit(*this->out_s, newcommon, this->common, this->destblt);
        this->common = newcommon;
        this->destblt = cmd;

//        this->destblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "DESTBLT NEW SEND: %s", buffer);

        return 0;
    }

    int pat_blt(const Rect & r, int rop, uint32_t bg_color, uint32_t fg_color, const RDPBrush & brush, const Rect &clip)
    {
        this->reserve_order(29);

//        char buffer[1000];
//        this->patblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "PATBLT OLD SEND: %s", buffer);

        RDPPatBlt cmd(r, (uint8_t)rop, bg_color, fg_color, brush);
        RDPOrderCommon newcommon(PATBLT, clip);
        cmd.emit(*this->out_s, newcommon, this->common, this->patblt);
        this->common = newcommon;
        this->patblt = cmd;

//        this->patblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "PATBLT NEW SEND: %s", buffer);

        return 0;
    }

    void mem_blt(int cache_id,
                int color_table, const Rect & r,
                int rop, int bmp_cx, int bmp_cy, int srcx, int srcy,
                int cache_idx, const Rect & clip)
    {
        printf("mem_blt(color_table=%d, r(%d, %d, %d, %d),"
               " rop=%.2x, bmp_cx=%d, bmp_cy=%d, srcx=%d, srcy=%d,"
               " clip(%d, %d, %d, %d) cache %d:%d\n",
               color_table, r.x, r.y, r.cx, r.cy, rop,
               bmp_cx, bmp_cy, srcx, srcy,
               clip.x, clip.y, clip.cx, clip.cy,
               cache_id, cache_idx);

        this->reserve_order(30);

//        char buffer[1000];
//        this->memblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "OLD SEND: %s", buffer);

        RDPMemBlt cmd(cache_id + color_table * 256, r, rop, srcx, srcy, cache_idx);
        RDPOrderCommon newcommon(MEMBLT, clip);
        cmd.emit(*this->out_s, newcommon, this->common, this->memblt);
        this->common = newcommon;
        this->memblt = cmd;

//        this->memblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "NEW SEND: %s", buffer);
//        LOG(LOG_INFO, "%s", buffer);
    }

    /*****************************************************************************/
    /* returns error */
    /* send a line order */
    /* max size 32 */
    void line(int back_mode, int startx, int starty,
             int endx, int endy, int rop2, int back_color,
             const RDPPen & pen,
             const Rect & clip)

    {
//        LOG(LOG_INFO, "Orders::line(%d, %d, %d, %d, %d, %d, %d, clip(%d, %d, %d, %d))\n",
//            back_mode, startx, starty, endx, endy, rop2, back_color, clip.x, clip.y, clip.cx, clip.cy);

        #warning this should move out of Order
        if (clip.intersect(Rect(startx, starty, (endx - startx) +1, (endy - starty)+1)).isempty()){
            return;
        }

        #warning parameters should not be changed inside Order
        if ((back_mode < 1) || (back_mode > 2)) { /* TRANSPARENT(1) or OPAQUE(2) */
            back_mode = 1;
        }
        if ((rop2 < 1) || (rop2 > 0x10)) {
            rop2 = 0x0d; /* R2_COPYPEN */
        }

        this->reserve_order(32);

//        char buffer[1000];
//        this->destblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "DESTBLT OLD SEND: %s", buffer);

        RDPLineTo cmd(back_mode, startx, starty, endx, endy, back_color, rop2, pen);
        RDPOrderCommon newcommon(LINE, clip);
        cmd.emit(*this->out_s, newcommon, this->common, this->lineto);
        this->common = newcommon;
        this->lineto = cmd;

//        this->lineto.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "LINETO NEW SEND: %s", buffer);
    }



    /*****************************************************************************/
    /* returns error */
    void glyph_index(int font, int flags, int mixmode, int fg_color, int bg_color,
             const Rect & text_clip, const Rect & box, int x, int y, uint8_t* data,
             int data_len, const Rect & clip)
    {

//        LOG(LOG_DEBUG, "text(font=%d, flags = %x mixmode=%d, fg_color=%x, bg_color=%x,  text_clip(%d, %d, %d, %d),  box(%d, %d, %d, %d), x=%d, y=%d, datalen=%d,  clip(%d, %d, %d, %d))\n", font, mixmode,flags, fg_color, bg_color, text_clip.x, text_clip.y, text_clip.cx, text_clip.cy, box.x, box.y, box.cx, box.cy, x, y,  data_len, clip.x, clip.y, clip.cx, clip.cy);

        #warning this should move out of Order
        if (clip.intersect(text_clip).isempty()){
            return;
        }

        this->reserve_order(297);

//        char buffer[1000];
//        this->text.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "GLYPHINDEX OLD SEND: %s", buffer);

        RDPOrderCommon newcommon(GLYPHINDEX, clip);
        RDPGlyphIndex cmd(font, flags, 0, mixmode,
                         fg_color,
                         bg_color,
                         text_clip,
                         box,
                         RDPBrush(),
                         x, y,
                         data_len, data);

        cmd.emit(*this->out_s, newcommon, this->common, this->text);
        this->common = newcommon;
        this->text = cmd;

//        this->text.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "GLYPHINDEX NEW SEND: %s", buffer);
    }

    /*****************************************************************************/
    // [MS-RDPGDI] 2.2.2.2.1.2.4 Cache Color Table (CACHE_COLOR_TABLE_ORDER)

    // The Cache Color Table Secondary Drawing Order is used by the server
    // to instruct the client to store a color table in a particular Color Table
    // Cache entry. Color tables are used in the MemBlt (section 2.2.2.2.1.1.2.9)
    // and Mem3Blt (section 2.2.2.2.1.1.2.10) Primary Drawing Orders.

    // Support for color table caching is not negotiated in the Color Table Cache
    // Capability Set (section 2.2.1.1), but is instead implied by support for
    // the MemBlt (section 2.2.2.2.1.1.2.9) and Mem3Blt (section 2.2.2.2.1.1.2.10)
    // Primary Drawing Orders. If support for these orders is advertised in the
    // Order Capability Set (see [MS-RDPBCGR] section 2.2.7.1.3), the existence
    // of a color table cache with entries for six palettes is assumed when
    // palettized color is being used, and the Cache Color Table is used to
    // update these palettes.

    // header (6 bytes): A Secondary Order Header, as defined in section
    // 2.2.2.2.1.2.1.1. The embedded orderType field MUST be set to
    // TS_CACHE_COLOR_TABLE (0x01).

    // cacheIndex (1 byte): An 8-bit, unsigned integer. An entry in the Cache
    // Color Table where the color table MUST be stored. This value MUST be in
    // the range 0 to 5 (inclusive).

    // numberColors (2 bytes): A 16-bit, unsigned integer. The number of Color Quad
    // (section 2.2.2.2.1.2.4.1) structures in the colorTable field. This field
    // MUST be set to 256 entries.

    // colorTable (variable): A Color Table composed of an array of Color Quad
    // (section 2.2.2.2.1.2.4.1) structures. The number of entries in the array
    // is given by the numberColors field.

    // 2.2.2.2.1.2.4.1 Color Quad (TS_COLOR_QUAD)
    // The TS_COLOR_QUAD structure is used to express the red, green, and blue
    // components necessary to reproduce a color in the additive RGB space.

    // blue (1 byte): An 8-bit, unsigned integer. The blue RGB color component.

    // green (1 byte): An 8-bit, unsigned integer. The green RGB color component.

    // red (1 byte): An 8-bit, unsigned integer. The red RGB color component.

    // pad1Octet (1 byte): An 8-bit, unsigned integer. Padding. Values in this
    // field are arbitrary and MUST be ignored.

    int send_palette(const uint32_t (& palette)[256], int cache_id)
    {
//        LOG(LOG_INFO, "Orders::send_palette(%d)", cache_id);
        this->reserve_order(2000);

        RDPColCache newcmd(8);
        memcpy(newcmd.palette[0], palette, 256);
        newcmd.emit(*this->out_s, 0);
        return 0;
    }

    /* max size width * height * Bpp + 16 */
    void send_raw_bitmap(int version, int width, int height, int bpp, const uint8_t* data, int cache_id, int cache_idx)
    {
       printf("orders::send_raw_bitmap width=%d height=%d bpp=%d data=%p %d:%d\n", width, height, bpp, data, cache_id, cache_idx);
        #warning RDPBmpCache can probably be merged with bitmap object
        RDPBmpCache bmp(version, width, height, bpp, data, cache_id, cache_idx);

        int bufsize = align4(width * nbbytes(bpp)) * height;
        // check reserved size depending on version
        this->reserve_order(bufsize + 16);
        bmp.emit(*this->out_s);
        bmp.data = 0;
    }

    /*****************************************************************************/
    /* returns error */
    /* max size width * height * Bpp + 16 */
    void send_bitmap(int width, int height, int bpp, const uint8_t* data, size_t bufsize, int cache_id, int cache_idx)
    {
       printf("orders::send_bitmap width=%d height=%d bpp=%d data=%p bufsize=%d %d:%d\n", width, height, bpp, data, bufsize, cache_id, cache_idx);

        int Bpp = nbbytes(bpp);
        this->reserve_order(bufsize + 16);
        int order_flags = STANDARD | SECONDARY;
        this->out_s->out_uint8(order_flags);
        this->out_s->out_uint16_le(bufsize + 10);
        this->out_s->out_uint16_le(8); /* flags */
        this->out_s->out_uint8(TS_CACHE_BITMAP_COMPRESSED); /* type */
        this->out_s->out_uint8(cache_id);
        this->out_s->out_clear_bytes(1); /* pad */
        this->out_s->out_uint8(width);
        this->out_s->out_uint8(height);
        this->out_s->out_uint8(bpp);
        this->out_s->out_uint16_le(bufsize/* + 8*/);
        this->out_s->out_uint16_le(cache_idx);



        this->out_s->out_clear_bytes(2); /* pad */

        this->out_s->out_uint16_le(bufsize);

        this->out_s->out_uint16_le(width * Bpp); /* line size */

        this->out_s->out_uint16_le(width * Bpp * height); /* final size */
        this->out_s->out_copy_bytes(data, bufsize);
    }


    void send_bitmap_small_headers(int width, int height, int bpp, const uint8_t* data, size_t bufsize, int cache_id, int cache_idx)
    {
       printf("orders::send_bitmap_small_headers width=%d height=%d bpp=%d data=%p bufsize=%d %d:%d\n", width, height, bpp, data, bufsize, cache_id, cache_idx);
        this->reserve_order(bufsize + 16);
        int order_flags = STANDARD | SECONDARY;
        this->out_s->out_uint8(order_flags);
        /* length after type minus 7 */
        this->out_s->out_uint16_le(bufsize + 2);
        this->out_s->out_uint16_le(1024); /* flags */
        this->out_s->out_uint8(TS_CACHE_BITMAP_COMPRESSED); /* type */

        this->out_s->out_uint8(cache_id);
        this->out_s->out_clear_bytes(1); /* pad */

        this->out_s->out_uint8(width);
        this->out_s->out_uint8(height);
        this->out_s->out_uint8(bpp);
        this->out_s->out_uint16_le(bufsize/* + 8*/);
        this->out_s->out_uint16_le(cache_idx);
        this->out_s->out_copy_bytes(data, bufsize);
    }


    /*****************************************************************************/
    /* returns error */
    /* max size width * height * Bpp + 14 */
    void send_bitmap2(BitmapCache & bmp_cache, uint8_t* data, size_t bufsize, int cache_id, int cache_idx)
    {
       BitmapCacheItem * entry =  bmp_cache.get_item(cache_id, cache_idx);
       int width = align4(entry->bmp.cx);
       int height = entry->bmp.cy;
       int bpp = entry->bmp.bpp;

       printf("orders::send_bitmap2 width=%d height=%d bpp=%d data=%p bufsize=%d %d:%d\n", width, height, bpp, data, bufsize, cache_id, cache_idx);

        int Bpp = nbbytes(bpp);
        this->reserve_order(bufsize + 14);
        this->out_s->out_uint8(STANDARD | SECONDARY);

        this->out_s->out_uint16_le(bufsize - 1); /* length after type minus 7 */
        this->out_s->out_uint16_le(0x400 | (((Bpp + 2) << 3) & 0x38) | (cache_id & 7)); /* flags */
        this->out_s->out_uint8(TS_CACHE_BITMAP_COMPRESSED_REV2); /* type */
        this->out_s->out_uint8(width);
        this->out_s->out_uint8(height);
        this->out_s->out_uint16_be(bufsize | 0x4000);
        this->out_s->out_uint8(((cache_idx >> 8) & 0xff) | 0x80);
        this->out_s->out_uint8(cache_idx);
        this->out_s->out_copy_bytes(data, bufsize);
    }


    #warning compression_mode should not be in bitmap_cache it is just about choosing right compression headers.
    void send_bitmap_common(BitmapCache & bmp_cache, uint8_t cache_id, uint16_t cache_idx)
    {
        using namespace RDP;
        BitmapCacheItem * entry =  bmp_cache.get_item(cache_id, cache_idx);

        switch (bmp_cache.compression_mode()){
        case BitmapCache::NOT_COMPRESSED:
        {
            RDPBmpCache bmp(TS_CACHE_BITMAP_UNCOMPRESSED,
                            entry->bmp.cx, entry->bmp.cy, entry->bmp.bpp,
                            entry->bmp.data_co, cache_id, cache_idx);
            // check reserved size depending on version
            this->reserve_order(align4(entry->bmp.cx * nbbytes(entry->bmp.bpp)) * entry->bmp.cy + 16);
            bmp.emit(*this->out_s);
            bmp.data = 0;
        }
        break;
        case BitmapCache::NEW_NOT_COMPRESSED:
        {
            RDPBmpCache bmp(TS_CACHE_BITMAP_UNCOMPRESSED_REV2,
                            entry->bmp.cx, entry->bmp.cy, entry->bmp.bpp,
                            entry->bmp.data_co, cache_id, cache_idx);
            // check reserved size depending on version
            this->reserve_order(align4(entry->bmp.cx * nbbytes(entry->bmp.bpp)) * entry->bmp.cy + 16);
            bmp.emit(*this->out_s);
            bmp.data = 0;
        }
        break;
        case BitmapCache::COMPRESSED:
        {
            Stream stream(16384);
            #warning this does a copy of the bitmap buffer, we should be able to avoid it
            #warning compressed bitmap should be kept in cache... it is more efficient as we do not have to compute it again and again
            entry->bmp.compress(stream);
            size_t bufsize = stream.p - stream.data;

            printf("------- Compressed V4 REV1---------\n");
            for (int i = 0; i < bufsize; i++){
                if (0==(i % 16)){
                    printf("\n");
                }
                printf("0x%.2x, ", stream.data[i]);
            }
            printf("\n");
            printf("\n----------------------------\n");
            printf("\n");


            this->send_bitmap(align4(entry->bmp.cx), entry->bmp.cy, entry->bmp.bpp, stream.data, bufsize, cache_id, cache_idx);
        }
        break;
        case BitmapCache::COMPRESSED_SMALL_HEADERS:
        {
            Stream stream(16384);
            entry->bmp.compress(stream);
            size_t bufsize = stream.p - stream.data;

            printf("------- Compressed V4 SH---------\n");
            for (int i = 0; i < bufsize; i++){
                if (0==(i % 16)){
                    printf("\n");
                }
                printf("0x%.2x, ", stream.data[i]);
            }
            printf("\n");
            printf("\n----------------------------\n");
            printf("\n");

            this->send_bitmap_small_headers(
                align4(entry->bmp.cx), entry->bmp.cy, entry->bmp.bpp,
                stream.data, bufsize, cache_id, cache_idx);
        }
        break;
        case BitmapCache::NEW_COMPRESSED:
        {
            Stream stream(16384);
            #warning this does a copy of the bitmap buffer, we should be able to avoid it
            #warning compressed bitmap should be kept in cache... it is more efficient as we do not have to compute it again and again
            entry->bmp.compress(stream);
            size_t bufsize = stream.p - stream.data;

            printf("------- Compressed V4 REV2---------\n");
            for (int i = 0; i < bufsize; i++){
                if (0==(i % 16)){
                    printf("\n");
                }
                printf("0x%.2x, ", stream.data[i]);
            }
            printf("\n");
            printf("\n----------------------------\n");
            printf("\n");


            this->send_bitmap2(bmp_cache, stream.data, bufsize, cache_id, cache_idx);
        }
        break;
        }
    }


    /*****************************************************************************/
    /* returns error */
    /* max size datasize + 18*/
    /* todo, only sends one for now */
    void send_font(struct FontChar* font_char, int font_index, int char_index)
    {
//        LOG(LOG_DEBUG, "orders_send_font(font_char.width=%d, font_index=%d, char_index=%d)\n"
//          ,font_char->width, font_index, char_index);
        int datasize = (font_char->height * nbbytes(font_char->width) + 3) & ~3;
        this->reserve_order(datasize + 18);
        int order_flags = STANDARD | SECONDARY;
        this->out_s->out_uint8(order_flags);
        int len = (datasize + 12) - 7; /* length after type minus 7 */
        this->out_s->out_uint16_le(len);
        this->out_s->out_uint16_le(8); /* flags */
        this->out_s->out_uint8(FONTCACHE); /* type */
        this->out_s->out_uint8(font_index);
        this->out_s->out_uint8(1); /* num of chars */
        this->out_s->out_uint16_le(char_index);
        this->out_s->out_uint16_le(font_char->offset);
        this->out_s->out_uint16_le(font_char->baseline);
        this->out_s->out_uint16_le(font_char->width);
        this->out_s->out_uint16_le(font_char->height);
        this->out_s->out_copy_bytes(font_char->data, datasize);
    }


    /*****************************************************************************/
    private:

    // check if the next order will fit in available packet size
    // if not send previous orders we got and init a new packet
    void reserve_order(size_t asked_size)
    {
        if (this->order_count > 0) {
            size_t max_packet_size = std::min(this->out_s->capacity, (size_t)16384);
            size_t used_size = (size_t)(this->out_s->p - this->order_count_ptr);

//            LOG(LOG_INFO, "reserve_order(%d) [%d(%d)/%d(%d)]\n",
//                asked_size, used_size, this->order_count,
//                    max_packet_size, this->out_s->capacity);

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

};
} /* namespaces */

#endif
