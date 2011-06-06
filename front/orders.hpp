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
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPScrBlt scrblt;
    RDPOpaqueRect opaquerect;
    RDPMemBlt memblt;
    RDPLineTo lineto;
    RDPGlyphIndex text;

    Stream out_stream;

    uint8_t* order_count_ptr;
    int order_count;
    int order_level;

    Orders() :
        common(0, Rect(0, 0, 1, 1)),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        scrblt(Rect(), 0, 0, 0),
        opaquerect(Rect(), 0),
        memblt(0, Rect(), 0, 0, 0, 0),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        text(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),
        out_stream(16384)
    {
        this->order_count = 0;
        this->order_level = 0;
    }

    ~Orders()
    {
    }

    void reset_xx() throw (Error)
    {
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


    void opaque_rect(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        RDPOrderCommon newcommon(RECT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->opaquerect);
        this->common = newcommon;
        this->opaquerect = cmd;
    }

    void scr_blt(const RDPScrBlt & cmd, const Rect & clip)
    {
        RDPOrderCommon newcommon(SCREENBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->scrblt);
        this->common = newcommon;
        this->scrblt = cmd;
    }

    void dest_blt(const RDPDestBlt & cmd, const Rect &clip)
    {
        RDPOrderCommon newcommon(DESTBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->destblt);
        this->common = newcommon;
        this->destblt = cmd;
    }

    void pat_blt(const RDPPatBlt & cmd, const Rect &clip)
    {
        RDPOrderCommon newcommon(PATBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->patblt);
        this->common = newcommon;
        this->patblt = cmd;
    }

    void mem_blt(const RDPMemBlt & cmd, const Rect & clip)
    {
//        RDPMemBlt cmd(cache_id + color_table * 256, r, rop, srcx, srcy, cache_idx);
        RDPOrderCommon newcommon(MEMBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->memblt);
        this->common = newcommon;
        this->memblt = cmd;
    }

    void line_to(const RDPLineTo & cmd, const Rect & clip)
    {
        RDPOrderCommon newcommon(LINE, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->lineto);
        this->common = newcommon;
        this->lineto = cmd;
    }



    /*****************************************************************************/
    void glyph_index(const RDPGlyphIndex & glyph_index, const Rect & clip)
    {

//        LOG(LOG_INFO, "glyph_index[%d](font=%d flags=%d mixmode=%d, fg_color=%x bg_color=%x text_clip(%d, %d, %d, %d) box(%d, %d, %d, %d), x=%d, y=%d data_len=%d clip(%d, %d, %d, %d)", this->order_count, font, flags, mixmode, fg_color, bg_color, text_clip.x, text_clip.y, text_clip.cx, text_clip.cy, box.x, box.y, box.cx, box.cy, x, y, data_len, clip.x, clip.y, clip.cx, clip.cy);

        RDPOrderCommon newcommon(GLYPHINDEX, clip);
        glyph_index.emit(this->out_stream, newcommon, this->common, this->text);
        this->common = newcommon;
        this->text = glyph_index;
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

    void send_palette(const uint32_t (& palette)[256], int cache_id)
    {
//        LOG(LOG_INFO, "send_palette[%d](cache_id=%d)\n", this->order_count, cache_id);

        RDPColCache newcmd;
        memcpy(newcmd.palette[0], palette, 256);
        newcmd.emit(this->out_stream, 0);
    }

    void send_brush(int width, int height, int bpp, int type, int size, uint8_t* data, int cache_id)
    {
        using namespace RDP;

//        LOG(LOG_INFO, "send_brush[%d](width=%d, height=%d bpp=%d type=%d, size=%d, cache_id=%d)\n", this->order_count, width, height, bpp, type, size, cache_id);

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
    }

    void send_bitmap_common(ClientInfo* client_info, Bitmap & bmp, uint8_t cache_id, uint16_t cache_idx)
    {
        using namespace RDP;

        RDPBmpCache bmp_order(&bmp, cache_id, cache_idx, client_info);

//        LOG(LOG_INFO, "/* send_bitmap[%d](bmp(bpp=%d, cx=%d, cy=%d, data=%p), cache_id=%d, cache_idx=%d) */\n", this->order_count, bmp.bpp, bmp.cx, bmp.cy, bmp.data_co, cache_id, cache_idx);

        bmp_order.emit(this->out_stream);
    }

    void send_font(const FontChar & font_char, int font_index, int char_index)
    {

        int datasize = font_char.datasize();

//        LOG(LOG_INFO, "send_font[%d](font_index=%d, char_index=%d)\n", this->order_count, font_index, char_index);

        int order_flags = STANDARD | SECONDARY;
        this->out_stream.out_uint8(order_flags);
        int len = (datasize + 12) - 7; /* length after type minus 7 */
        this->out_stream.out_uint16_le(len);
        this->out_stream.out_uint16_le(8); /* flags */
        this->out_stream.out_uint8(FONTCACHE); /* type */
        this->out_stream.out_uint8(font_index);
        this->out_stream.out_uint8(1); /* num of chars */
        this->out_stream.out_uint16_le(char_index);
        this->out_stream.out_uint16_le(font_char.offset);
        this->out_stream.out_uint16_le(font_char.baseline);
        this->out_stream.out_uint16_le(font_char.width);
        this->out_stream.out_uint16_le(font_char.height);
        this->out_stream.out_copy_bytes(font_char.data, datasize);
    }

};
} /* namespaces */

#endif
