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
   Copyright (C) Wallix 2014
   Author(s): Christophe Grosjean, Meng Tan

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestDArray
#include "system/redemption_unit_tests.hpp"

// #define LOGNULL
#define LOGPRINT

#include "utils/darray.hpp"
struct Something {
    int a;
    Something()
        : a(45)
    {}
    Something(int b)
    : a(b)
    {}
};

BOOST_AUTO_TEST_CASE(TestDArray)
{
    DArray<Something> collection(11);

    BOOST_CHECK_EQUAL(collection.size(), 11);
    BOOST_CHECK_EQUAL(collection[5].a, 45);
    collection[7] = 12;
    BOOST_CHECK_EQUAL(collection[7].a, 12);

    DArray<Something> copy_collection(collection);
    BOOST_CHECK_EQUAL(copy_collection[5].a, 45);
    BOOST_CHECK_EQUAL(copy_collection[7].a, 12);
    collection[7] = 9;
    BOOST_CHECK_EQUAL(copy_collection[7].a, 12);

    DArray<int> integers(0);
    BOOST_CHECK_EQUAL(integers.size(), 0);
}
