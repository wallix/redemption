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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "capture/rail_screen_computation.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"


RED_AUTO_TEST_CASE(TestRailScreenComputation)
{
    RailScreenComputation rail_screen(100, 100, false);

    RED_TEST(rail_screen.get_max_image_frame_rect().isempty());

    rail_screen.visibility_rect_event(Rect(10, 10, 25, 10));
    RED_TEST(rail_screen.get_max_image_frame_rect() == Rect(10, 10, 25, 10));
    RED_TEST(rail_screen.get_min_image_frame_dim() == Dimension(25, 10));

    // WINDOW_ORDER_FIELD_VISIBILITY
    RDP::RAIL::NewOrExistingWindow cmd;
    InStream in_stream(
        // orderSize(0), fieldsPresentFlags(VISIBILITY|VISOFFSET), windowId(1)
        "\x00\x00" "\x00\x12\x00\x00" "\x01\x00\x00\x00"
        // offsetX(0), offsetY(0)
        "\x00\x00\x00\x00" "\x00\x00\x00\x00"
        // size(1)
        "\x01\x00"
        // left(5), top(10), right(20), bottom(49)
        "\x05\x00" "\x0A\x00" "\x14\x00" "\x31\x00"
        ""_av);
    cmd.receive(in_stream);
    rail_screen.draw(cmd);
    RED_TEST(rail_screen.get_max_image_frame_rect() == Rect(5, 10, 30, 40));
    RED_TEST(rail_screen.get_min_image_frame_dim() == Dimension(25, 40));

    rail_screen.draw(RDP::RAIL::NonMonitoredDesktop());
    RED_TEST(rail_screen.get_max_image_frame_rect() == Rect(0, 0, 100, 100));
    RED_TEST(rail_screen.get_min_image_frame_dim() == Dimension(100, 100));
}
