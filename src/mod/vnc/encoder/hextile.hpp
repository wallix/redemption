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

   Vnc encoder module for Hextile bitmaps
*/

#pragma once

#include "utils/log.hpp"
#include "utils/verbose_flags.hpp"
#include "mod/vnc/vnc_verbose.hpp"

namespace VNC {
    namespace Encoder {
        class Hextile : public EncoderApi {
            const uint8_t bpp;
            const uint8_t Bpp;
            const size_t x;
            size_t y;
            const size_t cx;
            size_t cy;

            enum class HextileState {
                Header,
                Data,
                Exit
            } state;
        
        public:
            VNCVerbose verbose;

            Hextile(uint8_t bpp, uint8_t Bpp, size_t x, size_t y, size_t cx, size_t cy, VNCVerbose verbose) 
                : bpp(bpp), Bpp(Bpp), x(x), y(y), cx(cx), cy(cy)
                , state(RawState::Header)
                , verbose(verbose)
            {
            }
            
            virtual ~Hextile(){}
            
            // return is true if the Encoder has finished working (can be reset or deleted),
            // return is false if the encoder is waiting for more data
            bool consume(Buf64k & buf, gdi::GraphicApi & drawable) override
            {
                return true; // finished decoding
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
                            // TODO: fix here magic number 16 is vnc.bpp
                            const Bitmap tiled_bmp(raw, rect.cx, rect.cy, 16, src_tile);
                            const Rect dst_tile(rect.x + x, rect.y + y, cx, cy);
                            const RDPMemBlt cmd2(0, dst_tile, 0xCC, 0, 0, 0);
                            drawable.draw(cmd2, dst_tile, tiled_bmp);
                        }
                    }
                }
        };    
    } // namespace encoder
} // namespace VNC
