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

#define RED_TEST_MODULE TestUnderLyingCast
#include "system/redemption_unit_tests.hpp"

#include "utils/sugar/underlying_cast.hpp"

RED_AUTO_TEST_CASE(TestUnderlyingCast)
{
    enum E1 : char {A};
    enum E2 : long {B};
    auto a = underlying_cast(A);
    auto b = underlying_cast(B);
    char* ap = &a;
    long* bp = &b;
    (void)ap;
    (void)bp;
}
