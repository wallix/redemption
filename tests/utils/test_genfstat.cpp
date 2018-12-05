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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to region object
   Using lib boost functions, some tests need to be added

*/


#define RED_TEST_MODULE TestGenFstat
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/genfstat.hpp"

RED_AUTO_TEST_CASE(TestGenFstat)
{
    struct stat stat;
    Fstat fstat;
    int res = fstat.stat(FIXTURES_PATH "/sample0.wrm", stat);
    RED_CHECK_EQUAL(res, 0);
    RED_CHECK_EQUAL(stat.st_size, 1471394);
}
