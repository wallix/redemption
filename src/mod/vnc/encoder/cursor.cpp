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

Vnc encoder module for Cursor Pseudo Encoding
*/

#include "mod/vnc/encoder/cursor.hpp"

#include "core/RDP/rdp_pointer.hpp"
#include "core/buf64k.hpp"
#include "mod/vnc/vnc_verbose.hpp"
#include "mod/vnc/encoder/pointer_loader_vnc.hpp"
#include "utils/log.hpp"
#include "utils/hexdump.hpp"
#include "gdi/graphic_api.hpp"


// 7.7.2   Cursor Pseudo-encoding
// ------------------------------

// A client which requests the Cursor pseudo-encoding is
// declaring that it is capable of drawing a mouse cursor
// locally. This can significantly improve perceived performance
// over slow links.

// The server sets the cursor shape by sending a pseudo-rectangle
// with the Cursor pseudo-encoding as part of an update.

// x, y : The pseudo-rectangle's x-position and y-position
// indicate the hotspot of the cursor,

// cx, cy : width and height indicate the width and height of
// the cursor in pixels.

// The data consists of width * height pixel values followed by
// a bitmask.

// PIXEL array : width * height * bytesPerPixel
// bitmask     : floor((width + 7) / 8) * height

// The bitmask consists of left-to-right, top-to-bottom
// scanlines, where each scanline is padded to a whole number of
// bytes. Within each byte the most significant bit represents
// the leftmost pixel, with a 1-bit meaning the corresponding
// pixel in the cursor is valid.

namespace VNC
{
namespace Encoder
{
namespace
{

struct Cursor
{
    const BytesPerPixel Bpp;
    const Rect rect;
    uint16_t red_max;
    uint16_t green_max;
    uint16_t blue_max;
    uint8_t red_shift;
    uint8_t green_shift;
    uint8_t blue_shift;
    VNCVerbose verbose;

    EncoderState operator()(Buf64k & buf, gdi::GraphicApi & gd)
    {
        if (this->rect.isempty()) {
            // TODO: empty Pointer: no cursor data to read. Should we set an invisible pointer ? If so we should have some flag to configure it
            return EncoderState::Exit;
        }

        const size_t sz_pixel_array = this->rect.cx * this->rect.cy * underlying_cast(this->Bpp);
        const size_t sz_bitmask = ::nbbytes(this->rect.cx) * this->rect.cy;

        if (sz_pixel_array + sz_bitmask > 65536) {
            // TODO: as cursor size is not limited by VNC protocol, this could actually happen
            // we should really copy cursor data into local cursor buffer whenever it's incoming
            // and consume buffer data. But it's a small matter as such large pointers are never
            // actually happening.
            LOG(LOG_ERR,
                "VNC Encoding: Cursor, data buffer too small (65536 < %zu)",
                sz_pixel_array + sz_bitmask);
            throw Error(ERR_BUFFER_TOO_SMALL);
        }

        if (buf.remaining() < sz_pixel_array + sz_bitmask) {
            return EncoderState::NeedMoreData;
        }

        const auto cursor_buf = buf.av(sz_pixel_array + sz_bitmask);
        const auto data = cursor_buf.first(sz_pixel_array);
        const auto mask = cursor_buf.drop_front(sz_pixel_array);
        buf.advance(sz_pixel_array + sz_bitmask);

        // TODO: special dot cursor  if cx=1 cy=1 ? : a VNC pointer of 1x1 size is not visible, so a default minimal pointer (dot pointer) is provided instead ?
        // Pointer cursor(this->bpp, Pointer::CursorSize{this->cx, this->cy}, Hotspot{this->x, this->y}, this->mask, this->data, false);

        if (bool(this->verbose & VNCVerbose::cursor_encoder)) {
            LOG(LOG_DEBUG, "VNC Cursor(%hd, %hd, %hu, %hu) %u %zu",
                this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
                this->Bpp, sz_pixel_array);
            hexdump_d(data);
            hexdump_d(mask);
        }

        PointerLoaderVnc pointer_loader_vnc;
        RdpPointerView cursor = pointer_loader_vnc.load(
            this->Bpp, this->rect.cx, this->rect.cy, this->rect.x, this->rect.y,
            data, mask,
            this->red_shift, this->red_max,
            this->green_shift, this->green_max,
            this->blue_shift, this->blue_max);
        // TODO use a local cache and a local cache_idx
        gd.new_pointer(0, cursor);
        gd.cached_pointer(0);

        return EncoderState::Exit;
    }
};

} // namespace

Encoder cursor_encoder(
    BytesPerPixel Bpp, Rect rect,
    uint8_t red_shift, uint16_t red_max,
    uint8_t green_shift, uint16_t green_max,
    uint8_t blue_shift, uint16_t blue_max,
    VNCVerbose verbose)
{
    return Encoder(Cursor{
        Bpp, rect,
        red_max, green_max, blue_max,
        red_shift, green_shift, blue_shift,
        verbose
    });
}

} // namespace Encoder
} // namespace VNC
