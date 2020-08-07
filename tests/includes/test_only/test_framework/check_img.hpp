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


#define RED_CHECK_IMG(img, filedata_path) \
    RED_TEST(::ut::CheckImg(img, filedata_path) == nullptr)

class ConstImageDataView;

namespace ut
{
    struct CheckImg
    {
        std::string err;

        CheckImg(ConstImageDataView const& img, char const* filedata_path);

        operator char const* () const noexcept;
    };

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
    std::ostream& boost_test_print_type(std::ostream& ostr, CheckImg const& x);
#endif
}
