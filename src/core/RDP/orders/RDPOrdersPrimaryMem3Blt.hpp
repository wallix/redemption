/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Zhou Raphael

   New RDP Orders Coder / Decoder : Primary Order Mem 3 Blt
*/


#pragma once

// MS-RDPEGDI 2.2.2.2.1.1.2.10 Mem3Blt (MEM3BLT_ORDER)
// ---------------------------------------------------
// The Mem3Blt Primary Drawing Order is used to render a bitmap stored in the
//  bitmap cache or offscreen bitmap cache to the screen by using a specified
//  brush and three-way raster operation.
//
//   Encoding order number: 14 (0x0E)
//   Negotiation order number: 4 (0x04)
//   Number of fields: 16
//   Number of field encoding bytes: 3
//   Maximum encoded field length: 34 bytes
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       cacheId (optional)      |      nLeftRect (variable)     |
// +-------------------------------+-------------------------------+
// |      nTopRect (variable)      |       nWidth (variable)       |
// +-------------------------------+---------------+---------------+
// |       nHeight (variable)      |      bRop     |     nXSrc     |
// |                               |   (optional)  |   (variable)  |
// +---------------+---------------+---------------+---------------+
// |      ...      |        nYSrc (variable)       |   BackColor   |
// |               |                               |   (optional)  |
// +---------------+---------------+---------------+---------------+
// |                               |      ForeColor (optional)     |
// +---------------+---------------+---------------+---------------+
// |      ...      |   BrushOrgX   |   BrushOrgY   |   BrushStyle  |
// |               |   (optional)  |   (optional)  |   (optional)  |
// +---------------+---------------+---------------+---------------+
// |   BrushHatch  |             BrushExtra (optional)             |
// +---------------+-----------------------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |     cacheIndex (optional)     |
// +-------------------------------+

// cacheId (2 bytes): A 16-bit, unsigned integer. The cacheId field contains
//  the encoded bitmap cache ID and Color Table Cache entry. The high byte
//  contains the index of the color table entry to use (cached previously
//  with a Cache Color Table (section 2.2.2.2.1.2.4) Secondary Drawing
//  Order), and the low byte contains the ID of the bitmap cache in which the
//  source bitmap is stored (cached previously with a Cache Bitmap - Revision
//  1 (section 2.2.2.2.1.2.2) or Cache Bitmap - Revision 2 (section
//  2.2.2.2.1.2.3) Secondary Drawing Order).
//
//  The color table entry MUST be in the range 0 to 5 (inclusive).
//
//  If the client advertised support for the Revision 1 bitmap caches
//   (section 3.1.1.1.1) by sending the server the Revision 1 Bitmap Cache
//   Capability Set ([MS-RDPBCGR] section 2.2.7.1.4.1), then the cache ID
//   MUST be in the range 0 to 2 (inclusive). However, if the client
//   advertised support for the Revision 2 bitmap caches (section 3.1.1.1.1)
//   by sending the server the Revision 2 Bitmap Cache Capability Set
//   ([MS-RDPBCGR] section 2.2.7.1.4.2), then the cache ID MUST be greater
//   than or equal to 0 and less than the number of bitmap caches being used
//   for the connection (specified by the NumCellCaches field of the Revision
//   2 Bitmap Cache Capability Set).
//
//  If the bitmap cache ID is set to TS_BITMAPCACHE_SCREEN_ID (0xFF), the
//   cacheIndex field MUST contain the index of an entry in the Offscreen
//   Bitmap Cache that contains the source bitmap.

// nLeftRect (variable): The left coordinate of the blit rectangle specified
//  by using a Coord Field (section 2.2.2.2.1.1.1.1).

// nTopRect (variable): The top coordinate of the blit rectangle specified by
//  using a Coord Field (section 2.2.2.2.1.1.1.1).

// nWidth (variable): The width of the blit rectangle specified by using a
//  Coord Field (section 2.2.2.2.1.1.1.1).

// nHeight (variable): The height of the blit rectangle specified by using a
//  Coord Field (section 2.2.2.2.1.1.1.1).

// bRop (1 byte): The index of the ternary raster operation to perform (see
//  section 2.2.2.2.1.1.1.7).

// nXSrc (variable): The x-coordinate of the source rectangle within the
//  source bitmap specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// nYSrc (variable): The inverted y-coordinate of the source rectangle within
//  the source bitmap specified using a Coord Field (section
//  2.2.2.2.1.1.1.1). The actual value of the y-coordinate MUST be computed
//  using the following formula:
//
//   ActualYSrc = (SourceBitmapHeight - nHeight) - nYSrc

// BackColor (3 bytes): Background color described by using a Generic Color
//  (section 2.2.2.2.1.1.1.8) structure.

// ForeColor (3 bytes): Foreground color described by using a Generic Color
//  (section 2.2.2.2.1.1.1.8) structure.

// BrushOrgX (1 byte): An 8-bit, signed integer. The x-coordinate of the
//  point where the top leftmost pixel of a brush pattern MUST be anchored.

// BrushOrgY (1 byte): An 8-bit, signed integer. The y-coordinate of the
//  point where the top leftmost pixel of a brush pattern MUST be anchored.

// BrushStyle (1 byte): An 8-bit, unsigned integer. The contents and format
//  of this field are the same as the BrushStyle field of the PatBlt (section
//  2.2.2.2.1.1.2.3) Primary Drawing Order.

// BrushHatch (1 byte): An 8-bit, unsigned integer. The contents and format
//  of this field are the same as the BrushHatch field of the PatBlt (section
//  2.2.2.2.1.1.2.3) Primary Drawing Order.

// BrushExtra (7 bytes): The contents and format of this field are the same
//  as the BrushExtra field of the PatBlt (section 2.2.2.2.1.1.2.3) Primary
//  Drawing Order.

// cacheIndex (2 bytes): A 16-bit, unsigned integer. The index of the source
//  bitmap within the bitmap cache specified by the cacheId field.
//
//  If this field is set to BITMAPCACHE_WAITING_LIST_INDEX (32767), the last
//   bitmap cache entry MUST contain the bitmap data. Otherwise, this field
//   MUST contain a value that is greater than or equal to 0 and less than
//   the maximum number of entries allowed in the source bitmap cache. If the
//   client advertised support for the Revision 1 bitmap caches (section
//   3.1.1.1.1) by sending the server the Revision 1 Bitmap Cache Capability
//   Set ([MS-RDPBCGR] section 2.2.7.1.4.1), then the maximum number of
//   entries allowed in each individual Revision 1 bitmap cache is specified
//   in the Revision 1 Bitmap Cache Capability Set by the Cache0Entries,
//   Cache1Entries and Cache2Entries fields. If the client advertised support
//   for the Revision 2 bitmap caches (section 3.1.1.1.1) by sending the
//   server the Revision 2 Bitmap Cache Capability Set ([MS-RDPBCGR] section
//   2.2.7.1.4.2), then the maximum number of entries allowed in each
//   individual Revision 2 bitmap cache is specified in the Revision 2 Bitmap
//   Cache Capability Set ([MS-RDPBCGR] section 2.2.7.1.4.2) by the
//   BitmapCache0CellInfo, BitmapCache1CellInfo, BitmapCache2CellInfo,
//   BitmapCache3CellInfo and BitmapCache4CellInfo fields.
//
//  If the bitmap cache ID (specified in the cacheId field) is set to
//   TS_BITMAPCACHE_SCREEN_ID (0xFF), this field MUST contain the index of an
//   entry in the Offscreen Bitmap Cache that contains the source bitmap.

#include "core/RDP/orders/RDPOrdersCommon.hpp"

class RDPMem3Blt {
    public:
    uint16_t    cache_id;
    Rect        rect;
    uint8_t     rop;
    uint16_t    srcx;
    uint16_t    srcy;
    RDPColor    back_color;
    RDPColor    fore_color;
    RDPBrush    brush;
    uint16_t    cache_idx;

    static uint8_t id(void) {
        return RDP::MEM3BLT;
    }

    RDPMem3Blt( uint16_t cache_id
              , const Rect rect
              , uint8_t rop
              , uint16_t srcx
              , uint16_t srcy
              , RDPColor back_color
              , RDPColor fore_color
              , const RDPBrush & brush
              , uint16_t cache_idx)
    : cache_id(cache_id)
    , rect(rect)
    , rop(rop)
    , srcx(srcx)
    , srcy(srcy)
    , back_color(back_color)
    , fore_color(fore_color)
    , brush(brush)
    , cache_idx(cache_idx) {
    }

    bool operator==(const RDPMem3Blt &other) const {
        return (this->cache_id   == other.cache_id)
            && (this->rect       == other.rect)
            && (this->rop        == other.rop)
            && (this->srcx       == other.srcx)
            && (this->srcy       == other.srcy)
            && (this->back_color == other.back_color)
            && (this->fore_color == other.fore_color)
            && (this->brush      == other.brush)
            && (this->cache_idx  == other.cache_idx)
            ;
    }

    void emit( OutStream & stream
             , RDPOrderCommon & common
             , const RDPOrderCommon & oldcommon
             , const RDPMem3Blt & oldcmd) const {
        using namespace RDP;

        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)) {
            header.control |= BOUNDS;
        }

        // MEM3BLT fields bytes
        // ------------------------------
        // 0x0001: cacheId (2 bytes)
        // 0x0002: x coordinate (variable)
        // 0x0004: y coordinate (variable)
        // 0x0008: cx coordinate (variable)
        // 0x0010: cy coordinate (variable)
        // 0x0020: rop byte (1 byte)
        // 0x0040: srcx (variable)
        // 0x0080: srcy (variable)

        // 0x0100: BackColor (3 bytes)
        // 0x0200: ForeColor (3 bytes)
        // 0x0400: Brush Org X (1 byte)
        // 0x0800: Brush Org Y (1 byte)
        // 0x1000: Brush Style (1 byte)
        // 0x2000: Brush Hatch (1 byte)
        // 0x4000: Brush Extra (7 bytes)
        // 0x8000: cacheidx (2 bytes)

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we have bounds.
        //  Can't see the rationale and rdesktop don't do it by the book.
        //  Behavior should be checked with server and clients from
        //  Microsoft. Looks like an error in RDP specs.
        header.control |=
            (  (dr.fully_relative()
            && is_1_byte(this->srcx - oldcmd.srcx)
            && is_1_byte(this->srcy - oldcmd.srcy))
                                                    * DELTA);

        header.fields =
              (this->cache_id             != oldcmd.cache_id         ) * 0x0001

            | (dr.dleft                   != 0                       ) * 0x0002
            | (dr.dtop                    != 0                       ) * 0x0004
            | (dr.dwidth                  != 0                       ) * 0x0008
            | (dr.dheight                 != 0                       ) * 0x0010

            | (this->rop                  != oldcmd.rop              ) * 0x0020

            | ((this->srcx - oldcmd.srcx) != 0                       ) * 0x0040
            | ((this->srcy - oldcmd.srcy) != 0                       ) * 0x0080

            | (this->back_color           != oldcmd.back_color       ) * 0x0100
            | (this->fore_color           != oldcmd.fore_color       ) * 0x0200

            | (this->brush.org_x          != oldcmd.brush.org_x      ) * 0x0400
            | (this->brush.org_y          != oldcmd.brush.org_y      ) * 0x0800
            | (this->brush.style          != oldcmd.brush.style      ) * 0x1000
            | (this->brush.hatch          != oldcmd.brush.hatch      ) * 0x2000
            | (memcmp(this->brush.extra, oldcmd.brush.extra, 7) != 0 ) * 0x4000

            | (this->cache_idx            != oldcmd.cache_idx        ) * 0x8000
            ;

        common.emit(stream, header, oldcommon);

        if (header.fields & 0x0001) {
            stream.out_uint16_le(this->cache_id);
        }

        header.emit_rect(stream, 0x0002, this->rect, oldcmd.rect);

        if (header.fields & 0x0020) {
            stream.out_uint8(this->rop);
        }

        header.emit_src(stream, 0x0040, this->srcx, this->srcy, oldcmd.srcx, oldcmd.srcy);

        if (header.fields & 0x0100) {
            emit_rdp_color(stream, this->back_color);
        }
        if (header.fields & 0x0200) {
            emit_rdp_color(stream, this->fore_color);
        }

        header.emit_brush(stream, 0x0400, this->brush, oldcmd.brush);

        if (header.fields & 0x8000){
            stream.out_uint16_le(this->cache_idx);
        }
    }

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header) {
        using namespace RDP;

        if (header.fields & 0x0001) {
            this->cache_id = stream.in_uint16_le();
        }

        header.receive_rect(stream, 0x0002, this->rect);

        if (header.fields & 0x0020) {
            this->rop = stream.in_uint8();
        }

        header.receive_src(stream, 0x0040, this->srcx, this->srcy);

        if (header.fields & 0x0100) {
            receive_rdp_color(stream, this->back_color);
        }

        if (header.fields & 0x0200) {
            receive_rdp_color(stream, this->fore_color);
        }

        header.receive_brush(stream, 0x0400, this->brush);

        if (header.fields & 0x8000) {
            this->cache_idx = stream.in_uint16_le();
        }
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const {
        // TODO remove flag in common.str to draw clip or not, it is confusing. Better to have 2 functions
        size_t lg = common.str(buffer, sz, !common.clip.contains(this->rect));
        lg  += snprintf(
            buffer+lg,
            sz-lg,
            "mem3blt(cache_id=%d rect(%d,%d,%d,%d) rop=%x srcx=%d srcy=%d "
                "back_color=%.6x fore_color=%.6x "
                "brush.(org_x=%d, org_y=%d, style=%d hatch=%d extra=[%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x]) "
                "cache_idx=%d)",
            this->cache_id,
            this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
            unsigned(this->rop),
            this->srcx, this->srcy,
            this->back_color.as_bgr().to_u32(),
            this->fore_color.as_bgr().to_u32(),
            this->brush.org_x, this->brush.org_y, this->brush.style, this->brush.hatch,
            unsigned(this->brush.extra[0]), unsigned(this->brush.extra[1]),
            unsigned(this->brush.extra[2]), unsigned(this->brush.extra[3]),
            unsigned(this->brush.extra[4]), unsigned(this->brush.extra[5]),
            unsigned(this->brush.extra[6]), this->cache_idx);
        if (lg >= sz) {
            return sz;
        }
        return lg;
    }

    void log(int level, const Rect clip) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }

    void move(int offset_x, int offset_y) {
        this->rect = this->rect.offset(offset_x, offset_y);
    }
};

