/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_CORE_RDP_COMPRESS_AND_DRAW_BITMAP_UPDATE_HPP
#define REDEMPTION_CORE_RDP_COMPRESS_AND_DRAW_BITMAP_UPDATE_HPP

#include "bitmap.hpp"
#include "bitmapupdate.hpp"
#include "RDPGraphicDevice.hpp"

inline
void compress_and_draw_bitmap_update( const RDPBitmapData & bitmap_data, const Bitmap & bmp
                                    , uint8_t target_bpp, RDPGraphicDevice & gd) {
    StaticOutStream<65535> bmp_stream;
    bmp.compress(target_bpp, bmp_stream);

    RDPBitmapData target_bitmap_data = bitmap_data;

    target_bitmap_data.bits_per_pixel = bmp.bpp();
    target_bitmap_data.flags          = BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR;
    target_bitmap_data.bitmap_length  = bmp_stream.get_offset();

    gd.draw(target_bitmap_data, bmp_stream.get_data(), bmp_stream.get_offset(), bmp);
}

#endif
