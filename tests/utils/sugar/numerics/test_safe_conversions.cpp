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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#define RED_TEST_MODULE TestSplitter
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/numerics/safe_conversions.hpp"


RED_AUTO_TEST_CASE(TestTrim)
{
    RED_CHECK_EQUAL(int(saturated_cast<signed char>(1233412)), 127);
    RED_CHECK_EQUAL(int(saturated_cast<signed char>(-1233412)), -128);
    RED_CHECK_EQUAL(unsigned(saturated_cast<unsigned char>(1233412)), 255u);
    RED_CHECK_EQUAL(unsigned(saturated_cast<unsigned char>(-1233412)), 0u);
    RED_CHECK_EQUAL(saturated_cast<int>(-1233412), -1233412);

    RED_CHECK_EQUAL(checked_cast<char>(12), 12);
    RED_CHECK_EQUAL(checked_cast<int>(12), 12);

    RED_CHECK_EQUAL(int(saturated_int<signed char>(122312)), 127);
    RED_CHECK_EQUAL(int(saturated_int<signed char>(122312) = -3213), -128);

    RED_CHECK_EQUAL(int(checked_int<signed char>(12)), 12);
    RED_CHECK_EQUAL(int(checked_int<signed char>(12) = 13), 13);

    enum uE : unsigned char { uMin = 0, uMax = 255 };
    enum sE : signed char { sMin = -128, sMax = 127 };

    RED_CHECK_EQUAL(int(saturated_cast<sE>(1233412)), 127);
    RED_CHECK_EQUAL(int(saturated_cast<sE>(-1233412)), -128);
    RED_CHECK_EQUAL(unsigned(saturated_cast<uE>(1233412)), 255u);
    RED_CHECK_EQUAL(unsigned(saturated_cast<uE>(-1233412)), 0u);

    RED_CHECK_EQUAL(checked_cast<char>(12), 12);

    RED_CHECK_EQUAL(int(saturated_int<sE>(122312)), 127);
    RED_CHECK_EQUAL(int(saturated_int<sE>(122312) = -3213), -128);

    RED_CHECK_EQUAL(int(checked_int<sE>(12)), 12);
    RED_CHECK_EQUAL(int(checked_int<sE>(12) = 13), 13);

    is_safe_convertible<int, long long>{} = std::true_type{}; safe_cast<long>(1);
    is_safe_convertible<long long, int>{} = std::bool_constant<(sizeof(int) == sizeof(long long))>{};
    is_safe_convertible<int, long>{} = std::true_type{}; safe_cast<long>(1);
    is_safe_convertible<long, int>{} = std::bool_constant<(sizeof(int) == sizeof(long))>{};
    is_safe_convertible<signed char, unsigned>{} = std::false_type{};
    is_safe_convertible<unsigned, signed char>{} = std::false_type{};
    is_safe_convertible<sE, signed char>{} = std::true_type{}; safe_cast<sE>(static_cast<signed char>(1));
    is_safe_convertible<uE, long>{} = std::true_type{}; safe_cast<long>(uE{});
    is_safe_convertible<int, int>{} = std::true_type{}; safe_cast<int>(int{});
    is_safe_convertible<uE, signed char>{} = std::false_type{};
    is_safe_convertible<signed char, uE>{} = std::false_type{};
}
