/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.
h
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2018
   Author(s): Christophe Grosjean

   Vnc encoder module for RRE Encoded Rectangles
*/

#include "mod/vnc/encoder/rre.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/buf64k.hpp"
#include "gdi/graphic_api.hpp"
#include "utils/bitmap.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/sugar/update_lock.hpp"

#include <vector>


//  7.6.3   RRE Encoding
//  ====================

//    RRE stands for rise-and-run-length encoding and as its name implies, it is essentially a two-dimensional analogue
//    of run-length encoding. RRE-encoded rectangles arrive at the client in a form which can be rendered immediately and
//    efficiently by the simplest of graphics engines. RRE is not appropriate for complex desktops, but can be useful in
//    some situations.

//    The basic idea behind RRE is the partitioning of a rectangle of pixel data into rectangular subregions (subrectangles)
//    each of which consists of pixels of a single value and the union of which comprises the original rectangular region.
//    The near-optimal partition of a given rectangle into such subrectangles is relatively easy to compute.

//    The encoding consists of a background pixel value, Vb (typically the most prevalent pixel value in the rectangle)
//    and a count N, followed by a list of N subrectangles, each of which consists of a tuple <v, x, y, w, h> where v (!= Vb)
//    is the pixel value, (x, y) are the coordinates of the subrectangle relative to the top-left corner of the rectangle,
//    and (w, h) are the width and height of the subrectangle. The client can render the original rectangle by drawing a filled
//    rectangle of the background pixel value and then drawing a filled rectangle corresponding to each subrectangle.

//    On the wire, the data begins with the header:

//    No. of bytes      Type            Description
//         4             U32         number-of-subrectangles
//    bytesPerPixel     PIXEL       background-pixel-value

//    This is followed by number-of-subrectangles instances of the following structure:

//    No. of bytes     Type         Description
//    bytesPerPixel     PIXEL       subrect-pixel-value
//          2           U16         x-position
//          2           U16         y-position
//          2           U16         width
//          2           U16         height

namespace VNC
{
namespace Encoder
{
namespace
{
    // TODO: Check that encoder, it looks bad, working ?
    class RRE
    {
        const BitsPerPixel bpp;
        const uint8_t Bpp;
        Rect rect;

        enum class RREState {
            Header,
            Data,
            Exit
        } state;

        uint32_t number_of_subrectangles_remain;
        // this could be a fest creation of a monochrome bitmap
        // or we could also send a drawing order for background color:
        // easy and fast and use very few memory,
        // then sending data for colored rectangles on the go
        std::vector<uint8_t> rre_raw;

    public:
        RRE(BitsPerPixel bpp, BytesPerPixel Bpp, Rect rect)
            : bpp(bpp), Bpp(safe_int(Bpp)), rect(rect)
            , state(RREState::Header)
        {
            this->rre_raw.reserve(rect.cx * rect.cy * this->Bpp);
        }

        EncoderState operator()(Buf64k & buf, gdi::GraphicApi & drawable)
        {
            switch (this->state){
            case RREState::Header:
            {
                const size_t sz = 4 + this->Bpp;

                if (buf.remaining() < sz){ return EncoderState::NeedMoreData; }

                // TODO: fix that, no need to perform double copy!
                // we should be able to read number of subrect only
                // then pursue treatment the way it is done in Data
                InStream stream_rre(buf.av(sz));
                this->number_of_subrectangles_remain = stream_rre.in_uint32_be();

                uint8_t * point_cur = this->rre_raw.data();
                uint8_t * point_end = point_cur + this->rect.cx * this->rect.cy * Bpp;
                for (; point_cur < point_end ; point_cur += Bpp) {
                    memcpy(point_cur, stream_rre.get_current(), Bpp);
                }
                buf.advance(sz);

                this->state = RREState::Data;
                return EncoderState::Ready;
            }
            case RREState::Data:
            {
                if (!this->number_of_subrectangles_remain) {
                    // TODO: why are we sending the tiles that way
                    // TODO: using MultiDestBlt should be better
                    // TODO use MultiRect
                    update_lock<gdi::GraphicApi> lock(drawable);
                    this->draw_tile(rect, this->rre_raw.data(), drawable);
                    this->rre_raw.clear();
                    this->state = RREState::Header;
                    return EncoderState::Exit;
                }

                const size_t sz = 8 + this->Bpp;

                if (buf.remaining() < sz){ return EncoderState::NeedMoreData;}

                --this->number_of_subrectangles_remain;

                InStream subrectangles(buf.av(sz));
                auto bytes_per_pixel = subrectangles.get_current();
                subrectangles.in_skip_bytes(Bpp);
                auto subrec_x        = subrectangles.in_uint16_be();
                auto subrec_y        = subrectangles.in_uint16_be();
                auto subrec_width    = subrectangles.in_uint16_be();
                auto subrec_height   = subrectangles.in_uint16_be();

                auto ling_boundary = this->rect.cx * Bpp;
                auto point_line_cur = this->rre_raw.data() + subrec_y * ling_boundary;
                auto point_line_end = point_line_cur + subrec_height * ling_boundary;
                for (; point_line_cur < point_line_end; point_line_cur += ling_boundary) {
                    uint8_t * point_cur = point_line_cur + subrec_x * Bpp;
                    uint8_t * point_end = point_cur + subrec_width * Bpp;
                    for (; point_cur < point_end; point_cur += Bpp) {
                        memcpy(point_cur, bytes_per_pixel, Bpp);
                    }
                }

                buf.advance(sz);
                return EncoderState::Ready;
            }
            case RREState::Exit:
                break;
            }
            return EncoderState::Exit; // finished decoding
        }

    private:
        void draw_tile(Rect rect, const uint8_t * raw, gdi::GraphicApi & drawable)
        {
            const uint16_t TILE_CX = 32;
            const uint16_t TILE_CY = 32;

            for (int y = 0; y < rect.cy ; y += TILE_CY) {
                uint16_t cy = std::min(TILE_CY, uint16_t(rect.cy - y));

                for (int x = 0; x < rect.cx ; x += TILE_CX) {
                    uint16_t cx = std::min(TILE_CX, uint16_t(rect.cx - x));

                    const Rect src_tile(x, y, cx, cy);
                    const Bitmap tiled_bmp(raw, rect.cx, rect.cy, this->bpp, src_tile);
                    const Rect dst_tile(rect.x + x, rect.y + y, cx, cy);
                    const RDPMemBlt cmd2(0, dst_tile, 0xCC, 0, 0, 0);
                    drawable.draw(cmd2, dst_tile, tiled_bmp);
                }
            }
        }
    };
} // namespace

Encoder rre_encoder(BitsPerPixel bpp, BytesPerPixel Bpp, Rect rect)
{
    return Encoder{RRE(bpp, Bpp, rect)};
}

} // namespace Encoder
} // namespace VNC
