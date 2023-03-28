/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/config_enum_description_to_array.hpp"

struct Data
{
    static constexpr char const* string =
        "- name1     = 0x0001\n"
        "- name2     = 0x0002\n"
        "- long_name = 0x0100"
    ;
};

RED_AUTO_TEST_CASE(TestWidgetProfile)
{
    using namespace std::string_view_literals;
    RED_TEST_CONTEXT(Data::string) {
        constexpr auto r = config_enum_description_to_array<Data>();
        RED_CHECK_EQ_RANGES(r.names, (std::array{"name1"sv, "name2"sv, "long_name"sv, }));
        RED_CHECK_EQ_RANGES(r.bit_positions, (std::array{1, 2, 9}));
    }
}
