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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean
*/

#define UNIT_TEST_MODULE TestBitmapCodecs
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
#include "core/RDP/capabilities/bitmapcodecs.hpp"
#include "utils/stream.hpp"

RED_AUTO_TEST_CASE(TestBitmapCodecCaps_emit)
{
    StaticOutStream<1024> out_stream;
    BitmapCodecCaps cap;
    cap.emit(out_stream);

    InStream in_stream(out_stream.get_data(), out_stream.get_offset());
    BitmapCodecCaps cap2;
    cap2.recv(in_stream, CAPLEN_BITMAP_CODECS);
}
