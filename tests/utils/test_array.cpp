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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestArray
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
//#define LOGPRINT

#include "utils/array.hpp"



BOOST_AUTO_TEST_CASE(Test_Array)
{
    BOOST_CHECK(true);
    bool test_equal(false);

    Array array_0;
    test_equal = false;
    BOOST_CHECK_EQUAL(array_0.size(), 65536);
    if (array_0.get_data() != nullptr) {
        test_equal = true;
    }
    BOOST_CHECK_EQUAL(test_equal, true);


    Array array_1(42);
    test_equal = false;
    BOOST_CHECK_EQUAL(array_1.size(), 42);
    if (array_1.get_data() != nullptr) {
        test_equal = true;
    }
    BOOST_CHECK_EQUAL(test_equal, true);


    Array array_2(65536 + 1);
    test_equal = false;
    BOOST_CHECK_EQUAL(array_2.size(), 65537);
    if (array_2.get_data() != nullptr) {
        test_equal = true;
    }
    BOOST_CHECK_EQUAL(test_equal, true);


    Array array_3;
    test_equal = false;
    BOOST_CHECK_EQUAL(array_3.size(), 65536);
    if (array_3.get_data() != nullptr) {
        test_equal = true;
    }
    BOOST_CHECK_EQUAL(test_equal, true);
    array_3.copy(array_1);
    test_equal = false;
    BOOST_CHECK_EQUAL(array_3.size(), 42);
    if (array_3.get_data() != nullptr) {
        test_equal = true;
    }
    BOOST_CHECK_EQUAL(test_equal, true);


    Array array_4;
    test_equal = false;
    BOOST_CHECK_EQUAL(array_4.size(), 65536);
    if (array_4.get_data() != nullptr) {
        test_equal = true;
    }
    BOOST_CHECK_EQUAL(test_equal, true);
    array_4.init(42);
    test_equal = false;
    BOOST_CHECK_EQUAL(array_4.size(), 42);
    if (array_4.get_data() != nullptr) {
        test_equal = true;
    }
    BOOST_CHECK_EQUAL(test_equal, true);


    Array array_5;
    test_equal = false;
    BOOST_CHECK_EQUAL(array_5.size(), 65536);
    if (array_5.get_data() != nullptr) {
        test_equal = true;
    }
    BOOST_CHECK_EQUAL(test_equal, true);
    uint8_t source[] = {0,  1,  2,  3};
    array_5.copy(source, 3, 1);
    test_equal = false;
    BOOST_CHECK_EQUAL(array_5.size(), 65536);
    if (array_5.get_data() != nullptr) {
        test_equal = true;
    }
    BOOST_CHECK_EQUAL(test_equal, true);
    BOOST_CHECK_EQUAL(array_5.get_data()[0], 0);
    BOOST_CHECK_EQUAL(array_5.get_data()[1], 0);
    BOOST_CHECK_EQUAL(array_5.get_data()[2], 1);


}
