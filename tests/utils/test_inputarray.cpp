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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to stream object
   Using lib boost functions, some tests need to be added
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestInputArray
#include <boost/test/auto_unit_test.hpp>

//#define LOGNULL
#define LOGPRINT
#include "log.hpp"

#include <string.h>

#include "inputarray.hpp"

BOOST_AUTO_TEST_CASE(TestInputArrayCreation)
{
    struct DS : public DataSource
    {
        uint64_t read(uint8_t * target, size_t target_size)
        {
            enum { read_size = 10 };
            memset(target, 0x11, read_size);
            return read_size;
        }
    } ds;


    // test we can create an InputArray object
    StaticInputArray<50> b(ds);

    BOOST_CHECK(b.capacity() == 50);

    BOOST_CHECK(b.size() == 10);
    size_t i = 0;
    for (; i < b.size() ; ++i){
        BOOST_CHECK(b.get_uint8(i) == 0x11);
    }

}

BOOST_AUTO_TEST_CASE(TestInputBufferCreation)
{
    struct DS : public DataSource
    {
        uint64_t read(uint8_t * target, size_t target_size)
        {
            enum { read_size = 10 };
            memset(target, 0x11, read_size);
            return read_size;
        }
    } ds;


    // test we can create an InputArray object
    StaticBufferArray<50> b(ds);

    BOOST_CHECK(b.capacity() == 50);

    BOOST_CHECK(b.size() == 10);
    size_t i = 0;
    for (; i < b.size() ; ++i){
        BOOST_CHECK(b.get_uint8() == 0x11);
    }
}

