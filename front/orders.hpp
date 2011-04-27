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

enum compression_type_t {
    NOT_COMPRESSED,
    COMPRESSED,
    COMPRESSED_SMALL_HEADERS,
    NEW_NOT_COMPRESSED,
    NEW_COMPRESSED
};

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

    Stream out_stream;
    struct server_rdp* rdp_layer;

    uint8_t* order_count_ptr;
    int order_count;
    int order_level;

    Orders(struct server_rdp* rdp_layer) :
        common(0, Rect(0, 0, 1, 1)),
        memblt(0, Rect(), 0, 0, 0, 0),
        opaquerect(Rect(), 0),
        scrblt(Rect(), 0, 0, 0),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        text(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),
        out_stream(16384)
    {
        this->order_count = 0;
        this->order_level = 0;
        this->rdp_layer = rdp_layer;
    }

    ~Orders()
    {
    }

    void reset() throw (Error)
    {
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

    int init()
    {
        this->order_level++;
        if (this->order_level == 1) {
            this->order_count = 0;
        }
        return 0;
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
        LOG(LOG_INFO, "Orders::force_init()");
        #warning see with order limit is this big enough ?
        this->out_stream.init(16384);
        this->rdp_layer->server_rdp_init_data(this->out_stream);
        this->out_stream.out_uint16_le(RDP_UPDATE_ORDERS);
        this->out_stream.out_clear_bytes(2); /* pad */
        this->order_count_ptr = this->out_stream.p;
        this->out_stream.out_clear_bytes(2); /* number of orders, set later */
        this->out_stream.out_clear_bytes(2); /* pad */
    }

    void force_send()
    {
        LOG(LOG_ERR, "force_send: level=%d order_count=%d", this->order_level, this->order_count);
        this->out_stream.mark_end();
        this->out_stream.p = this->order_count_ptr;
        this->out_stream.out_uint16_le(this->order_count);
        this->rdp_layer->server_rdp_send_data(this->out_stream, RDP_DATA_PDU_UPDATE);
        this->order_count = 0;
    }

    void opaque_rect(const Rect & r, int color, const Rect & clip)
    {
        this->reserve_order(23);

        LOG(LOG_INFO, "opaque_rect[%d](r(%d, %d, %d, %d) color=%x clip(%d, %d, %d, %d)",
            this->order_count, r.x, r.y, r.cx, r.cy, color, clip.x, clip.y, clip.cx, clip.cy);

        RDPOpaqueRect cmd(r, color);
        RDPOrderCommon newcommon(RECT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->opaquerect);
        this->common = newcommon;
        this->opaquerect = cmd;
    }

    void screen_blt(const Rect & r, int16_t srcx, int16_t srcy, uint8_t rop, const Rect &clip)
    {
        this->reserve_order(25);

        LOG(LOG_INFO, "screen_blt[%d](r(%d, %d, %d, %d) srcx=%d srcy=%d rop=%d clip(%d, %d, %d, %d)",
            this->order_count, r.x, r.y, r.cx, r.cy, srcx, srcy, rop, clip.x, clip.y, clip.cx, clip.cy);

        RDPScrBlt cmd(r, rop, srcx, srcy);
        RDPOrderCommon newcommon(SCREENBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->scrblt);
        this->common = newcommon;
        this->scrblt = cmd;
    }

    void dest_blt(const Rect & r, uint8_t rop, const Rect &clip)
    {
        this->reserve_order(21);

        LOG(LOG_INFO, "dest_blt[%d](r(%d, %d, %d, %d) rop=%d clip(%d, %d, %d, %d)",
            this->order_count, r.x, r.y, r.cx, r.cy, rop, clip.x, clip.y, clip.cx, clip.cy);

        RDPDestBlt cmd(r, rop);
        RDPOrderCommon newcommon(DESTBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->destblt);
        this->common = newcommon;
        this->destblt = cmd;
    }

    void pat_blt(const Rect & r, int rop, uint32_t bg_color, uint32_t fg_color, const RDPBrush & brush, const Rect &clip)
    {
        this->reserve_order(29);

        LOG(LOG_INFO, "pat_blt[%d](r(%d, %d, %d, %d) bg_color=%x fg_color=%d brush.style=%d clip(%d, %d, %d, %d)",
            this->order_count, r.x, r.y, r.cx, r.cy, bg_color, fg_color, brush.style, clip.x, clip.y, clip.cx, clip.cy);

        RDPPatBlt cmd(r, (uint8_t)rop, bg_color, fg_color, brush);
        RDPOrderCommon newcommon(PATBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->patblt);
        this->common = newcommon;
        this->patblt = cmd;
    }

    void mem_blt(int cache_id,
                int color_table, const Rect & r,
                int rop, int srcx, int srcy,
                int cache_idx, const Rect & clip)
    {
        this->reserve_order(30);

        LOG(LOG_INFO, "mem_blt[%d](cache_id=%d color_table=%d r(%d, %d, %d, %d) rop=%d srcx=%d srcy=%d cache_idx=%d clip(%d, %d, %d, %d)", this->order_count, cache_id, color_table, r.x, r.y, r.cx, r.cy, rop, srcx, srcy, cache_idx, clip.x, clip.y, clip.cx, clip.cy);

        RDPMemBlt cmd(cache_id + color_table * 256, r, rop, srcx, srcy, cache_idx);
        RDPOrderCommon newcommon(MEMBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->memblt);
        this->common = newcommon;
        this->memblt = cmd;
    }

    void line_to(int back_mode, int startx, int starty,
             int endx, int endy, int rop2, int back_color,
             const RDPPen & pen,
             const Rect & clip)
    {
        this->reserve_order(32);

        LOG(LOG_INFO, "line[%d](back_mode=%d startx=%d starty=%d, endx=%d endy=%d rop2=%d back_color=%x pen.color=%x clip(%d, %d, %d, %d)", this->order_count, back_mode, startx, starty, endx, endy, rop2, back_color, pen.color, clip.x, clip.y, clip.cx, clip.cy);


        RDPLineTo cmd(back_mode, startx, starty, endx, endy, back_color, rop2, pen);
        RDPOrderCommon newcommon(LINE, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->lineto);
        this->common = newcommon;
        this->lineto = cmd;
    }



    /*****************************************************************************/
    /* returns error */
    void glyph_index(int font, int flags, int mixmode, int fg_color, int bg_color,
             const Rect & text_clip, const Rect & box, int x, int y, uint8_t* data,
             int data_len, const Rect & clip)
    {

        #warning this should move out of Order
        if (clip.intersect(text_clip).isempty()){
            return;
        }

        this->reserve_order(297);

        LOG(LOG_INFO, "glyph_index[%d](font=%d flags=%d mixmode=%d, fg_color=%x bg_color=%x text_clip(%d, %d, %d, %d) box(%d, %d, %d, %d), x=%d, y=%d data_len=%d clip(%d, %d, %d, %d)", this->order_count, font, flags, mixmode, fg_color, bg_color, text_clip.x, text_clip.y, text_clip.cx, text_clip.cy, box.x, box.y, box.cx, box.cy, x, y, data_len, clip.x, clip.y, clip.cx, clip.cy);


        RDPOrderCommon newcommon(GLYPHINDEX, clip);
        RDPGlyphIndex cmd(font, flags, 0, mixmode,
                         fg_color,
                         bg_color,
                         text_clip,
                         box,
                         RDPBrush(),
                         x, y,
                         data_len, data);

        cmd.emit(this->out_stream, newcommon, this->common, this->text);
        this->common = newcommon;
        this->text = cmd;
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
        this->reserve_order(2000);

        LOG(LOG_INFO, "send_palette[%d](cache_id=%d)\n", this->order_count, cache_id);

        RDPColCache newcmd;
        memcpy(newcmd.palette[0], palette, 256);
        newcmd.emit(this->out_stream, 0);
        return 0;
    }

    int send_brush(int width, int height, int bpp, int type, int size, uint8_t* data, int cache_id)
    {
        using namespace RDP;

        this->reserve_order(size + 12);

        LOG(LOG_INFO, "send_brush[%d](width=%d, height=%d bpp=%d type=%d, size=%d, cache_id=%d)\n", this->order_count, width, height, bpp, type, size, cache_id);

        int order_flags = STANDARD | SECONDARY;
        this->out_stream.out_uint8(order_flags);
        int len = (size + 6) - 7; /* length after type minus 7 */
        this->out_stream.out_uint16_le(len);
        this->out_stream.out_uint16_le(0); /* flags */
        this->out_stream.out_uint8(BRUSHCACHE); /* type */
        this->out_stream.out_uint8(cache_id);
        this->out_stream.out_uint8(bpp);
        this->out_stream.out_uint8(width);
        this->out_stream.out_uint8(height);
        this->out_stream.out_uint8(type);
        this->out_stream.out_uint8(size);
        this->out_stream.out_copy_bytes(data, size);

        return 0;
    }

    /*****************************************************************************/
    /* returns error */
    /* max size width * height * Bpp + 16 */
    void send_bitmap(Stream & stream, Bitmap & bmp, int cache_id, int cache_idx)
    {

        Stream tmp(16384);
        bmp.compress(tmp);
        size_t bufsize = tmp.p - tmp.data;

        this->reserve_order(bufsize + 16);

        LOG(LOG_INFO, "send_bitmap[%d](cache_id=%d, cache_idx=%d)\n", this->order_count, cache_id, cache_idx);

        int order_flags = STANDARD | SECONDARY;
        stream.out_uint8(order_flags);
        stream.out_uint16_le(bufsize + 10);
        stream.out_uint16_le(8); /* flags */
        stream.out_uint8(TS_CACHE_BITMAP_COMPRESSED); /* type */
        stream.out_uint8(cache_id);
        stream.out_clear_bytes(1); /* pad */
        stream.out_uint8(align4(bmp.cx));
        stream.out_uint8(bmp.cy);
        stream.out_uint8(bmp.bpp);
        stream.out_uint16_le(bufsize/* + 8*/);
        stream.out_uint16_le(cache_idx);
        stream.out_clear_bytes(2); /* pad */
        stream.out_uint16_le(bufsize);
        stream.out_uint16_le(bmp.line_size);
        stream.out_uint16_le(bmp.bmp_size); /* final size */
        #warning we should compress directly into main stream to avoid useless copy
        stream.out_copy_bytes(tmp.data, bufsize);
    }


    void send_bitmap_small_headers(Stream & stream, Bitmap & bmp, int cache_id, int cache_idx)
    {
        Stream tmp(16384);
        bmp.compress(tmp);
        size_t bufsize = tmp.p - tmp.data;

        this->reserve_order(bufsize + 16);

        LOG(LOG_INFO, "send_bitmap_small_headers[%d](cache_id=%d, cache_idx=%d)\n", this->order_count, cache_id, cache_idx);

        int order_flags = STANDARD | SECONDARY;
        stream.out_uint8(order_flags);
        /* length after type minus 7 */
        stream.out_uint16_le(bufsize + 2);
        stream.out_uint16_le(1024); /* flags */
        stream.out_uint8(TS_CACHE_BITMAP_COMPRESSED); /* type */

        stream.out_uint8(cache_id);
        stream.out_clear_bytes(1); /* pad */

        stream.out_uint8(align4(bmp.cx));
        stream.out_uint8(bmp.cy);
        stream.out_uint8(bmp.bpp);
        stream.out_uint16_le(bufsize/* + 8*/);
        stream.out_uint16_le(cache_idx);
        stream.out_copy_bytes(tmp.data, bufsize);
    }


    void send_bitmap2(Stream & stream, Bitmap & bmp, int cache_id, int cache_idx)
    {
        Stream tmp(16384);
        bmp.compress(tmp);
        size_t bufsize = tmp.p - tmp.data;

        int Bpp = nbbytes(bmp.bpp);
        this->reserve_order(bufsize + 14);

        LOG(LOG_INFO, "send_bitmap2[%d](cache_id=%d, cache_idx=%d)\n", this->order_count, cache_id, cache_idx);

        stream.out_uint8(STANDARD | SECONDARY);

        stream.out_uint16_le(bufsize - 1); /* length after type minus 7 */
        stream.out_uint16_le(0x400 | (((Bpp + 2) << 3) & 0x38) | (cache_id & 7)); /* flags */
        stream.out_uint8(TS_CACHE_BITMAP_COMPRESSED_REV2); /* type */
        stream.out_uint8(align4(bmp.cx));
        stream.out_uint8(bmp.cy);
        stream.out_uint16_be(bufsize | 0x4000);
        stream.out_uint8(((cache_idx >> 8) & 0xff) | 0x80);
        stream.out_uint8(cache_idx);
        stream.out_copy_bytes(tmp.data, bufsize);
    }


    int get_compression_type()
    {
        int compressed_cache_type = 0;
        switch (((this->rdp_layer->client_info.bitmap_cache_version != 0) * 4)
              + ((this->rdp_layer->client_info.use_bitmap_comp      != 0) * 2)
              +  (this->rdp_layer->client_info.op2                  != 0)    ){
        case 0: case 1:
            compressed_cache_type = NOT_COMPRESSED;
            break;
        case 2:
            compressed_cache_type = COMPRESSED;
            break;
        case 3:
            compressed_cache_type = COMPRESSED_SMALL_HEADERS;
            break;
        case 4: case 5:
            compressed_cache_type = NEW_NOT_COMPRESSED;
            break;
        case 6: case 7:
            compressed_cache_type = NEW_COMPRESSED;
            break;
        }
        return compressed_cache_type;
    }

    void send_bitmap_common(Bitmap & bmp, uint8_t cache_id, uint16_t cache_idx)
    {
        using namespace RDP;

        switch (this->get_compression_type()){
        case NOT_COMPRESSED:
        {
            RDPBmpCache bmp_order(TS_CACHE_BITMAP_UNCOMPRESSED, bmp, cache_id, cache_idx);
            // check reserved size depending on version
            this->reserve_order(align4(bmp.cx * nbbytes(bmp.bpp)) * bmp.cy + 16);

            LOG(LOG_INFO, "send_bitmap_common::UNCOMPRESSED[%d](cache_id=%d, cache_idx=%d)\n", this->order_count, cache_id, cache_idx);

            bmp_order.emit(this->out_stream);
            bmp_order.data = 0;
        }
        break;
        case NEW_NOT_COMPRESSED:
        {
            RDPBmpCache bmp_order(TS_CACHE_BITMAP_UNCOMPRESSED_REV2, bmp, cache_id, cache_idx);
            // check reserved size depending on version
            this->reserve_order(align4(bmp.cx * nbbytes(bmp.bpp)) * bmp.cy + 16);

            LOG(LOG_INFO, "send_bitmap_common::UNCOMPRESSED_REV2[%d](cache_id=%d, cache_idx=%d)\n", this->order_count, cache_id, cache_idx);

            bmp_order.emit(this->out_stream);
            bmp_order.data = 0;
        }
        break;
        case COMPRESSED:
        {
            this->send_bitmap(this->out_stream, bmp, cache_id, cache_idx);
        }
        break;
        case COMPRESSED_SMALL_HEADERS:
        {
            this->send_bitmap_small_headers(this->out_stream, bmp, cache_id, cache_idx);
        }
        break;
        case NEW_COMPRESSED:
        {
            this->send_bitmap2(this->out_stream, bmp, cache_id, cache_idx);
        }
        break;
        }
    }

    void send_font(struct FontChar* font_char, int font_index, int char_index)
    {
        LOG(LOG_INFO, "send_font[%d](font_index=%d, char_index=%d)\n", this->order_count, font_index, char_index);

        int datasize = font_char->datasize();
        this->reserve_order(datasize + 18);
        int order_flags = STANDARD | SECONDARY;
        this->out_stream.out_uint8(order_flags);
        int len = (datasize + 12) - 7; /* length after type minus 7 */
        this->out_stream.out_uint16_le(len);
        this->out_stream.out_uint16_le(8); /* flags */
        this->out_stream.out_uint8(FONTCACHE); /* type */
        this->out_stream.out_uint8(font_index);
        this->out_stream.out_uint8(1); /* num of chars */
        this->out_stream.out_uint16_le(char_index);
        this->out_stream.out_uint16_le(font_char->offset);
        this->out_stream.out_uint16_le(font_char->baseline);
        this->out_stream.out_uint16_le(font_char->width);
        this->out_stream.out_uint16_le(font_char->height);
        this->out_stream.out_copy_bytes(font_char->data, datasize);
    }

};
} /* namespaces */

#endif
