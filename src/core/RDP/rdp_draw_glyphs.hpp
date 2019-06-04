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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

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
// +-----------------------------------------------+---------------+
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

#include "core/font.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "utils/rect.hpp"

#include <cassert>


namespace detail
{
namespace
{
    template<class SetPoint>
    void rdp_draw_glyph(SetPoint& set_point, FontChar const & fc, int16_t px, int16_t py, Rect clip)
    {
        const uint8_t * fc_data = fc.data.get();
        for (int yy = 0; yy < fc.height; yy++)
        {
            uint8_t fc_bit_mask = 128;
            for (int xx = 0; xx < fc.width; xx++)
            {
                if (!fc_bit_mask)
                {
                    fc_data++;
                    fc_bit_mask = 128;
                }
                if (clip.contains_pt(px + xx, py + yy)
                && (fc_bit_mask & *fc_data))
                {
                    set_point(px + xx, py + yy);
                }
                fc_bit_mask >>= 1;
            }
            fc_data++;
        }
    }
}
}

template<class SetPoint>
void rdp_draw_glyphs(
    SetPoint&& set_point,
    uint8_t (&fragment_cache)[MAXIMUM_NUMBER_OF_FRAGMENT_CACHE_ENTRIES][1 /* size */ + MAXIMUM_SIZE_OF_FRAGMENT_CACHE_ENTRIE],
    uint8_t const * data, uint16_t size,
    bool has_delta_bytes, uint16_t ui_charinc,
    uint16_t & draw_pos_ref, int16_t offset_y,
    int16_t bmp_pos_x, int16_t bmp_pos_y, Rect clip,
    uint8_t cache_id, const GlyphCache & gly_cache)
{
    InStream variable_bytes(data, size);

    uint8_t const * fragment_begin_position = variable_bytes.get_current();

    while (variable_bytes.in_remain())
    {
        uint8_t data = variable_bytes.in_uint8();
        if (data <= 0xFD)
        {
            FontChar const & fc = gly_cache.glyphs[cache_id][data].font_item;
            if (!fc)
            {
                LOG( LOG_WARNING,
                    "RDPDrawable::rdp_draw_glyphs: Unknown glyph, cacheId=%u cacheIndex=%u",
                    cache_id, data);
                assert(fc);
            }

            if (has_delta_bytes)
            {
                data = variable_bytes.in_uint8();
                if (data == 0x80)
                {
                    draw_pos_ref += variable_bytes.in_uint16_le();
                }
                else
                {
                    draw_pos_ref += data;
                }
            }

            if (fc)
            {
                const int16_t x = draw_pos_ref + bmp_pos_x;
                const int16_t y = offset_y + bmp_pos_y;
                if (Rect(0,0,0,0) != clip.intersect(Rect(x, y, fc.incby, fc.height))){
                    detail::rdp_draw_glyph(set_point, fc, x + fc.offsetx, y + fc.offsety, clip);
                }
            }

            if (ui_charinc) {
                draw_pos_ref += ui_charinc;
            }
        }
        else if (data == 0xFE)
        {
            const uint8_t fragment_index = variable_bytes.in_uint8();

            uint16_t delta = 0;
            if (has_delta_bytes)
            {
                delta = variable_bytes.in_uint8();
                if (delta == 0x80)
                {
                    delta = variable_bytes.in_uint16_le();
                }
            }
            assert(!delta);  // Fragment's position delta is not yet supported.

            LOG(LOG_WARNING,
                "RDPDrawable::rdp_draw_glyphs: "
                    "Experimental support of USE (0xFE) operation byte in "
                    "GlyphIndex Primary Drawing Order. "
                    "fragment_index=%u fragment_size=%u delta=%u",
                fragment_index, fragment_cache[fragment_index][0], delta);

            fragment_begin_position = variable_bytes.get_current();

            if (ui_charinc) {
                draw_pos_ref += ui_charinc;
            }

            rdp_draw_glyphs(set_point, fragment_cache, &fragment_cache[fragment_index][1],
                fragment_cache[fragment_index][0], has_delta_bytes, ui_charinc,
                draw_pos_ref, offset_y, bmp_pos_x, bmp_pos_y, clip,
                cache_id, gly_cache);
        }
        else if (data == 0xFF)
        {
            const uint8_t fragment_index = variable_bytes.in_uint8();
            const uint8_t fragment_size  = variable_bytes.in_uint8();

            LOG(LOG_WARNING,
                "RDPDrawable::rdp_draw_glyphs: "
                    "Experimental support of ADD (0xFF) operation byte in "
                    "GlyphIndex Primary Drawing Order. "
                    "fragment_index=%u fragment_size=%u",
                fragment_index, fragment_size);

            assert(!variable_bytes.in_remain());

            assert(fragment_begin_position + fragment_size + 3 == variable_bytes.get_current());

            fragment_cache[fragment_index][0] = fragment_size;
            ::memcpy(&fragment_cache[fragment_index][1],
                        fragment_begin_position, fragment_size);

            fragment_begin_position = variable_bytes.get_current();
        }
    }
}
