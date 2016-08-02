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
#define BOOST_TEST_MODULE TestSnappyCompressionTransport
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "transport/snappy_compression_transport.hpp"
#include "transport/test_transport.hpp"

BOOST_AUTO_TEST_CASE(TestSnappyCompressionTransport)
{
    //size_t source_length = 56000;
    //LOG(LOG_INFO, "snappy_max_compressed_length(%u)=%u", source_length, ::snappy_max_compressed_length(source_length));

    //for (unsigned int i = 0; i < 100000; i++) {
        MemoryTransport mt;

        {
            SnappyCompressionOutTransport out_trans(mt);

            out_trans.send(
                  "azert"
                  "azert"
                  "azert"
                  "azert"
                , 21);
            out_trans.send(
                  "wallix"
                  "wallix"
                  "wallix"
                  "wallix"
                  "wallix"
                , 31);
            out_trans.next();
            out_trans.send(
                  "0123456789ABCDEF"
                  "0123456789ABCDEF"
                  "0123456789ABCDEF"
                  "0123456789ABCDEF"
                , 65);
        }

        {
            SnappyCompressionInTransport  in_trans(mt);

            char   in_data[128] = { 0 };
            char * in_buffer   = in_data;


            in_trans.recv(&in_buffer, 21);
            BOOST_CHECK_EQUAL(in_data,
                "azert"
                "azert"
                "azert"
                "azert");

            in_buffer = in_data;
            in_trans.recv(&in_buffer, 31);
            BOOST_CHECK_EQUAL(in_data,
                "wallix"
                "wallix"
                "wallix"
                "wallix"
                "wallix");

            in_buffer = in_data;
            in_trans.recv(&in_buffer, 65);
            BOOST_CHECK_EQUAL(in_data,
                "0123456789ABCDEF"
                "0123456789ABCDEF"
                "0123456789ABCDEF"
                "0123456789ABCDEF");
        }
    //}
}
