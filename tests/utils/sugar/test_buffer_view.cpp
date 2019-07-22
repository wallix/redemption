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

#include "utils/sugar/buffer_view.hpp"


// test for compilation
int main()
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
    buffer_view ba;
    const_buffer_view cba;
    array_view_char av{s, 1};
    array_view_u8 uav{us, 1};
    array_view_const_char cav{cs, 1};
    array_view_const_u8 cuav{cus, 1};

    buffer_view{bs, 1};
    buffer_view{a};
    buffer_view{ua};
    buffer_view{av};
    buffer_view{uav};
    buffer_view{ba};

    const_buffer_view{bs, 1};
    const_buffer_view{a};
    const_buffer_view{ua};
    const_buffer_view{av};
    const_buffer_view{uav};
    const_buffer_view{ba};

    const_buffer_view{cbs, 1};
    const_buffer_view{ca};
    const_buffer_view{cua};
    const_buffer_view{cav};
    const_buffer_view{cuav};
    const_buffer_view{cba};

    buffer_view{} = a;
    buffer_view{} = ua;
    buffer_view{} = av;
    buffer_view{} = uav;
    buffer_view{} = ba;

    const_buffer_view{} = a;
    const_buffer_view{} = ua;
    const_buffer_view{} = av;
    const_buffer_view{} = uav;
    const_buffer_view{} = ba;

    const_buffer_view{} = ca;
    const_buffer_view{} = cua;
    const_buffer_view{} = cav;
    const_buffer_view{} = cuav;
    const_buffer_view{} = cba;

    array_view_u8{} = ba;
    array_view_const_u8{} = ba;
    array_view_const_u8{} = cba;

    [](buffer_view){}(a);
    [](buffer_view){}(ua);
    [](buffer_view){}(av);
    [](buffer_view){}(uav);
    [](buffer_view){}(ba);

    [](const_buffer_view){}(a);
    [](const_buffer_view){}(ua);
    [](const_buffer_view){}(av);
    [](const_buffer_view){}(uav);
    [](const_buffer_view){}(ba);

    [](const_buffer_view){}(ca);
    [](const_buffer_view){}(cua);
    [](const_buffer_view){}(cav);
    [](const_buffer_view){}(cuav);
    [](const_buffer_view){}(cba);
}
