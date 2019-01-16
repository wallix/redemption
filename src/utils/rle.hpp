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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni, Martin Potier,
              Meng Tan, Clement Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   This file implement the bitmap items data structure
   including RDP RLE compression and decompression algorithms

   It also features storage and color versionning of the bitmap
   returning a pointer on a table, corresponding to the required
   color model.
*/

#pragma once

#include <cstddef>

class OutStream;
class ConstImageDataView;
class MutableImageDataView;

void rle_compress(ConstImageDataView const & image, OutStream & outbuffer);

void rle_compress60(ConstImageDataView const & image, OutStream & outbuffer);

// Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
void rle_decompress(
    MutableImageDataView const & image,
    const uint8_t* input, uint16_t src_cx, uint16_t src_cy, std::size_t size, std::size_t* RM18446_adjusted_size);

void rle_decompress60(
  MutableImageDataView const & image,
  uint16_t src_cx, uint16_t src_cy, const uint8_t *data, std::size_t data_size);
