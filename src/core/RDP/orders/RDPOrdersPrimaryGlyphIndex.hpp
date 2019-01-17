/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou

    New RDP Orders Coder / Decoder : Primary Order GlyphIndex
*/


#pragma once

#include "core/RDP/orders/RDPOrdersCommon.hpp"

// [MS-RDPEGDI] - 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)
// =============================================================

// The GlyphIndex Primary Drawing Order encodes a set of glyph indices at a
//  specified position.

//  Encoding order number: 27 (0x1B)
//  Negotiation order number: 27 (0x1B)
//  Number of fields: 22
//  Number of field encoding bytes: 3
//  Maximum encoded field length: 297 bytes

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |    cacheId    |    flAccel    |   ulCharInc   |   BackColor   |
// |   (optional)  |   (optional)  |   (optional)  |   (optional)  |
// +---------------+---------------+---------------+---------------+
// |            fOpRedundant (optional)            |   ForeColor   |
// |                                               |   (optional)  |
// +-------------------------------+---------------+---------------+
// |              ...              |       BkLeft (optional)       |
// +-------------------------------+---------------+---------------+
// |        BkTop (optional)       |       BkRight (optional)      |
// +-------------------------------+-------------------------------+
// |      BkBottom (optional)      |       OpLeft (optional)       |
// +-------------------------------+-------------------------------+
// |        OpTop (optional)       |       OpRight (optional)      |
// +-------------------------------+---------------+---------------+
// |      OpBottom (optional)      |   BrushOrgX   |   BrushOrgY   |
// |                               |   (optional)  |   (optional)  |
// +---------------+---------------+---------------+---------------+
// |   BrushStyle  |   BrushHatch  |     BrushExtra (optional)     |
// |   (optional)  |   (optional)  |                               |
// +---------------+---------------+-------------------------------+
// |                              ...                              |
// +---------------+-------------------------------+---------------+
// |      ...      |          X (optional)         |  Y (optional) |
// +---------------+-------------------------------+---------------+
// |      ...      |    VariableBytes (variable)   |
// +---------------+-------------------------------+

// cacheId (1 byte): An 8-bit, unsigned integer. The ID of the glyph cache in
//  which the glyph data MUST be stored. This value MUST be in the range 0 to
//  9 (inclusive).

// flAccel (1 byte): An 8-bit, unsigned integer. Accelerator flags. For glyph
//  related terminology, see [YUAN] figures 14-17 and 15-1. For information
//  about string widths and heights, see [MSDN-SWH]. For information about
//  character widths, see [MSDN-CW]. This field MUST contain a combination of
//  the following flags.

//  +---------------------------+----------------------------------------------+
//  | Value                     | Meaning                                      |
//  +---------------------------+----------------------------------------------+
//  | SO_FLAG_DEFAULT_PLACEMENT | This flag MUST be set.                       |
//  | 0x01                      |                                              |
//  +---------------------------+----------------------------------------------+
//  | SO_HORIZONTAL             | Text is horizontal, left-to-right or         |
//  | 0x02                      | right-to-left, depending on SO_REVERSED.     |
//  +---------------------------+----------------------------------------------+
//  | SO_VERTICAL               | Text is vertical, top-to-bottom or           |
//  | 0x04                      | bottom-to-top, depending on SO_REVERSED.     |
//  +---------------------------+----------------------------------------------+
//  | SO_REVERSED               | Set if horizontal text is right-to-left or   |
//  | 0x08                      | vertical text is bottom-to-top.              |
//  +---------------------------+----------------------------------------------+
//  | SO_ZERO_BEARINGS          | For a given glyph in the font, the A-width   |
//  | 0x10                      | (left-side bearing) and C-width (right-side  |
//  |                           | bearing) associated with the glyph have a    |
//  |                           | value of zero.                               |
//  +---------------------------+----------------------------------------------+
//  | SO_CHAR_INC_EQUAL_BM_BASE | For a given glyph in the font, the B-width   |
//  | 0x20                      | associated with the glyph equals the advance |
//  |                           | width of the glyph.                          |
//  +---------------------------+----------------------------------------------+
//  | SO_MAXEXT_EQUAL_BM_SIDE   | The height of the bitmap associated with a   |
//  | 0x40                      | given glyph in the font is always equal to   |
//  |                           | the sum of the ascent and descent. This      |
//  |                           | implies that the tops and bottoms of all     |
//  |                           | glyph bitmaps lie on the same line in the    |
//  |                           | direction of writing.                        |
//  +---------------------------+----------------------------------------------+

enum {
      SO_FLAG_DEFAULT_PLACEMENT = 0x01
    , SO_HORIZONTAL             = 0x02
    , SO_VERTICAL               = 0x04
    , SO_REVERSED               = 0x08
    , SO_ZERO_BEARINGS          = 0x10
    , SO_CHAR_INC_EQUAL_BM_BASE = 0x20
    , SO_MAXEXT_EQUAL_BM_SIDE   = 0x40
};

// ulCharInc (1 byte): An 8-bit, unsigned integer. Specifies whether or not
//  the font is a fixed-pitch (monospace) font. If so, this member is equal
//  to the advance width of the glyphs in pixels (see [YUAN] figures 14-17);
//  if not, this field is set to 0x00. The minimum value for this field is
//  0x00 (inclusive), and the maximum value is 0xFF (inclusive).

// fOpRedundant (1 byte): An 8-bit, unsigned integer. A Boolean value
//  indicating whether or not the opaque rectangle is redundant. Redundant,
//  in this context, means that the text background is transparent.

//  +-------+-----------------------------+
//  | Value | Meaning                     |
//  +-------+-----------------------------+
//  | FALSE | Rectangle is not redundant. |
//  | 0x00  |                             |
//  +-------+-----------------------------+
//  | TRUE  | Rectangle is redundant.     |
//  | 0x01  |                             |
//  +-------+-----------------------------+

// BackColor (3 bytes): The text color described by using a Generic Color
//  (section 2.2.2.2.1.1.1.8) structure.

// ForeColor (3 bytes): Color of the opaque rectangle described by using a
//  Generic Color (section 2.2.2.2.1.1.1.8) structure.

// BkLeft (2 bytes): A 16-bit, signed integer. The left coordinate of the
//  text background rectangle.

// BkTop (2 bytes): A 16-bit, signed integer. The top coordinate of the text
//  background rectangle.

// BkRight (2 bytes): A 16-bit, signed integer. The right coordinate of the
//  text background rectangle.

// BkBottom (2 bytes): A 16-bit, signed integer. The bottom coordinate of the
//  text background rectangle.

// OpLeft (2 bytes): A 16-bit, signed integer. The left coordinate of the
//  opaque rectangle. This field MUST be set to 0 if the fOpRedundant flag is
//  set.

// OpTop (2 bytes): A 16-bit, signed integer. The top coordinate of the
//  opaque rectangle. This field MUST be set to 0 if the fOpRedundant flag is
//  set.

// OpRight (2 bytes): A 16-bit, signed integer. The right coordinate of the
//  opaque rectangle. This field MUST be set to 0 if the fOpRedundant flag is
//  set.

// OpBottom (2 bytes): A 16-bit, signed integer. The bottom coordinate of the
//  opaque rectangle. This field MUST be set to 0 if the fOpRedundant flag is
//  set.

// BrushOrgX (1 byte): An 8-bit, signed integer. The x-coordinate of the
//  point where the top leftmost pixel of a brush pattern MUST be anchored.

// BrushOrgY (1 byte): An 8-bit, signed integer. The y-coordinate of the
//  point where the top leftmost pixel of a brush pattern MUST be anchored.

// BrushStyle (1 byte): An 8-bit, unsigned integer. This field MUST be set to
//  BS_SOLID (0x00), as the GlyphIndex Primary Drawing Order MUST only use
//  solid color brushes to render the opaque rectangle.

// BrushHatch (1 byte): An 8-bit, unsigned integer. This field MUST be set to
//  0x00, as the GlyphIndex Primary Drawing Order MUST only use solid color
//  brushes to render the opaque rectangle.

// BrushExtra (7 bytes): This field is not used, as the GlyphIndex Primary
//  Drawing Order MUST only use solid color brushes to render the opaque
//  rectangle.

// X (2 bytes): A 16-bit, signed integer. The x-coordinate of the point where
//  the origin of the starting glyph MUST be positioned.

// Y (2 bytes): A 16-bit, signed integer. The y-coordinate of the point where
//  the origin of the starting glyph MUST be positioned.

// VariableBytes (variable): A One-Byte Header Variable Field (section
//  2.2.2.2.1.1.1.2) structure. This field MUST contain glyph fragments
//  (which are composed of a series of one or more glyph cache indices) and
//  instructions to use entries previously stored in the glyph fragment
//  cache. Multiple glyph fragments can be contained in this field. The first
//  byte of each fragment is either a USE (0xFE) operation byte or a glyph
//  index (0x00 to 0x0FD) byte:

//  * A value of 0xFE (USE) indicates that a previously stored fragment MUST
//    be displayed. The byte following the USE byte is the index in the
//    fragment cache where the fragment is located. This fragment MUST be
//    read and displayed. If the ulCharInc field is set to 0 and the flAccel
//    field does not contain the SO_CHAR_INC_EQUAL_BM_BASE (0x20) flag, then
//    the index byte MUST be followed by a delta byte that indicates the
//    distance between two consecutive fragments; this distance is measured
//    in pixels from the beginning of the first fragment to the beginning of
//    the next. If the distance is greater than 127 (0x7F), then the value
//    0x80 MUST be used, and the following two bytes will be set to contain
//    the actual distance formatted as an unsigned integer in little-endian
//    order.

//  * If not preceded by 0xFE, a value of 0x00 to 0xFD identifies a glyph
//    stored at the given index in the glyph cache. Multiple glyphs can be
//    sent at one time. If the ulCharInc field is set to 0 and the flAccel
//    field does not contain the SO_CHAR_INC_EQUAL_BM_BASE (0x20) flag, then
//    the index byte MUST be followed by a delta byte that indicates the
//    distance between two consecutive glyphs; this distance is measured in
//    pixels from the beginning of the first glyph to the beginning of the
//    next. If the distance is greater than 127 (0x7F), then the value 0x80
//    MUST be used, and the following two bytes will be set to contain the
//    actual distance formatted as an unsigned integer in little-endian
//    order.

//  If a series of glyph indices ends with an ADD (0xFF) operation byte, the
//   preceding glyph information MUST be collected, displayed, and then
//   stored in the fragment cache. The byte following the ADD byte is the
//   index of the cache where the fragment MUST be stored. A final byte that
//   indicates the size of the fragment follows the index byte. (The ADD
//   byte, index byte, and size byte MUST NOT be counted when calculating the
//   value of the size byte.)

//  All glyph cache indices MUST be greater than or equal to 0, and less than
//   the maximum number of entries allowed in the glyph cache with the ID
//   specified by the cacheId field. The maximum number of entries allowed in
//   each of the ten glyph caches is specified in the GlyphCache field of the
//   Glyph Cache Capability Set ([MS-RDPBCGR] section 2.2.7.1.8).

//  All fragment cache indices MUST be in the range 0 to 255 (inclusive).

class RDPGlyphIndex {
    // GlyphIndex field encoding bytes (3)
    // ===================================

    // 0x01: cache_id (font = ID of the glyph cache)
    // 0x02: flAccel (flags)
    // 0x04: UICharInc
    // 0x08: fOpRedundant (mix_mode)
    // 0x10: back_color
    // 0x20: fore_color

    // Coordinates of background
    // 0x040: bk_left (x coordinate)
    // 0x080: bk_top (y coordinate)
    // 0x100: bk_right (x + cx coordinate)
    // 0x200: bk_bottom (y + cy coordinate)

    // Coordinates of opaque rectangle
    // 0x0400: op_left (x coordinate)
    // 0x0800: op_top (y coordinate)
    // 0x1000: op_right (x + cx coordinate)
    // 0x2000: op_bottom (y + cy coordinate)

    // 0x4000 - 0x40000 : brush

    // 0x280000 glyph_x
    // 0x100000 glyph_y
    // 0x200000 data_len, data

public:
    uint8_t  cache_id;
    int16_t  fl_accel;
    int16_t  ui_charinc;
    int16_t  f_op_redundant;

    RDPColor back_color;
    RDPColor fore_color;

    Rect     bk;
    Rect     op;

    RDPBrush brush;

    int16_t  glyph_x;
    int16_t  glyph_y;

    uint8_t  data_len;
    uint8_t  data[256];

    static uint8_t id() {
        return RDP::GLYPHINDEX;
    }

    RDPGlyphIndex( uint8_t cache_id, uint8_t fl_accel, uint8_t ui_charinc
                 , uint8_t f_op_redundant, RDPColor back_color, RDPColor fore_color
                 , const Rect bk, const Rect op, const RDPBrush & brush
                 , int16_t glyph_x, int16_t glyph_y
                 , uint8_t data_len, const uint8_t * data)
    : cache_id(cache_id)
    , fl_accel(fl_accel)
    , ui_charinc(ui_charinc)
    , f_op_redundant(f_op_redundant)
    , back_color(back_color)
    , fore_color(fore_color)
    , bk(bk)
    , op(op)
    , brush(brush)
    , glyph_x(glyph_x)
    , glyph_y(glyph_y)
    , data_len(data_len) {
        memcpy(this->data, data, data_len);
        memset(this->data + data_len, 0, sizeof(this->data) - data_len);
    }

    RDPGlyphIndex(const RDPGlyphIndex & gi)
    : cache_id(gi.cache_id)
    , fl_accel(gi.fl_accel)
    , ui_charinc(gi.ui_charinc)
    , f_op_redundant(gi.f_op_redundant)
    , back_color(gi.back_color)
    , fore_color(gi.fore_color)
    , bk(gi.bk)
    , op(gi.op)
    , brush(gi.brush)
    , glyph_x(gi.glyph_x)
    , glyph_y(gi.glyph_y)
    , data_len(gi.data_len) {
        memcpy(this->data, gi.data, gi.data_len);
        memset(this->data + gi.data_len, 0, sizeof(this->data) - gi.data_len);
    }

    RDPGlyphIndex & operator = (const RDPGlyphIndex &) = default;

    bool operator==(const RDPGlyphIndex & other) const {
        return  (this->cache_id       == other.cache_id)
             && (this->fl_accel       == other.fl_accel)
             && (this->ui_charinc     == other.ui_charinc)
             && (this->f_op_redundant == other.f_op_redundant)
             && (this->back_color     == other.back_color)
             && (this->fore_color     == other.fore_color)
             && (this->bk             == other.bk)
             && (this->op             == other.op)
             && (this->brush          == other.brush)
             && (this->glyph_x        == other.glyph_x)
             && (this->glyph_y        == other.glyph_y)
             && (this->data_len       == other.data_len)
             && (0 == memcmp(this->data, other.data, this->data_len))
             ;
    }

    void emit( OutStream & stream
             , RDPOrderCommon & common
             , const RDPOrderCommon & oldcommon
             , const RDPGlyphIndex & oldcmd) const {
        RDPPrimaryOrderHeader header(RDP::STANDARD, 0);

        if (!common.clip.contains(this->bk)) {
            header.control |= RDP::BOUNDS;
        }

        if (this->op.cx > 1 && !common.clip.contains(this->op)) {
            header.control |= RDP::BOUNDS;
        }

        // DELTA flag Does not seems to works as usual for this function
        // (at least in rdesktop)
        // hence coordinates fields are transmitted as absolute fields
        // DELTA may also raise some problems with empty rects,
        // if initial rect is empty DELTA should not be used
//        control |=
//            (is_1_byte(this->bk.x               - oldcmd.bk.x                )
//          && is_1_byte(this->bk.y               - oldcmd.bk.y                )
//          && is_1_byte(this->bk.x + this->bk.cx - oldcmd.bk.x + oldcmd.bk.cx )
//          && is_1_byte(this->bk.y + this->bk.cy - oldcmd.bk.y + oldcmd.bk.cy )
//          && is_1_byte(this->op.x               - oldcmd.op.x                )
//          && is_1_byte(this->op.y               - oldcmd.op.y                )
//          && is_1_byte(this->op.x + this->op.cx - oldcmd.op.x + oldcmd.op.cx )
//          && is_1_byte(this->op.y + this->op.cy - oldcmd.op.y + oldcmd.op.cy )
//          && is_1_byte(this->glyph_x            - oldcmd.glyph_x             )
//          && is_1_byte(this->glyph_y            - oldcmd.glyph_y             )
//                                                                    ) * DELTA;

        header.fields =
              (this->cache_id           != oldcmd.cache_id            ) * 0x000001
            | (this->fl_accel           != oldcmd.fl_accel            ) * 0x000002
            | (this->ui_charinc         != oldcmd.ui_charinc          ) * 0x000004
            | (this->f_op_redundant     != oldcmd.f_op_redundant      ) * 0x000008

            | (this->back_color         != oldcmd.back_color          ) * 0x000010
            | (this->fore_color         != oldcmd.fore_color          ) * 0x000020

            | (this->bk.x               != oldcmd.bk.x                ) * 0x000040
            | (this->bk.y               != oldcmd.bk.y                ) * 0x000080
            | (this->bk.x + this->bk.cx != oldcmd.bk.x + oldcmd.bk.cx ) * 0x000100
            | (this->bk.y + this->bk.cy != oldcmd.bk.y + oldcmd.bk.cy ) * 0x000200

            | (this->op.x               != oldcmd.op.x                ) * 0x000400
            | (this->op.y               != oldcmd.op.y                ) * 0x000800
            | (this->op.x + this->op.cx != oldcmd.op.x + oldcmd.op.cx ) * 0x001000
            | (this->op.y + this->op.cy != oldcmd.op.y + oldcmd.op.cy ) * 0x002000

            | (this->brush.org_x        != oldcmd.brush.org_x         ) * 0x004000
            | (this->brush.org_y        != oldcmd.brush.org_y         ) * 0x008000
            | (this->brush.style        != oldcmd.brush.style         ) * 0x010000
            | (this->brush.hatch        != oldcmd.brush.hatch         ) * 0x020000
            | (memcmp(this->brush.extra, oldcmd.brush.extra, 7) != 0  ) * 0x040000

            | (this->glyph_x            != oldcmd.glyph_x             ) * 0x080000
            | (this->glyph_y            != oldcmd.glyph_y             ) * 0x100000
            | (data_len                 != 0                          ) * 0x200000
            ;

        common.emit(stream, header, oldcommon);

        if (header.fields & 0x001) { stream.out_uint8(this->cache_id); }
        if (header.fields & 0x002) { stream.out_uint8(this->fl_accel); }
        if (header.fields & 0x004) { stream.out_uint8(this->ui_charinc); }
        if (header.fields & 0x008) { stream.out_uint8(this->f_op_redundant); }

        if (header.fields & 0x010) {
            emit_rdp_color(stream, this->back_color);
        }
        if (header.fields & 0x020) {
            emit_rdp_color(stream, this->fore_color);
        }

        if (header.fields & 0x0040) { stream.out_uint16_le(this->bk.x); }
        if (header.fields & 0x0080) { stream.out_uint16_le(this->bk.y); }
        if (header.fields & 0x0100) { stream.out_uint16_le(this->bk.x + this->bk.cx - 1); }
        if (header.fields & 0x0200) { stream.out_uint16_le(this->bk.y + this->bk.cy - 1); }

        if (header.fields & 0x0400) { stream.out_uint16_le(this->op.x); }
        if (header.fields & 0x0800) { stream.out_uint16_le(this->op.y); }
        if (header.fields & 0x1000) { stream.out_uint16_le(this->op.x + this->op.cx - 1); }
        if (header.fields & 0x2000) { stream.out_uint16_le(this->op.y + this->op.cy - 1); }

        header.emit_brush(stream, 0x4000, this->brush, oldcmd.brush);

        if (header.fields & 0x080000) { stream.out_uint16_le(this->glyph_x); }
        if (header.fields & 0x100000) { stream.out_uint16_le(this->glyph_y); }

        if (header.fields & 0x200000) {
            stream.out_uint8(this->data_len);
            stream.out_copy_bytes(this->data, this->data_len);
        }
    }

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header) {
        if (header.fields & 0x001) { this->cache_id       = stream.in_uint8(); }
        if (header.fields & 0x002) { this->fl_accel       = stream.in_uint8(); }
        if (header.fields & 0x004) { this->ui_charinc     = stream.in_uint8(); }
        if (header.fields & 0x008) { this->f_op_redundant = stream.in_uint8(); }

        if (header.fields & 0x010) {
            receive_rdp_color(stream, this->back_color);
        }

        if (header.fields & 0x020) {
            receive_rdp_color(stream, this->fore_color);
        }

        int16_t bk_left   = this->bk.x;
        int16_t bk_top    = this->bk.y;
        int16_t bk_right  = this->bk.x + this->bk.cx - 1;
        int16_t bk_bottom = this->bk.y + this->bk.cy - 1;

        if (header.fields & 0x0040) { bk_left   = stream.in_uint16_le(); }
        if (header.fields & 0x0080) { bk_top    = stream.in_uint16_le(); }
        if (header.fields & 0x0100) { bk_right  = stream.in_uint16_le(); }
        if (header.fields & 0x0200) { bk_bottom = stream.in_uint16_le(); }

        this->bk.x  = bk_left;
        this->bk.y  = bk_top;
        this->bk.cx = bk_right - this->bk.x + 1;
        this->bk.cy = bk_bottom - this->bk.y + 1;

        int16_t op_left   = this->op.x;
        int16_t op_top    = this->op.y;
        int16_t op_right  = this->op.x + this->op.cx - 1;
        int16_t op_bottom = this->op.y + this->op.cy - 1;

        if (header.fields & 0x0400) { op_left   = stream.in_uint16_le(); }
        if (header.fields & 0x0800) { op_top    = stream.in_uint16_le(); }
        if (header.fields & 0x1000) { op_right  = stream.in_uint16_le(); }
        if (header.fields & 0x2000) { op_bottom = stream.in_uint16_le(); }

        this->op.x  = op_left;
        this->op.y  = op_top;
        this->op.cx = op_right - this->op.x + 1;
        this->op.cy = op_bottom - this->op.y + 1;

        header.receive_brush(stream, 0x4000, this->brush);

        if (header.fields & 0x080000) { this->glyph_x = stream.in_uint16_le(); }
        if (header.fields & 0x100000) { this->glyph_y = stream.in_uint16_le(); }

        if (header.fields & 0x200000) {
            this->data_len = stream.in_uint8();
            stream.in_copy_bytes(this->data, this->data_len);
        }
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const {
        size_t lg = common.str(buffer, sz,
            !(!common.clip.contains(this->bk) || ((this->op.cx > 1) && !common.clip.contains(this->op))));
        lg += snprintf( buffer + lg, sz - lg
                      , "glyphIndex(cache_id=%.2x "
                        "fl_accel=%d ui_charinc=%d f_op_redundant=%d "
                        "back_color=%.6x fore_color=%.6x "
                        "bk=(%d,%d,%d,%d) "
                        "op=(%d,%d,%d,%d) "
                        "brush.(org_x=%d, org_y=%d, style=%d hatch=%d extra=[%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x]) "
                        "glyph_x=%d glyph_y=%d data_len=%d "
                      , unsigned(this->cache_id)
                      , this->fl_accel
                      , this->ui_charinc
                      , this->f_op_redundant
                      , this->back_color.as_bgr().to_u32()
                      , this->fore_color.as_bgr().to_u32()
                      , this->bk.x, this->bk.y, this->bk.cx, this->bk.cy
                      , this->op.x, this->op.y, this->op.cx, this->op.cy
                      , this->brush.org_x, this->brush.org_y, this->brush.style, this->brush.hatch
                      , unsigned(this->brush.extra[0]), unsigned(this->brush.extra[1])
                      , unsigned(this->brush.extra[2]), unsigned(this->brush.extra[3])
                      , unsigned(this->brush.extra[4]), unsigned(this->brush.extra[5])
                      , unsigned(this->brush.extra[6])
                      , this->glyph_x, this->glyph_y
                      , this->data_len
                      );
        lg += snprintf(buffer + lg, sz - lg, "[");
        for (int i = 0; i < this->data_len; i++) {
            if (i) {
                lg += snprintf( buffer + lg, sz - lg, " ");
            }
            lg += snprintf( buffer + lg, sz - lg, "0x%.2x", unsigned(this->data[i]));
        }
        lg += snprintf(buffer + lg, sz - lg, "]");
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
        this->bk = this->bk.offset(offset_x, offset_y);
        this->op = this->op.offset(offset_x, offset_y);
        this->glyph_x += offset_x;
        this->glyph_y += offset_y;
    }
};
