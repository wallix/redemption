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
#define BOOST_TEST_MODULE TestRemotePrograms
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL

#include "log.hpp"
#include "RDP/remote_programs.hpp"

BOOST_AUTO_TEST_CASE(TestRAILPDUHeader)
{
    BStream stream(128);

    RAILPDUHeader_Send header_s(stream, TS_RAIL_ORDER_EXEC);
    header_s.set_orderLength(24);

    stream.rewind();

    RAILPDUHeader_Recv header_r(stream);

    BOOST_CHECK_EQUAL(header_r.orderType(),   TS_RAIL_ORDER_EXEC);
    BOOST_CHECK_EQUAL(header_r.orderLength(), 24                );
}
