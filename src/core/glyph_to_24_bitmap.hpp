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

#include "utils/colors.hpp"
#include "core/font.hpp"

class GlyphTo24Bitmap
{
    // TODO BGRArray<n>
    uint8_t raw_data[16384/*RDPSerializer::MAX_ORDERS_SIZE*/];

public:
    [[nodiscard]] uint8_t const * data() const noexcept { return this->raw_data; }

    GlyphTo24Bitmap(
        FontChar const & fc,
        const BGRColor color_fore,
        const BGRColor color_back) noexcept
    {
        assert(fc.width*fc.height*3 < int(sizeof(this->raw_data)));

        const uint8_t * fc_data = fc.data.get();

        for (int y = 0 ; y < fc.height; y++) {
            uint8_t fc_bit_mask = 128;
            for (int x = 0 ; x < fc.width; x++) {
                if (!fc_bit_mask) {
                    fc_data++;
                    fc_bit_mask = 128;
                }

                const uint16_t xpix = x * 3;
                const uint16_t ypix = y * fc.width * 3;

                const BGRColor color = (fc_bit_mask & *fc_data) ? color_back : color_fore;
                this->raw_data[xpix + ypix    ] = color.blue();
                this->raw_data[xpix + ypix + 1] = color.green();
                this->raw_data[xpix + ypix + 2] = color.red();

                fc_bit_mask >>= 1;
            }
            fc_data++;
        }
    }
};
