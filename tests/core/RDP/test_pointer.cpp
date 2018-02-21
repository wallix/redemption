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

#define RED_TEST_MODULE TestPointer
#include "system/redemption_unit_tests.hpp"

#include "core/RDP/pointer.hpp"

RED_AUTO_TEST_CASE(TestDataSize)
{
    Pointer p;

    RED_CHECK_EQUAL(p.data_size(), 32 * 32 * 3);
}

RED_AUTO_TEST_CASE(TestDataSize1)
{
    Pointer p;

    const Pointer::CursorSize dimensions(24, 24);
    p.set_dimensions(dimensions);

    RED_CHECK_EQUAL(p.data_size(), 24 * 24 * 3);
}

RED_AUTO_TEST_CASE(TestMaskSize)
{
    Pointer p;
 
    const Pointer::CursorSize dimensions(7, 7);
    p.set_dimensions(dimensions);

    RED_CHECK_EQUAL(p.mask_size(), 14);
}
