/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestGeneratorRIO
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include"log.hpp"
#include"rio/rio.h"
#include"rio/rio_impl.h"

BOOST_AUTO_TEST_CASE(TestGenerator)
{
    // RIO Generator is used in test to provide data from test body
    // instead of reading it from system files
    // It helps writing tests running faster and easier to read

    // We initialize it with some buffer data and size
    // When generator consumers will call receive the data will be provided back

    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_generator(&status, "We read what we provide!", 24);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    uint8_t buffer[1024];
    
    // We call rio_recv asking for 3 characters, 
    // it gives back the first 3 characters of the generator data "We "
    BOOST_CHECK_EQUAL(3, rio_recv(rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));

    // We call rio_recv asking for next 1024 characters,
    // The generator does not have 1024 characters remaining
    // hence it returns back what is available 21 characters
    // "read what we provide!" is appended to buffer
    BOOST_CHECK_EQUAL(21, rio_recv(rt, buffer+3, 1024));
    BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));

    // next call to rio_recv will return 0 (EOF)
    BOOST_CHECK_EQUAL(0, rio_recv(rt, buffer+24, 1024)); // EOF
    
    rio_delete(rt);
}

BOOST_AUTO_TEST_CASE(TestGenerator2)
{
    // same test as above but using object allocated on stack 
    // instead of mallocated on heap
    RIO rt;
    RIO_ERROR status = rio_init_generator(&rt, "We read what we provide!", 24);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    uint8_t buffer[1024];
    
    BOOST_CHECK_EQUAL(3, rio_recv(&rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));

    BOOST_CHECK_EQUAL(21, rio_recv(&rt, buffer+3, 1024));
    BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));

    // next call to rio_recv will return 0 (EOF)
    BOOST_CHECK_EQUAL(0, rio_recv(&rt, buffer+24, 1024)); // EOF
    
    rio_clear(&rt);
}
