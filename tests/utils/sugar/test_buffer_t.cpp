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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define RED_TEST_MODULE TestBufferT
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/buffer_t.hpp"


RED_AUTO_TEST_CASE(TestBufferT)
{
    char a[2]{};
    uint8_t ua[2]{};
    char const ca[2]{};
    uint8_t const cua[2]{};
    char * s = a+1;
    uint8_t * us = ua;
    char const * cs = ca;
    uint8_t const * cus = cua;

    byte_ptr bs{s};
    const_byte_ptr cbs{cs};
    buffer_t ba;
    const_buffer_t cba;
    array_view_char av{s, 1};
    array_view_u8 uav{us, 1};
    array_view_const_char cav{cs, 1};
    array_view_const_u8 cuav{cus, 1};

    buffer_t{bs, 1};
    buffer_t{a};
    buffer_t{ua};
    buffer_t{av};
    buffer_t{uav};
    buffer_t{ba};

    const_buffer_t{bs, 1};
    const_buffer_t{a};
    const_buffer_t{ua};
    const_buffer_t{av};
    const_buffer_t{uav};
    const_buffer_t{ba};

    const_buffer_t{cbs, 1};
    const_buffer_t{ca};
    const_buffer_t{cua};
    const_buffer_t{cav};
    const_buffer_t{cuav};
    const_buffer_t{cba};

    buffer_t{} = a;
    buffer_t{} = ua;
    buffer_t{} = av;
    buffer_t{} = uav;
    buffer_t{} = ba;

    const_buffer_t{} = a;
    const_buffer_t{} = ua;
    const_buffer_t{} = av;
    const_buffer_t{} = uav;
    const_buffer_t{} = ba;

    const_buffer_t{} = ca;
    const_buffer_t{} = cua;
    const_buffer_t{} = cav;
    const_buffer_t{} = cuav;
    const_buffer_t{} = cba;

    array_view_u8{} = ba;
    array_view_const_u8{} = ba;
    array_view_const_u8{} = cba;

    [](buffer_t){}(a);
    [](buffer_t){}(ua);
    [](buffer_t){}(av);
    [](buffer_t){}(uav);
    [](buffer_t){}(ba);

    [](const_buffer_t){}(a);
    [](const_buffer_t){}(ua);
    [](const_buffer_t){}(av);
    [](const_buffer_t){}(uav);
    [](const_buffer_t){}(ba);

    [](const_buffer_t){}(ca);
    [](const_buffer_t){}(cua);
    [](const_buffer_t){}(cav);
    [](const_buffer_t){}(cuav);
    [](const_buffer_t){}(cba);
}
