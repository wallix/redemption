/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "headlessclient/headless_configuration.hpp"
#include "core/client_info.hpp"
#include "configs/config.hpp"

RED_AUTO_TEST_CASE(TestHeadlessConfiguration)
{
    ClientInfo client_info;
    Inifile ini;

    client_info.glyph_cache_caps.GlyphCache[0].CacheEntries = 10;
    char str[] = "[client_info] glyph_cache[0].entries = 5";
    load_headless_config_from_string(ini, client_info, str);
    RED_CHECK(client_info.glyph_cache_caps.GlyphCache[0].CacheEntries == 5);

    RED_CHECK(headless_client_info_config_as_string(client_info).find("glyph_cache[0].entries = 5\n") != std::string::npos);
}
