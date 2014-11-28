/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestBufferizationTransport
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
//#define LOGPRINT
#include "log.hpp"

#include "bufferization_transport.hpp"
#include "test_transport.hpp"

BOOST_AUTO_TEST_CASE(TestBufferizationTransport)
{
    //for (unsigned int i = 0; i < 100000; i++) {
        MemoryTransport mt;

        const char data1[] =
            "azert"
            "azert"
            "azert"
            "azert";
        const char data2[] =
            "wallix"
            "wallix"
            "wallix"
            "wallix"
            "wallix";
        const char data3[] =
            "0123456789ABCDEF"
            "0123456789ABCDEF"
            "0123456789ABCDEF"
            "0123456789ABCDEF";
        const char handle = 4;

        {
            BufferizationOutTransport out_trans(mt, 0xFFFF);

            out_trans.send(data1, sizeof(data1));
            out_trans.send(data2, sizeof(data2));
            BOOST_CHECK_EQUAL(mt.out_stream.get_offset(), 0);
            out_trans.next();
            BOOST_CHECK_EQUAL(mt.out_stream.get_offset(), sizeof(data1) + sizeof(data2) + handle);
            out_trans.send(data3, sizeof(data3));
        }

        {
            BufferizationInTransport in_trans(mt, 0xFFFF);

            char   in_data[128] = { 0 };
            char * in_buffer   = in_data;

            in_trans.recv(&in_buffer, sizeof(data1));
            BOOST_CHECK_EQUAL(in_data, data1); in_buffer = in_data;
            in_trans.recv(&in_buffer, sizeof(data2));
            BOOST_CHECK_EQUAL(in_data, data2); in_buffer = in_data;
            in_trans.recv(&in_buffer, sizeof(data3));
            BOOST_CHECK_EQUAL(in_data, data3);
        }
    //}
}
