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

#include "utils/log.hpp"
#include "utils/verbose_flags.hpp"
#include "mod/vnc/vnc_verbose.hpp"

namespace VNC {
    namespace Encoder {
        class CopyRect : public EncoderApi {
            const uint8_t bpp;
            const uint8_t Bpp;
            const size_t x;
            size_t y;
            const size_t cx;
            size_t cy;

			const uint16_t width;
			const uint16_t height;

            enum class CopyrectState {
                Header,
                Data,
                Exit
            } state;
        
        public:
            VNCVerbose verbose;

            CopyRect(uint8_t bpp, uint8_t Bpp, size_t x, size_t y, size_t cx, size_t cy, uint16_t width, uint16_t height, VNCVerbose verbose) 
                : bpp(bpp), Bpp(Bpp), x(x), y(y), cx(cx), cy(cy), width(width), height(height)
                , state(CopyrectState::Header)
                , verbose(verbose)
            {
            }
            
            virtual ~CopyRect(){}
            
            // return is true if the Encoder has finished working (can be reset or deleted),
            // return is false if the encoder is waiting for more data
            bool consume(Buf64k & buf, gdi::GraphicApi & drawable) override
            {
		        const size_t sz = 4;

		        if (buf.remaining() < sz) { return false; }

		        InStream stream_copy_rect(buf.av(sz));
		        uint16_t const srcx = stream_copy_rect.in_uint16_be();
		        uint16_t const srcy = stream_copy_rect.in_uint16_be();

		        buf.advance(sz);

		        update_lock<gdi::GraphicApi> lock(drawable);
		        drawable.draw(
		            RDPScrBlt(Rect(this->x, this->y, this->cx, this->cy), 0xCC, srcx, srcy),
		            Rect(0, 0, this->width, this->height)
		        );

                return true; // finished decoding
            }           
        };
    } // namespace encoder
} // namespace VNC
