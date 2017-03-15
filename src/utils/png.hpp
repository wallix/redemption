/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/

#pragma once

#include <cstddef>
#include <cstdio> // FILE
#include "utils/sugar/array_view.hpp"

class Transport;
class FileTransport;
namespace gdi
{
    class GraphicApi;
}

void transport_dump_png24(
    Transport & trans,
    uint8_t const * data,
    size_t width,
    size_t height,
    size_t rowsize,
    bool bgr
);

void file_transport_dump_png24(
    FileTransport & trans,
    uint8_t const * data,
    size_t width,
    size_t height,
    size_t rowsize,
    bool bgr
);

void dump_png24(
    std::FILE * fd,
    uint8_t const * data,
    size_t width,
    size_t height,
    size_t rowsize,
    bool bgr
);

void read_png24(
    std::FILE * fd,
    uint8_t * data,
    size_t width,
    size_t height,
    size_t rowsize
);

void transport_read_png24(
    Transport & trans,
    uint8_t * data,
    size_t width,
    size_t height,
    size_t rowsize
);

void set_rows_from_image_chunk(
    Transport & trans,
    uint16_t chunk_type,
    uint32_t chunk_size,
    uint16_t cx,
    array_view<gdi::GraphicApi*>
);
