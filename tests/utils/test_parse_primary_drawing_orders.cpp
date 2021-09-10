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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/log_buffered.hpp"

#include "utils/parse_primary_drawing_orders.hpp"
#include "utils/parse_primary_drawing_orders.hpp"

RED_AUTO_TEST_CASE(TestPrimaryDrawaingOrders)
{
    ut::log_buffered log_buf;
    auto orders = parse_primary_drawing_orders("4  25 0X8,,0xf 666 2", false);
    RED_CHECK(orders.as_uint() == (
        OrdersIndexes::TS_NEG_SCRBLT_INDEX
      | OrdersIndexes::TS_NEG_MEM3BLT_INDEX
      | OrdersIndexes::TS_NEG_LINETO_INDEX
      | OrdersIndexes::TS_NEG_MULTIDSTBLT_INDEX
      | OrdersIndexes::TS_NEG_ELLIPSE_SC_INDEX
    ).as_uint());
    RED_CHECK(log_buf.buf() == "WARNING -- Unknown RDP PrimaryDrawingOrder=666\n");
}
