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
#define BOOST_TEST_MODULE TestBulkCompressionTransport
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "transport/bulk_compression_transport.hpp"
#include "transport/test_transport.hpp"

BOOST_AUTO_TEST_CASE(TestBulkCompressionTransport)
{
    //for (unsigned int i = 0; i < 100000; i++) {
        MemoryTransport mt;

        {
            BulkCompressionOutTransport out_trans(mt);

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
            BulkCompressionInTransport  in_trans(mt);

            char   in_data1[21] = { 0 };
            char * in_buffer   = in_data1;

            in_trans.recv(&in_buffer, 21);
            BOOST_CHECK_EQUAL(in_data1,
                "azert"
                "azert"
                "azert"
                "azert");

            char   in_data2[31] = { 0 };
            in_buffer = in_data2;

            in_trans.recv(&in_buffer, 31);
            BOOST_CHECK_EQUAL(in_data2,
                "wallix"
                "wallix"
                "wallix"
                "wallix"
                "wallix");

            char   in_data3[65] = { 0 };
            in_buffer = in_data3;
            ;
            in_trans.recv(&in_buffer, 65);
            BOOST_CHECK_EQUAL(in_data3,
                "0123456789ABCDEF"
                "0123456789ABCDEF"
                "0123456789ABCDEF"
                "0123456789ABCDEF");
        }
    //}
}
