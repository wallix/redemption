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

#include "mod/vnc/vnc.hpp"
#include "test_only/test_framework/redemption_unit_tests.hpp"

// TEST missing
RED_AUTO_TEST_CASE(vnc_mouse)
{
    StaticOutStream<128> out_stream;
    auto get_bytes = [&]{
        auto av = out_stream.get_bytes();
        out_stream.rewind();
        return av;
    };

    mod_vnc::Mouse mouse;
    mouse.move(out_stream, 10, 10);
    RED_TEST(get_bytes() == "\x05\x00\x00\x0a\x00\x0a"_av);
    mouse.scroll(out_stream, 8);
    RED_TEST(get_bytes() == "\x05\x08\x00\x0a\x00\x0a\x05\x00\x00\x0a\x00\x0a"_av);
    mouse.click(out_stream, 10, 10, 1, true);
    RED_TEST(get_bytes() == "\x05\x01\x00\x0a\x00\x0a"_av);
    mouse.click(out_stream, 10, 10, 2, true);
    RED_TEST(get_bytes() == "\x05\x03\x00\x0a\x00\x0a"_av);
    mouse.click(out_stream, 10, 10, 1, false);
    RED_TEST(get_bytes() == "\x05\x02\x00\x0a\x00\x0a"_av);
    mouse.move(out_stream, 15, 17);
    RED_TEST(get_bytes() == "\x05\x02\x00\x0f\x00\x11"_av);
    mouse.click(out_stream, 15, 18, 2, false);
    RED_TEST(get_bytes() == "\x05\x00\x00\x0f\x00\x12"_av);
}
