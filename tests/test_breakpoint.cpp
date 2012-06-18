/*
 * T his program *is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2010-2012
 * Author(s): Christophe Grosjean, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestBreakpoint
#include <boost/test/auto_unit_test.hpp>

#include "capture.hpp"
#include "transport.hpp"

#include "unlink.hpp"

BOOST_AUTO_TEST_CASE(TestBreakpoint)
{
    //MetaWRM meta(800, 600, 24);
    MetaWRM meta(1024, 912, 16);
    Capture cap(meta.width, meta.height, "/tmp/test_breakpoint", 0, 0);
    cap.emit_meta(meta);
    Rect clip(0, 0, meta.width, meta.height);
    cap.draw(RDPOpaqueRect(Rect(10,844,500,42), RED), clip);
    BOOST_CHECK(1);
    cap.breakpoint();
    BOOST_CHECK(1);
    cap.draw(RDPOpaqueRect(Rect(777,110,144,188), GREEN), clip);
    cap.breakpoint();
    BOOST_CHECK(1);
    cap.draw(RDPOpaqueRect(Rect(200,400,60,60), BLUE), clip);
    cap.timestamp();
    BOOST_CHECK(1);

    TODO("if boost::unit_test::error_count() == 0")
    unlink_wrm("/tmp/test_breakpoint", 3);
}
