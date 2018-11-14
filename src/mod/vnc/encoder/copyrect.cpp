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

   Vnc encoder module for RAW bitmaps
*/

#include "mod/vnc/encoder/copyrect.hpp"

#include "core/buf64k.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/vnc/encoder/encoder_api.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/update_lock.hpp"

// 7.6.2   CopyRect Encoding
// =========================

// The CopyRect (copy rectangle) encoding is a very simple and efficient encoding which can be used when the client
// already has the same pixel data elsewhere in its framebuffer. The encoding on the wire simply consists of an X,Y
// coordinate. This gives a position in the framebuffer from which the client can copy the rectangle of pixel data.
// This can be used in a variety of situations, the most obvious of which are when the user moves a window across the
// screen, and when the contents of a window are scrolled. A less obvious use is for optimising drawing of text or other
// epeating patterns. An intelligent server may be able to send a pattern explicitly only once, and knowing the previous
// position of the pattern in the framebuffer, send subsequent occurrences of the same pattern using the CopyRect encoding.

// No. of bytes     Type    Description
//      2           U16     src-x-position
//      2           U16     src-y-position


namespace VNC
{
namespace Encoder
{
namespace
{
    struct CopyRect
    {
        const Rect rect;
        const uint16_t width;
        const uint16_t height;

        EncoderState operator()(Buf64k& buf, gdi::GraphicApi& drawable)
        {
            const size_t sz = 4;

            if (buf.remaining() < sz) { return EncoderState::NeedMoreData;  }

            InStream stream_copy_rect(buf.av(sz));
            uint16_t const srcx = stream_copy_rect.in_uint16_be();
            uint16_t const srcy = stream_copy_rect.in_uint16_be();

            buf.advance(sz);

            update_lock<gdi::GraphicApi> lock(drawable);
            drawable.draw(
                RDPScrBlt(this->rect, 0xCC, srcx, srcy),
                Rect(0, 0, this->width, this->height)
            );

            return EncoderState::Exit; // finished decoding
        }
    };
} // namespace

Encoder copy_rect_encoder(Rect rect, uint16_t width, uint16_t height)
{
    return Encoder(CopyRect{rect, width, height});
}

} // namespace Encoder
} // namespace VNC
