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
  Copyright (C) Wallix 2012-2014
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#define RED_TEST_MODULE TestTheme
#include "system/redemption_unit_tests.hpp"
#include <stdio.h>


#include "utils/theme.hpp"
#include "configs/config.hpp"

RED_AUTO_TEST_CASE(TestColorFromFile)
{
    // test we can read from a file (and not only from a stream)
    Theme colors;

    RED_CHECK_EQUAL((DARK_BLUE_BIS),  colors.global.bgcolor);
    RED_CHECK_EQUAL((WHITE),          colors.global.fgcolor);
    RED_CHECK_EQUAL((LIGHT_BLUE),     colors.global.separator_color);
    RED_CHECK_EQUAL((WINBLUE),        colors.global.focus_color);

    RED_CHECK_EQUAL((WHITE),          colors.edit.bgcolor);
    RED_CHECK_EQUAL((BLACK),          colors.edit.fgcolor);

    RED_CHECK_EQUAL((LIGHT_YELLOW),   colors.tooltip.bgcolor);
    RED_CHECK_EQUAL((BLACK),          colors.tooltip.fgcolor);
    RED_CHECK_EQUAL((BLACK),          colors.tooltip.border_color);

    RED_CHECK_EQUAL((PALE_BLUE),      colors.selector_line1.bgcolor);
    RED_CHECK_EQUAL((BLACK),          colors.selector_line1.fgcolor);
    RED_CHECK_EQUAL((LIGHT_BLUE),     colors.selector_line2.bgcolor);
    RED_CHECK_EQUAL((BLACK),          colors.selector_line2.fgcolor);
    RED_CHECK_EQUAL((MEDIUM_BLUE),    colors.selector_selected.bgcolor);
    RED_CHECK_EQUAL((WHITE),          colors.selector_selected.fgcolor);
    RED_CHECK_EQUAL((WINBLUE),        colors.selector_focus.bgcolor);
    RED_CHECK_EQUAL((WHITE),          colors.selector_focus.fgcolor);
    RED_CHECK_EQUAL((MEDIUM_BLUE),    colors.selector_label.bgcolor);
    RED_CHECK_EQUAL((WHITE),          colors.selector_label.fgcolor);


    configuration_load(ThemeHolder(colors), FIXTURES_PATH "/rdpcolor.ini");

    RED_CHECK_EQUAL(BGRColor(0xdc8115),  colors.global.bgcolor);
    RED_CHECK_EQUAL((WHITE),              colors.global.fgcolor);
    RED_CHECK_EQUAL(BGRColor(0xc46a00),  colors.global.separator_color);
    RED_CHECK_EQUAL(BGRColor(0x9d1c22),  colors.global.focus_color);

    RED_CHECK_EQUAL((WHITE),              colors.edit.bgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.edit.fgcolor);

    RED_CHECK_EQUAL((PALE_BLUE),          colors.tooltip.bgcolor);
    RED_CHECK_EQUAL((DARK_BLUE),          colors.tooltip.fgcolor);
    RED_CHECK_EQUAL((DARK_GREEN),         colors.tooltip.border_color);

    RED_CHECK_EQUAL(BGRColor(0xffd59a),  colors.selector_line1.bgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.selector_line1.fgcolor);
    RED_CHECK_EQUAL(BGRColor(0xffbf64),  colors.selector_line2.bgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.selector_line2.fgcolor);
    RED_CHECK_EQUAL(BGRColor(0xb72d30),  colors.selector_selected.bgcolor);
    RED_CHECK_EQUAL((WHITE),              colors.selector_selected.fgcolor);
    RED_CHECK_EQUAL(BGRColor(0x9d1c22),  colors.selector_focus.bgcolor);
    RED_CHECK_EQUAL((WHITE),              colors.selector_focus.fgcolor);
    RED_CHECK_EQUAL(BGRColor(0xb72d30),  colors.selector_label.bgcolor);
    RED_CHECK_EQUAL((WHITE),              colors.selector_label.fgcolor);
}

RED_AUTO_TEST_CASE(TestConfigDefault)
{
    std::stringstream   oss("");
    Theme          colors;
    configuration_load(ThemeHolder(colors), oss);

    RED_CHECK_EQUAL((DARK_BLUE_BIS),  colors.global.bgcolor);
    RED_CHECK_EQUAL((WHITE),          colors.global.fgcolor);
    RED_CHECK_EQUAL((LIGHT_BLUE),     colors.global.separator_color);
    RED_CHECK_EQUAL((WINBLUE),        colors.global.focus_color);

    RED_CHECK_EQUAL((WHITE),          colors.edit.bgcolor);
    RED_CHECK_EQUAL((BLACK),          colors.edit.fgcolor);

    RED_CHECK_EQUAL((LIGHT_YELLOW),   colors.tooltip.bgcolor);
    RED_CHECK_EQUAL((BLACK),          colors.tooltip.fgcolor);
    RED_CHECK_EQUAL((BLACK),          colors.tooltip.border_color);

    RED_CHECK_EQUAL((PALE_BLUE),      colors.selector_line1.bgcolor);
    RED_CHECK_EQUAL((BLACK),          colors.selector_line1.fgcolor);
    RED_CHECK_EQUAL((LIGHT_BLUE),     colors.selector_line2.bgcolor);
    RED_CHECK_EQUAL((BLACK),          colors.selector_line2.fgcolor);
    RED_CHECK_EQUAL((MEDIUM_BLUE),    colors.selector_selected.bgcolor);
    RED_CHECK_EQUAL((WHITE),          colors.selector_selected.fgcolor);
    RED_CHECK_EQUAL((WINBLUE),        colors.selector_focus.bgcolor);
    RED_CHECK_EQUAL((WHITE),          colors.selector_focus.fgcolor);
    RED_CHECK_EQUAL((MEDIUM_BLUE),    colors.selector_label.bgcolor);
    RED_CHECK_EQUAL((WHITE),          colors.selector_label.fgcolor);
}

RED_AUTO_TEST_CASE(TestConfigPartial)
{
    std::stringstream   oss(
                            "[global]\n"
                            "fgcolor=grey\n"
                            "separator_color=RED\n"
                            "focus_color=0x565412\n"
                            "\n"
                            "\n"
                            "[selector]\n"
                            "line2_bgcolor=grEEn\n"
                            "selected_fgcolor=0xab5e00\n"
                            "\n"
                            );
    Theme          colors;

    configuration_load(ThemeHolder(colors), oss);
    RED_CHECK_EQUAL((DARK_BLUE_BIS),      colors.global.bgcolor);
    RED_CHECK_EQUAL((GREY),               colors.global.fgcolor);
    RED_CHECK_EQUAL((RED),                colors.global.separator_color);
    RED_CHECK_EQUAL(BGRColor(0x125456),  colors.global.focus_color);

    RED_CHECK_EQUAL((WHITE),              colors.edit.bgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.edit.fgcolor);

    RED_CHECK_EQUAL((LIGHT_YELLOW),       colors.tooltip.bgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.tooltip.fgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.tooltip.border_color);

    RED_CHECK_EQUAL((PALE_BLUE),          colors.selector_line1.bgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.selector_line1.fgcolor);
    RED_CHECK_EQUAL((GREEN),              colors.selector_line2.bgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.selector_line2.fgcolor);
    RED_CHECK_EQUAL((MEDIUM_BLUE),        colors.selector_selected.bgcolor);
    RED_CHECK_EQUAL(BGRColor(0x005eab),  colors.selector_selected.fgcolor);
    RED_CHECK_EQUAL((WINBLUE),            colors.selector_focus.bgcolor);
    RED_CHECK_EQUAL((WHITE),              colors.selector_focus.fgcolor);
    RED_CHECK_EQUAL((MEDIUM_BLUE),        colors.selector_label.bgcolor);
    RED_CHECK_EQUAL((WHITE),              colors.selector_label.fgcolor);
}

RED_AUTO_TEST_CASE(TestConfigPartialFile)
{
    Theme          colors;

    configuration_load(ThemeHolder(colors), app_path_s(AppPath::Cfg) + "/themes/test_theme/theme.ini");
    RED_CHECK_EQUAL((DARK_BLUE_BIS),      colors.global.bgcolor);
    RED_CHECK_EQUAL((GREY),               colors.global.fgcolor);
    RED_CHECK_EQUAL((RED),                colors.global.separator_color);
    RED_CHECK_EQUAL(BGRColor(0x125456),  colors.global.focus_color);

    RED_CHECK_EQUAL((WHITE),              colors.edit.bgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.edit.fgcolor);

    RED_CHECK_EQUAL((LIGHT_YELLOW),       colors.tooltip.bgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.tooltip.fgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.tooltip.border_color);

    RED_CHECK_EQUAL((PALE_BLUE),          colors.selector_line1.bgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.selector_line1.fgcolor);
    RED_CHECK_EQUAL((GREEN),              colors.selector_line2.bgcolor);
    RED_CHECK_EQUAL((BLACK),              colors.selector_line2.fgcolor);
    RED_CHECK_EQUAL((MEDIUM_BLUE),        colors.selector_selected.bgcolor);
    RED_CHECK_EQUAL(BGRColor(0x005eab),  colors.selector_selected.fgcolor);
    RED_CHECK_EQUAL((WINBLUE),            colors.selector_focus.bgcolor);
    RED_CHECK_EQUAL((WHITE),              colors.selector_focus.fgcolor);
    RED_CHECK_EQUAL((MEDIUM_BLUE),        colors.selector_label.bgcolor);
    RED_CHECK_EQUAL((WHITE),              colors.selector_label.fgcolor);
}

RED_AUTO_TEST_CASE(TestConfigPartialIni)
{
    Inifile ini;
    Theme const & colors = ini.get<cfg::theme>();

    RED_CHECK_EQUAL((DARK_BLUE_BIS),  colors.global.bgcolor);
    RED_CHECK_EQUAL((WHITE),          colors.global.fgcolor);
    RED_CHECK_EQUAL((LIGHT_BLUE),     colors.global.separator_color);
    RED_CHECK_EQUAL((WINBLUE),        colors.global.focus_color);

    ini.set<cfg::internal_mod::theme>("test_theme");

    RED_CHECK_EQUAL((DARK_BLUE_BIS),     colors.global.bgcolor);
    RED_CHECK_EQUAL((GREY),              colors.global.fgcolor);
    RED_CHECK_EQUAL((RED),               colors.global.separator_color);
    RED_CHECK_EQUAL(BGRColor(0x125456),  colors.global.focus_color);

    RED_CHECK_EQUAL(true,                colors.global.logo);
    RED_CHECK_EQUAL(app_path(AppPath::Cfg) + std::string("/themes/test_theme/logo.png"), colors.global.logo_path);
}
