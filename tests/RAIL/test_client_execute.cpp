/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Meng Tan
*/

#include "RAIL/client_execute.hpp"

#include "test_only/test_framework/redemption_unit_tests.hpp"


RED_AUTO_TEST_CASE(TestDirections)
{
    Rect window(100,100,200,200);

    ClientExecute::Zone zone;

    RED_CHECK(Rect(124, 100, 152, 3) == zone.get_zone(zone.ZONE_N, window));
    RED_CHECK(Rect(100, 100, 24, 3) == zone.get_zone(zone.ZONE_NWN, window));
    RED_CHECK(Rect(100, 100, 3, 24) == zone.get_zone(zone.ZONE_NWW, window));
    RED_CHECK(Rect(100, 124, 3, 152) == zone.get_zone(zone.ZONE_W, window));
    RED_CHECK(Rect(100, 276, 3, 24) == zone.get_zone(zone.ZONE_SWW, window));
    RED_CHECK(Rect(100, 297, 24, 3) == zone.get_zone(zone.ZONE_SWS, window));
    RED_CHECK(Rect(124, 297, 152, 3) == zone.get_zone(zone.ZONE_S, window));
    RED_CHECK(Rect(276, 297, 24, 3) == zone.get_zone(zone.ZONE_SES, window));
    RED_CHECK(Rect(297, 276, 3, 24) == zone.get_zone(zone.ZONE_SEE, window));
    RED_CHECK(Rect(297, 124, 3, 152) == zone.get_zone(zone.ZONE_E, window));
    RED_CHECK(Rect(297, 100, 3, 24) == zone.get_zone(zone.ZONE_NEE, window));
    RED_CHECK(Rect(276, 100, 24, 3) == zone.get_zone(zone.ZONE_NEN, window));

    RED_CHECK(Rect(101, 101, 21, 23) == zone.get_zone(zone.ZONE_ICON, window));
    RED_CHECK(Rect(122, 101, 66, 23) == zone.get_zone(zone.ZONE_TITLE, window));
    RED_CHECK(Rect(151, 101, 37, 23) == zone.get_zone(zone.ZONE_RESIZE, window));
    RED_CHECK(Rect(188, 101, 37, 23) == zone.get_zone(zone.ZONE_MINI, window));
    RED_CHECK(Rect(225, 101, 37, 23) == zone.get_zone(zone.ZONE_MAXI, window));
    RED_CHECK(Rect(262, 101, 37, 23) == zone.get_zone(zone.ZONE_CLOSE, window));
}
