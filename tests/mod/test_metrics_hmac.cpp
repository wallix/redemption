/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*1324
*   Product name: redemption, a FLOSS VNC proxy
*   Copyright (C) Wallix 2018
*   Author(s): Clément Moroldo
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/metrics_hmac.hpp"


RED_AUTO_TEST_CASE(TestVNCMetricsH)
{
    auto key = "random key"_av;

    RED_CHECK_MEM("A6022FBB95CB52C8012CDE117B5FEE39E32B9DD8A789A9D857AD72C1F97400ED"_av,
        hmac_user("primaryuser"_av, key));

    RED_CHECK_MEM("7D47FE8C5EC59EC47527B5509B4AB75F0645A802B1A77AEE2F89D0F6EBA32B3A"_av,
        hmac_account("secondaryuser"_av, key));

    RED_CHECK_MEM("1CDD18B14EE86005EAB1E1F9BF95ECA85D52FCBA599C2D08359178816414526A"_av,
        hmac_device_service("device1"_av, "service1", key));

    ScreenInfo info;
    RED_CHECK_MEM("2DACE33E1B966CB34ACD0B66B724810DE3D5E2B042729A9F2698EE9E23D6F40E"_av,
        hmac_client_info("10.10.13.12", info, key));
}
