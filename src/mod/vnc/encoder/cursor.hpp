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

#pragma once

#include "utils/log.hpp"
#include "utils/verbose_flags.hpp"
#include "mod/vnc/vnc_verbose.hpp"

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

namespace VNC {
    namespace Encoder {
        class Cursor : public EncoderApi {
            const uint8_t bpp;
            const uint8_t Bpp;
            const size_t x;
            size_t y;
            const size_t cx;
            size_t cy;
            int red_shift;
            int red_max;
            int green_shift;
            int green_max;
            int blue_shift;
            int blue_max;

        public:
            VNCVerbose verbose;

            Cursor(uint8_t bpp, uint8_t Bpp, size_t x, size_t y, size_t cx, size_t cy,
                   int red_shift, int red_max, int green_shift, int green_max, int blue_shift, int blue_max, 
                   VNCVerbose verbose) 
                : bpp(bpp), Bpp(Bpp), x(x), y(y), cx(cx), cy(cy),
                  red_shift(red_shift), red_max(red_max), 
                  green_shift(green_shift), green_max(green_max), 
                  blue_shift(blue_shift), blue_max(blue_max)
                , verbose(verbose)
            {
            }
            
            virtual ~Cursor(){}
            
            // return is true if the Encoder has finished working (can be reset or deleted),
            // return is false if the encoder is waiting for more data
            EncoderState consume(Buf64k & buf, gdi::GraphicApi & drawable) override
            {
                // TODO see why we get these empty rects ?
                if (this->cx <= 0 && this->cy <= 0) {
                    LOG(LOG_INFO, "empty rect %s", Rect(this->x, this->y, this->cx, this->cy));
                }
                if (this->cx == 0 || this->cy == 0) {
                    // TODO: empty Pointer: no cursor data to read. Should we set an invisible pointer ? If so we should have some flag to configure it
                    return EncoderState::Exit;
                }

                const int sz_pixel_array = this->cx * this->cy * this->Bpp;
                const int sz_bitmask = nbbytes(this->cx) * this->cy;

                if (sz_pixel_array + sz_bitmask > 65536)
                {
                    // TODO: as cursor size is not limited by VNC protocol, this could actually happen
                    // we should really copy cursor data into local cursor buffer whenever it's incoming
                    // and consume buffer data. But it's a small matter as such large pointers are never
                    // actually happening.
                    LOG(LOG_ERR,
                        "VNC Encoding: Cursor, data buffer too small (65536 < %d)",
                        sz_pixel_array + sz_bitmask);
                    throw Error(ERR_BUFFER_TOO_SMALL);
                }

                if (buf.remaining() < sz_pixel_array + sz_bitmask)
                {
                    return EncoderState::NeedMoreData; 
                }

                auto cursor_buf = buf.av(sz_pixel_array + sz_bitmask).data();
                std::vector<uint8_t> data(cursor_buf, cursor_buf + sz_pixel_array);
                std::vector<uint8_t> mask(cursor_buf + sz_pixel_array, cursor_buf + sz_pixel_array + sz_bitmask);
                buf.advance(sz_pixel_array + sz_bitmask);

                // TODO: special dot cursor  if cx=1 cy=1 ? : a VNC pointer of 1x1 size is not visible, so a default minimal pointer (dot pointer) is provided instead ?
//                Pointer cursor(this->bpp, Pointer::CursorSize{this->cx, this->cy}, Pointer::Hotspot{this->x, this->y}, {this->mask.data(), this->mask.size()}, {this->data.data(), this->data.size()}, false);

                LOG(LOG_INFO, "VNC Cursor(%u, %u, %u, %u) %u %u %u", this->x, this->y, this->cx, this->cy, this->Bpp, this->bpp, sz_pixel_array);
                hexdump_d(data.data(), data.size());
                hexdump_d(mask.data(), mask.size());
                Pointer cursor(this->Bpp, 
                                Pointer::CursorSize{static_cast<unsigned>(this->cx), static_cast<unsigned>(this->cy)}, 
                                Pointer::Hotspot{static_cast<unsigned>(this->x), static_cast<unsigned>(this->y)}, 
                                data, mask, 
                                this->red_shift, this->red_max, this->green_shift, this->green_max, this->blue_shift, this->blue_max);
//                Pointer cursor(Pointer::POINTER_DOT);
                drawable.begin_update();
                drawable.set_pointer(cursor);
                drawable.end_update();

                return EncoderState::Exit; 
            }
        };
    } // namespace encoder
} // namespace VNC
