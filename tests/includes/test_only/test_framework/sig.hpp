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
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2020
 */

#pragma once

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/bytes_view.hpp"


#define RED_CHECK_SIG_A(obj, expected_sig) \
    RED_TEST(::ut::sig(obj).bytes() == expected_sig)

#define RED_CHECK_SIG(obj, expected_sig) \
    RED_CHECK_SIG_A(obj, expected_sig ""_av)

namespace ut
{
    struct Sig
    {
        unsigned char buf[20];

        flagged_bytes_view bytes() const noexcept { return hex(bytes_view(buf, sizeof(buf))); }
    };

    Sig sig(bytes_view data);
} // namespace ut
