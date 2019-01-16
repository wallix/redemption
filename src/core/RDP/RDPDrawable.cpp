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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Poelen Jonathan, Raphael Zhou, Meng Tan
*/

#include <utility>

#include "core/RDP/RDPDrawable.hpp"

#include "utils/bitmap.hpp"

#include "core/font.hpp"

#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/rdp_pointer.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryCreateNinegridBitmap.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"


namespace
{
    using Color = Drawable::Color;

    Color u32_to_color(Drawable const& drawable, BGRColor color)
    {
        return drawable.u32bgr_to_color(color.to_u32());
    }

    Color u32rgb_to_color(Drawable const& drawable, gdi::ColorCtx color_ctx, RDPColor color)
    {
        return u32_to_color(drawable, color_decode(color, color_ctx));
    }

    std::pair<Color, Color> u32rgb_to_color(
        Drawable const& drawable, gdi::ColorCtx color_ctx, RDPColor color1, RDPColor color2,
        BGRPalette const& mod_palette_rgb)
    {
        using gdi::Depth;

        switch (color_ctx.depth()){
            case Depth::depth8():
                // TODO color_ctx.palette()
                return {
                    u32_to_color(drawable, decode_color8()(color1, mod_palette_rgb)),
                    u32_to_color(drawable, decode_color8()(color2, mod_palette_rgb))
                };
            case Depth::depth15():
                return {
                    u32_to_color(drawable, decode_color15()(color1)),
                    u32_to_color(drawable, decode_color15()(color2))
                };
            case Depth::depth16():
                return {
                    u32_to_color(drawable, decode_color16()(color1)),
                    u32_to_color(drawable, decode_color16()(color2))
                };
            case Depth::depth24():
                return {
                    u32_to_color(drawable, decode_color24()(color1)),
                    u32_to_color(drawable, decode_color24()(color2))
                };
            case Depth::unspecified(): default:;
        }

        assert(false);
        return {Color{0, 0, 0}, Color{0, 0, 0}};
    }

    Rect intersect(Drawable const& drawable, Rect const& a, Rect const& b)
    {
        return a.intersect(drawable.width(), drawable.height()).intersect(b);
    }

    // TODO removed when RDPMultiDstBlt and RDPMultiOpaqueRect contains a rect member
    //@{
    Rect to_rect(RDPMultiDstBlt const & cmd)
    { return Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight); }

    Rect to_rect(RDPMultiOpaqueRect const & cmd)
    { return Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight); }

    Rect to_rect(RDP::RDPMultiPatBlt const & cmd)
    { return cmd.rect; }

    Rect to_rect(RDP::RDPMultiScrBlt const & cmd)
    { return cmd.rect; }
    //@}

    template<class RDPMulti, class FRect>
    void draw_multi(Drawable const& drawable, const RDPMulti & cmd, Rect clip, FRect f)
    {
        const Rect clip_drawable_cmd_intersect = intersect(drawable, clip, to_rect(cmd));

        Rect cmd_rect;

        for (uint8_t i = 0; i < cmd.nDeltaEntries; i++) {
            cmd_rect.x  += cmd.deltaEncodedRectangles[i].leftDelta;
            cmd_rect.y  += cmd.deltaEncodedRectangles[i].topDelta;
            cmd_rect.cx =  cmd.deltaEncodedRectangles[i].width;
            cmd_rect.cy =  cmd.deltaEncodedRectangles[i].height;
            f(clip_drawable_cmd_intersect.intersect(cmd_rect));
        }
    }
} // namespace


RDPDrawable::RDPDrawable(const uint16_t width, const uint16_t height)
: drawable(width, height)
, mouse_cursor_pos_x(width / 2)
, mouse_cursor_pos_y(height / 2)
, mouse_cursor_hotspot_x(0)
, mouse_cursor_hotspot_y(0)
, dont_show_mouse_cursor(false)
, current_pointer(&this->default_pointer)
, frame_start_count(0)
, mod_palette_rgb(BGRPalette::classic_332())
{
    Pointer p = drawable_default_pointer(true);
    auto av     = p.get_24bits_xor_mask();
    auto avmask = p.get_monochrome_and_mask();
    this->default_pointer.initialize(32, 32, av.data(), avmask.data());
}


void RDPDrawable::draw(RDPColCache   const & /*cmd*/)
{}

void RDPDrawable::draw(RDPBrushCache const & /*cmd*/)
{}

void RDPDrawable::draw(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    const Rect trect = intersect(this->drawable, clip, cmd.rect);
    this->drawable.opaquerect(trect, u32rgb_to_color(this->drawable, color_ctx, cmd.color));
    this->last_update_index++;
}

void RDPDrawable::draw(RDPEllipseSC const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // TODO clip is not used
    (void)clip;
    this->drawable.ellipse(cmd.el, cmd.bRop2, cmd.fillMode, u32rgb_to_color(this->drawable, color_ctx, cmd.color));
    this->last_update_index++;
}

// TODO This will draw a standard ellipse without brush style
void RDPDrawable::draw(RDPEllipseCB const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // TODO clip is not used
    (void)clip;
    this->drawable.ellipse(cmd.el, cmd.brop2, cmd.fill_mode, u32rgb_to_color(this->drawable, color_ctx, cmd.back_color));
    this->last_update_index++;
}

void RDPDrawable::draw(const RDPScrBlt & cmd, Rect clip)
{
    // Destination rectangle : drect
    const Rect drect = intersect(this->drawable, clip, cmd.rect);
    if (drect.isempty()){ return; }
    // adding delta move dest to source
    const signed int deltax = cmd.srcx - cmd.rect.x;
    const signed int deltay = cmd.srcy - cmd.rect.y;
    this->drawable.scrblt(drect.x + deltax, drect.y + deltay, drect, cmd.rop);
    this->last_update_index++;
}

void RDPDrawable::draw(const RDPDestBlt & cmd, Rect clip)
{
    const Rect trect = intersect(this->drawable, clip, cmd.rect);
    this->drawable.destblt(trect, cmd.rop);
    this->last_update_index++;
}

void RDPDrawable::draw(RDPNineGrid const &  /*cmd*/, Rect  /*rect*/, gdi::ColorCtx  /*color_ctx*/, Bitmap const &  /*bmp*/)
{}

void RDPDrawable::draw(const RDPMultiDstBlt & cmd, Rect clip)
{
    draw_multi(this->drawable, cmd, clip, [&](const Rect & trect) {
        this->drawable.destblt(trect, cmd.bRop);
    });
    this->last_update_index++;
}

void RDPDrawable::draw(RDPMultiOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    const Color color = u32rgb_to_color(this->drawable, color_ctx, cmd._Color);
    draw_multi(this->drawable, cmd, clip, [color, this](const Rect & trect) {
        this->drawable.opaquerect(trect, color);
    });
    this->last_update_index++;
}

void RDPDrawable::draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // TODO PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way  with stripes  etc.) and also there is quite a lot of possible ternary operators  and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then  work on correct computation of pattern and fix it.
    if (cmd.brush.style == 0x03 && (cmd.bRop == 0xF0 || cmd.bRop == 0x5A)) {
        enum { BackColor, ForeColor };
        auto colors = u32rgb_to_color(this->drawable, color_ctx, cmd.BackColor, cmd.ForeColor, this->mod_palette_rgb);
        uint8_t brush_data[8];
        memcpy(brush_data, cmd.brush.extra, 7);
        brush_data[7] = cmd.brush.hatch;
        draw_multi(this->drawable, cmd, clip, [&](const Rect & trect) {
            this->drawable.patblt_ex(
                trect, cmd.bRop,
                std::get<BackColor>(colors), std::get<ForeColor>(colors),
                brush_data, cmd.brush.org_x, cmd.brush.org_y
            );
        });
    }
    else {
        const Color color = u32rgb_to_color(this->drawable, color_ctx, cmd.BackColor);
        draw_multi(this->drawable, cmd, clip, [&](const Rect & trect) {
            this->drawable.patblt(trect, cmd.bRop, color);
        });
    }
    this->last_update_index++;
}

void RDPDrawable::draw(const RDP::RDPMultiScrBlt & cmd, Rect clip)
{
    const signed int deltax = cmd.nXSrc - cmd.rect.x;
    const signed int deltay = cmd.nYSrc - cmd.rect.y;
    draw_multi(this->drawable, cmd, clip, [&](const Rect & trect) {
        this->drawable.scrblt(trect.x + deltax, trect.y + deltay, trect, cmd.bRop);
    });
    this->last_update_index++;
}

void RDPDrawable::draw(RDPPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    const Rect trect = intersect(this->drawable, clip, cmd.rect);
    // TODO PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way  with stripes  etc.) and also there is quite a lot of possible ternary operators  and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then  work on correct computation of pattern and fix it.

    if (cmd.brush.style == 0x03 && (cmd.rop == 0xF0 || cmd.rop == 0x5A)) {
        enum { BackColor, ForeColor };
        auto colors = u32rgb_to_color(this->drawable, color_ctx, cmd.back_color, cmd.fore_color, this->mod_palette_rgb);
        uint8_t brush_data[8];
        memcpy(brush_data, cmd.brush.extra, 7);
        brush_data[7] = cmd.brush.hatch;

        this->drawable.patblt_ex(
            trect, cmd.rop,
            std::get<BackColor>(colors), std::get<ForeColor>(colors),
            brush_data, cmd.brush.org_x, cmd.brush.org_y
        );
    }
    else {
        this->drawable.patblt(trect, cmd.rop, u32rgb_to_color(this->drawable, color_ctx, cmd.back_color));
    }
    this->last_update_index++;
}

void RDPDrawable::draw(const RDPMemBlt & cmd_, Rect clip, const Bitmap & bmp)
{
    RDPMemBlt cmd(cmd_);

    cmd.rect = cmd_.rect.intersect(this->drawable.width(), this->drawable.height());
    cmd.srcx += (cmd.rect.x - cmd_.rect.x);
    cmd.srcy += (cmd.rect.y - cmd_.rect.y);

    const Rect rect = clip.intersect(cmd.rect);
    if (rect.isempty()){
        return ;
    }

    switch (cmd.rop) {
    case 0x00:
        this->drawable.black_color(rect);
    break;
    case 0xFF:
        this->drawable.white_color(rect);
    break;
    case 0x55:
        this->drawable.mem_blt_invert(rect, bmp
            , cmd.srcx + (rect.x - cmd.rect.x)
            , cmd.srcy + (rect.y - cmd.rect.y));
    break;
    case 0xCC:
        this->drawable.mem_blt(rect, bmp
            , cmd.srcx + (rect.x - cmd.rect.x)
            , cmd.srcy + (rect.y - cmd.rect.y));
    break;
    case 0x22:  // dest = dest AND (NOT source)
    case 0x66:  // dest = source XOR dest (SRCINVERT)
    case 0x88:  // dest = source AND dest (SRCAND)
    case 0xBB:  // dest = (NOT source) OR dest (MERGEPAINT)
    case 0xEE:  // dest = source OR dest (SRCPAINT)
        this->drawable.mem_blt_ex(rect, bmp
            , cmd.srcx + (rect.x - cmd.rect.x)
            , cmd.srcy + (rect.y - cmd.rect.y)
            , cmd.rop);
        break;
    default:
        // should not happen
        //LOG(LOG_INFO, "Unsupported Rop=0x%02X", cmd.rop);
    break;
    }
    this->last_update_index++;
}

void RDPDrawable::draw(RDPMem3Blt const & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bmp)
{
    const Rect rect = clip.intersect(cmd.rect);
    if (rect.isempty()){
        return ;
    }

    this->drawable.mem_3_blt(rect, bmp
        , cmd.srcx + (rect.x  - cmd.rect.x)
        , cmd.srcy + (rect.y  - cmd.rect.y)
        , cmd.rop
        , u32rgb_to_color(this->drawable, color_ctx, cmd.fore_color)
    );
    this->last_update_index++;
}

/*
    *
    *            +----+----+
    *            |\   |   /|  4 cases.
    *            | \  |  / |  > Case 1 is the normal case
    *            |  \ | /  |  > Case 2 has a negative coeff
    *            | 3 \|/ 2 |  > Case 3 and 4 are the same as
    *            +----0---->x    Case 1 and 2 but one needs to
    *            | 4 /|\ 1 |     exchange begin and end.
    *            |  / | \  |
    *            | /  |  \ |
    *            |/   |   \|
    *            +----v----+
    *                 y
    *  Anyway, we base the line drawing on bresenham's algorithm
    */
void RDPDrawable::draw(const RDPLineTo & lineto, Rect clip, gdi::ColorCtx color_ctx)
{
    this->drawable.draw_line(
        lineto.back_mode,
        lineto.startx, lineto.starty,
        lineto.endx, lineto.endy,
        lineto.rop2, u32rgb_to_color(this->drawable, color_ctx, lineto.pen.color), clip
    );
    this->last_update_index++;
}

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

namespace
{
    void draw_glyph(Drawable& drawable, FontChar const & fc, int16_t px, int16_t py, Color fg_color, Rect clip)
    {
        const uint8_t * fc_data            = fc.data.get();
        for (int yy = 0 ; yy < fc.height; yy++)
        {
            uint8_t   fc_bit_mask        = 128;
            for (int xx = 0 ; xx < fc.width; xx++)
            {
                if (!fc_bit_mask)
                {
                    fc_data++;
                    fc_bit_mask = 128;
                }
                if (clip.contains_pt(px + xx, py + yy)
                && (fc_bit_mask & *fc_data))
                {
                    drawable.draw_pixel(px + xx, py + yy, fg_color);
                }
                fc_bit_mask >>= 1;
            }
            fc_data++;
        }
    }

    void draw_VariableBytes(
        Drawable& drawable,
        uint8_t (&fragment_cache)[MAXIMUM_NUMBER_OF_FRAGMENT_CACHE_ENTRIES][1 /* size */ + MAXIMUM_SIZE_OF_FRAGMENT_CACHE_ENTRIE],
        uint8_t const * data, uint16_t size,
        bool has_delta_bytes, uint16_t ui_charinc,
        uint16_t & draw_pos_ref, int16_t offset_y, Color color,
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
                    LOG( LOG_WARNING
                        , "RDPDrawable::draw_VariableBytes: Unknown glyph, cacheId=%u cacheIndex=%u"
                        , cache_id, data);
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
                        draw_glyph(drawable, fc, x + fc.offsetx, y + fc.offsety, color, clip);
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
                    "RDPDrawable::draw_VariableBytes: "
                        "Experimental support of USE (0xFE) operation byte in "
                        "GlyphIndex Primary Drawing Order. "
                        "fragment_index=%u fragment_size=%u delta=%u",
                    fragment_index, fragment_cache[fragment_index][0], delta);

                fragment_begin_position = variable_bytes.get_current();

                if (ui_charinc) {
                    draw_pos_ref += ui_charinc;
                }

                draw_VariableBytes(drawable, fragment_cache, &fragment_cache[fragment_index][1],
                    fragment_cache[fragment_index][0], has_delta_bytes, ui_charinc,
                    draw_pos_ref, offset_y, color, bmp_pos_x, bmp_pos_y, clip,
                    cache_id, gly_cache);
            }
            else if (data == 0xFF)
            {
                const uint8_t fragment_index = variable_bytes.in_uint8();
                const uint8_t fragment_size  = variable_bytes.in_uint8();

                LOG(LOG_WARNING,
                    "RDPDrawable::draw_VariableBytes: "
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
} // namespace

void RDPDrawable::draw(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache)
{
    Rect screen_rect = clip.intersect(this->drawable.width(), this->drawable.height());
    if (screen_rect.isempty()){
        return ;
    }

    Rect const clipped_glyph_fragment_rect = cmd.bk.intersect(screen_rect);
    if (clipped_glyph_fragment_rect.isempty()) {
        return;
    }

    // set a background color
    {
        Rect ajusted = cmd.f_op_redundant ? cmd.bk : cmd.op;
        if ((ajusted.cx > 1) && (ajusted.cy > 1)) {
            ajusted.cy--;
            this->drawable.opaquerect(ajusted.intersect(screen_rect), u32rgb_to_color(this->drawable, color_ctx, cmd.fore_color));
        }
    }

    bool has_delta_bytes = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));
    const Color color = u32rgb_to_color(this->drawable, color_ctx, cmd.back_color);
    const int16_t offset_y = /*cmd.bk.cy - (*/cmd.glyph_y - cmd.bk.y/* + 1)*/;
    const int16_t offset_x = cmd.glyph_x - cmd.bk.x;

    uint16_t draw_pos = 0;

    draw_VariableBytes(
        this->drawable, this->fragment_cache, cmd.data,
        cmd.data_len, has_delta_bytes, cmd.ui_charinc,
        draw_pos, offset_y, color, cmd.bk.x + offset_x, cmd.bk.y,
        clipped_glyph_fragment_rect, cmd.cache_id, gly_cache);
    this->last_update_index++;
}

void RDPDrawable::draw(RDPPolyline const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    int16_t startx = cmd.xStart;
    int16_t starty = cmd.yStart;

    const Color color = u32rgb_to_color(this->drawable, color_ctx, cmd.PenColor);

    for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
        int16_t const endx = startx + cmd.deltaEncodedPoints[i].xDelta;
        int16_t const endy = starty + cmd.deltaEncodedPoints[i].yDelta;

        this->drawable.draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, color, clip);

        startx = endx;
        starty = endy;
    }
    this->last_update_index++;
}

// TODO this functions only draw polygon borders but do not fill them with solid color.
void RDPDrawable::draw(RDPPolygonSC const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    int16_t startx = cmd.xStart;
    int16_t starty = cmd.yStart;

    int16_t endx;
    int16_t endy;

    const Color BrushColor = u32rgb_to_color(this->drawable, color_ctx, cmd.BrushColor);

    for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
        endx = startx + cmd.deltaPoints[i].xDelta;
        endy = starty + cmd.deltaPoints[i].yDelta;

        this->drawable.draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, BrushColor, clip);

        startx = endx;
        starty = endy;
    }
    endx = cmd.xStart;
    endy = cmd.yStart;

    this->drawable.draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, BrushColor, clip);
    this->last_update_index++;
}

// TODO this functions only draw polygon borders but do not fill them with brush color.
void RDPDrawable::draw(RDPPolygonCB const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    int16_t startx = cmd.xStart;
    int16_t starty = cmd.yStart;

    int16_t endx;
    int16_t endy;

    const Color foreColor = u32rgb_to_color(this->drawable, color_ctx, cmd.foreColor);

    for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
        endx = startx + cmd.deltaPoints[i].xDelta;
        endy = starty + cmd.deltaPoints[i].yDelta;

        this->drawable.draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, foreColor, clip);

        startx = endx;
        starty = endy;
    }
    endx = cmd.xStart;
    endy = cmd.yStart;

    this->drawable.draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, foreColor, clip);
    this->last_update_index++;
}

void RDPDrawable::draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp)
{
    const Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                      , bitmap_data.dest_right - bitmap_data.dest_left + 1
                      , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

    const Rect trect = rectBmp.intersect(this->drawable.width(), this->drawable.height());

    this->drawable.draw_bitmap(trect, bmp);
    this->last_update_index++;
}

void RDPDrawable::draw(const RDP::FrameMarker & order)
{
    this->frame_start_count += ((order.action == RDP::FrameMarker::FrameStart) ? 1 : -1);
    assert(this->frame_start_count >= 0);
    this->drawable.logical_frame_ended = (this->frame_start_count == 0);
    this->last_update_index++;
}

void RDPDrawable::trace_mouse()
{
    if (this->dont_show_mouse_cursor || !this->current_pointer) {
        return;
    }
    this->save_mouse_x = this->mouse_cursor_pos_x;
    this->save_mouse_y = this->mouse_cursor_pos_y;
    int x = this->save_mouse_x - this->mouse_cursor_hotspot_x;
    int y = this->save_mouse_y - this->mouse_cursor_hotspot_y;
    return this->drawable.trace_mouse(this->current_pointer, x, y, this->save_mouse);
}

void RDPDrawable::clear_mouse()
{
    if (this->dont_show_mouse_cursor || !this->current_pointer) {
        return;
    }
    int x = this->save_mouse_x - this->mouse_cursor_hotspot_x;
    int y = this->save_mouse_y - this->mouse_cursor_hotspot_y;
    return this->drawable.clear_mouse(this->current_pointer, x, y, this->save_mouse);
}

void RDPDrawable::set_pointer(const Pointer & cursor)
{
    const auto dimensions = cursor.get_dimensions();
    const auto hotspot = cursor.get_hotspot();
    auto av_xor = cursor.get_24bits_xor_mask();
    auto av_and = cursor.get_monochrome_and_mask();

    this->dynamic_pointer.initialize(dimensions.width, dimensions.height, av_xor.data(), av_and.data());
    this->mouse_cursor_hotspot_x = hotspot.x;
    this->mouse_cursor_hotspot_y = hotspot.y;
    this->current_pointer = &this->dynamic_pointer ;
}
