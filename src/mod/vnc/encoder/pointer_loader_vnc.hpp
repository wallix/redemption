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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include "core/RDP/rdp_pointer.hpp"

namespace VNC
{

// VNC Pointer format
// ==================

// The data consists of width * height pixel values followed by
// a bitmask.

// PIXEL array : width * height * bytesPerPixel
// bitmask     : floor((width + 7) / 8) * height

// The bitmask consists of left-to-right, top-to-bottom
// scanlines, where each scanline is padded to a whole number of
// bytes. Within each byte the most significant bit represents
// the leftmost pixel, with a 1-bit meaning the corresponding
// pixel in the cursor is valid.

struct PointerLoaderVnc
{
    PointerLoaderVnc() = default;

    RdpPointerView load(
        BytesPerPixel Bpp,
        uint16_t width, uint16_t height,
        uint16_t hsx, uint16_t hsy,
        u8_array_view vncdata, u8_array_view vncmask,
        unsigned red_shift, unsigned red_max,
        unsigned green_shift, unsigned green_max,
        unsigned blue_shift, unsigned blue_max) noexcept
    {
        uint16_t minheight = std::min(height, uint16_t(32));
        uint16_t minwidth = 32;

        this->dimensions = CursorSize{minwidth, minheight};
        this->hotspot = Hotspot{hsx, hsy};

        size_t target_offset_line = 0;
        size_t target_mask_offset_line = 0;
        size_t source_offset_line = (minheight-1) * width * underlying_cast(Bpp);
        size_t source_mask_offset_line = (minheight-1) * ::nbbytes(width);

        // LOG(LOG_INFO, "r%u rs<<%u g%u gs<<%u b%u bs<<%u", red_max, red_shift, green_max, green_shift, blue_max, blue_shift);
        for (size_t y = 0; y < minheight; ++y) {
            for (size_t x = 0; x < ::nbbytes(minwidth); ++x) {
                // LOG(LOG_INFO, "y=%u xx=%u source_mask_offset=%u target_mask_offset=%u")";
                if (x < ::nbbytes(width)){
                    this->and_mask[target_mask_offset_line+x] = 0xFF ^ vncmask[source_mask_offset_line+x];
                }
                else {
                    this->and_mask[target_mask_offset_line+x] = 0xFF;
                }
            }

            if (minwidth % 8) {
                this->and_mask[target_mask_offset_line+::nbbytes(minwidth)-1] |= (0xFF>>(minwidth % 8));
            }

            size_t source_offset = source_offset_line;
            for (size_t x = 0; x < minwidth; ++x, source_offset += underlying_cast(Bpp)) {
                const size_t target_offset = target_offset_line + x*3;
                // don't use vncdata for hidden pixel because should be random initialized
                if (x >= width || (this->and_mask[target_mask_offset_line+x/8] & (0x80 >> (x % 8)))) {
                    this->xor_mask[target_offset  ] = 0;
                    this->xor_mask[target_offset+1] = 0;
                    this->xor_mask[target_offset+2] = 0;
                }
                else {
                    unsigned pixel = 0;
                    for(size_t i = 0 ; i < underlying_cast(Bpp); ++i) {
                        pixel |= unsigned(vncdata[source_offset+i]) << (i*8);
                    }
                    const unsigned red   = (pixel >> red_shift  ) & red_max;
                    const unsigned green = (pixel >> green_shift) & green_max;
                    const unsigned blue  = (pixel >> blue_shift ) & blue_max;
                    // LOG(LOG_INFO, "pixel=%.2X (%.1X, %.1X, %.1X)", pixel, red, green, blue);
                    this->xor_mask[target_offset  ] = uint8_t((red   << 3) | (red   >> 2));
                    this->xor_mask[target_offset+1] = uint8_t((green << 2) | (green >> 4));
                    this->xor_mask[target_offset+2] = uint8_t((blue  << 3) | (blue  >> 2));
                }
            }

            target_offset_line += minwidth*3;
            target_mask_offset_line += ::nbbytes(minwidth);
            source_offset_line -= width*underlying_cast(Bpp);
            source_mask_offset_line -= ::nbbytes(width);
        }

        return as_pointer_view();
    }

    RdpPointerView as_pointer_view() const noexcept
    {
        return RdpPointerView::from_raw_ptr(
            this->dimensions,
            this->hotspot,
            BitsPerPixel{24},
            this->xor_mask,
            this->and_mask);
    }

private:
    CursorSize dimensions {32, 32};
    Hotspot hotspot {0, 0};

    uint8_t xor_mask[RdpPointer::MAX_WIDTH * RdpPointer::MAX_HEIGHT * 3];
    uint8_t and_mask[RdpPointer::MAX_WIDTH * RdpPointer::MAX_HEIGHT / 8];
};

} // namespace VNC
