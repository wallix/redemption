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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestTheme
#include "system/redemption_unit_tests.hpp"
#include <stdio.h>

#define LOGNULL
//#define LOGPRINT

#include "utils/theme.hpp"
#include "configs/config.hpp"

BOOST_AUTO_TEST_CASE(TestColorFromFile)
{
    // test we can read from a file (and not only from a stream)
    Theme colors;

    BOOST_CHECK_EQUAL(static_cast<int>(DARK_BLUE_BIS),  colors.global.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.global.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(LIGHT_BLUE),     colors.global.separator_color);
    BOOST_CHECK_EQUAL(static_cast<int>(WINBLUE),        colors.global.focus_color);

    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.edit.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.edit.fgcolor);

    BOOST_CHECK_EQUAL(static_cast<int>(LIGHT_YELLOW),   colors.tooltip.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.tooltip.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.tooltip.border_color);

    BOOST_CHECK_EQUAL(static_cast<int>(PALE_BLUE),      colors.selector_line1.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.selector_line1.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(LIGHT_BLUE),     colors.selector_line2.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.selector_line2.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(MEDIUM_BLUE),    colors.selector_selected.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_selected.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WINBLUE),        colors.selector_focus.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_focus.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(MEDIUM_BLUE),    colors.selector_label.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_label.fgcolor);


    ThemeHolder theme_holder(colors);
    ConfigurationLoader cfg_loader(theme_holder, FIXTURES_PATH "/rdpcolor.ini");

    BOOST_CHECK_EQUAL(0xdc8115,                         colors.global.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.global.fgcolor);
    BOOST_CHECK_EQUAL(0xc46a00,                         colors.global.separator_color);
    BOOST_CHECK_EQUAL(0x9d1c22,                         colors.global.focus_color);

    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.edit.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.edit.fgcolor);

    BOOST_CHECK_EQUAL(static_cast<int>(PALE_BLUE),      colors.tooltip.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(DARK_BLUE),      colors.tooltip.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(DARK_GREEN),     colors.tooltip.border_color);

    BOOST_CHECK_EQUAL(0xffd59a,                         colors.selector_line1.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.selector_line1.fgcolor);
    BOOST_CHECK_EQUAL(0xffbf64,                         colors.selector_line2.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.selector_line2.fgcolor);
    BOOST_CHECK_EQUAL(0xb72d30,                         colors.selector_selected.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_selected.fgcolor);
    BOOST_CHECK_EQUAL(0x9d1c22,                         colors.selector_focus.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_focus.fgcolor);
    BOOST_CHECK_EQUAL(0xb72d30,                         colors.selector_label.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_label.fgcolor);
}
BOOST_AUTO_TEST_CASE(TestConfigDefault)
{
    std::stringstream   oss("");
    Theme          colors;
    ThemeHolder theme_holder(colors);
    ConfigurationLoader cfg_loader(theme_holder, oss);
    BOOST_CHECK_EQUAL(static_cast<int>(DARK_BLUE_BIS),  colors.global.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.global.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(LIGHT_BLUE),     colors.global.separator_color);
    BOOST_CHECK_EQUAL(static_cast<int>(WINBLUE),        colors.global.focus_color);

    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.edit.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.edit.fgcolor);

    BOOST_CHECK_EQUAL(static_cast<int>(LIGHT_YELLOW),   colors.tooltip.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.tooltip.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.tooltip.border_color);

    BOOST_CHECK_EQUAL(static_cast<int>(PALE_BLUE),      colors.selector_line1.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.selector_line1.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(LIGHT_BLUE),     colors.selector_line2.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.selector_line2.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(MEDIUM_BLUE),    colors.selector_selected.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_selected.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WINBLUE),        colors.selector_focus.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_focus.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(MEDIUM_BLUE),    colors.selector_label.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_label.fgcolor);

}

BOOST_AUTO_TEST_CASE(TestConfigPartial)
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
    ThemeHolder theme_holder(colors);
    ConfigurationLoader cfg_loader(theme_holder, oss);
    BOOST_CHECK_EQUAL(static_cast<int>(DARK_BLUE_BIS),  colors.global.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(GREY),           colors.global.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(RED),            colors.global.separator_color);
    BOOST_CHECK_EQUAL(0x125456,                         colors.global.focus_color);

    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.edit.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.edit.fgcolor);

    BOOST_CHECK_EQUAL(static_cast<int>(LIGHT_YELLOW),   colors.tooltip.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.tooltip.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.tooltip.border_color);

    BOOST_CHECK_EQUAL(static_cast<int>(PALE_BLUE),      colors.selector_line1.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.selector_line1.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(GREEN),          colors.selector_line2.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.selector_line2.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(MEDIUM_BLUE),    colors.selector_selected.bgcolor);
    BOOST_CHECK_EQUAL(0x005eab,                         colors.selector_selected.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WINBLUE),        colors.selector_focus.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_focus.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(MEDIUM_BLUE),    colors.selector_label.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_label.fgcolor);

}

BOOST_AUTO_TEST_CASE(TestConfigPartialFile)
{
    Theme          colors;
    ThemeHolder theme_holder(colors);
    ConfigurationLoader cfg_loader(theme_holder, CFG_PATH "/themes/test_theme/theme.ini");
    BOOST_CHECK_EQUAL(static_cast<int>(DARK_BLUE_BIS),  colors.global.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(GREY),           colors.global.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(RED),            colors.global.separator_color);
    BOOST_CHECK_EQUAL(0x125456,                         colors.global.focus_color);

    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.edit.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.edit.fgcolor);

    BOOST_CHECK_EQUAL(static_cast<int>(LIGHT_YELLOW),   colors.tooltip.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.tooltip.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.tooltip.border_color);

    BOOST_CHECK_EQUAL(static_cast<int>(PALE_BLUE),      colors.selector_line1.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.selector_line1.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(GREEN),          colors.selector_line2.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(BLACK),          colors.selector_line2.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(MEDIUM_BLUE),    colors.selector_selected.bgcolor);
    BOOST_CHECK_EQUAL(0x005eab,                         colors.selector_selected.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WINBLUE),        colors.selector_focus.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_focus.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(MEDIUM_BLUE),    colors.selector_label.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.selector_label.fgcolor);

}

BOOST_AUTO_TEST_CASE(TestConfigPartialIni)
{
    Inifile ini;
    Theme const & colors = ini.get<cfg::theme>();

    BOOST_CHECK_EQUAL(static_cast<int>(DARK_BLUE_BIS),  colors.global.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(WHITE),          colors.global.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(LIGHT_BLUE),     colors.global.separator_color);
    BOOST_CHECK_EQUAL(static_cast<int>(WINBLUE),        colors.global.focus_color);

    ini.set<cfg::internal_mod::theme>("test_theme");

    BOOST_CHECK_EQUAL(static_cast<int>(DARK_BLUE_BIS),  colors.global.bgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(GREY),           colors.global.fgcolor);
    BOOST_CHECK_EQUAL(static_cast<int>(RED),            colors.global.separator_color);
    BOOST_CHECK_EQUAL(0x125456,                         colors.global.focus_color);

    BOOST_CHECK_EQUAL(true,                             colors.global.logo);
    BOOST_CHECK_EQUAL(CFG_PATH "/themes/test_theme/logo.png", colors.global.logo_path);
}
