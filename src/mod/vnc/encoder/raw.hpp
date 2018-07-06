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

#pragma once

#include "core/buf64k.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/vnc/encoder/encoder_api.hpp"
#include "mod/vnc/vnc_verbose.hpp"
#include "utils/bitmap.hpp"
#include "utils/log.hpp"
#include "utils/sugar/update_lock.hpp"
#include "utils/verbose_flags.hpp"


// 7.6.1   Raw Encoding
// ====================

// The simplest encoding type is raw pixel data. In this case the data consists of width * height pixel values (where width
// and height are the width and height of the rectangle). The values simply represent each pixel in left-to-right scanline order.
// All RFB clients must be able to cope with pixel data in this raw encoding, and RFB servers should only produce raw encoding
// unless the client specifically asks for some other encoding type.

//         No. of bytes                    Type               Description
// width * height * bytesPerPixel      PIXEL array              pixels

namespace VNC {
    namespace Encoder {
        class Raw : public EncoderApi {
            const uint8_t bpp;
            const uint8_t Bpp;
            const size_t x;
            size_t y;
            const size_t cx;
            size_t cy;

        public:
            VNCVerbose verbose;

            Raw(uint8_t bpp, uint8_t Bpp, size_t x, size_t y, size_t cx, size_t cy, VNCVerbose verbose)
                : bpp(bpp), Bpp(Bpp), x(x), y(y), cx(cx), cy(cy)
                , verbose(verbose)
            {
            }

            virtual ~Raw(){}

            // return is true if the Encoder has finished working (can be reset or deleted),
            // return is false if the encoder is waiting for more data
            EncoderState consume(Buf64k & buf, gdi::GraphicApi & drawable) override
            {
                if (this->cx == 0 || this->cy == 0)
                {
                    return EncoderState::Exit;
                }

                size_t const line_size = this->cx * this->Bpp;

                if (buf.remaining() < line_size) {
                    return EncoderState::NeedMoreData;
                }

                auto const cy = std::min<size_t>(buf.remaining() / line_size, this->cy);
                auto const new_av = buf.av(cy * line_size);

                Rect rect(this->x, this->y, this->cx, cy);

                update_lock<gdi::GraphicApi> lock(drawable);
                this->draw_tile(rect, new_av.data(), drawable);

                this->y += cy;
                this->cy -= cy;

                buf.advance(new_av.size());
                if (this->cy == 0){
                    return EncoderState::Exit;
                }
                return EncoderState::Ready;
            }

            public:
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

    } // namespace encoder
} // namespace VNC
