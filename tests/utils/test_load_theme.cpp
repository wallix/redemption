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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "configs/config.hpp"
#include "utils/theme.hpp"
#include "utils/load_theme.hpp"
#include "utils/strutils.hpp"

namespace
{
    using Rgb = ::configs::spec_types::rgb;

    inline Rgb to_rgb(NamedBGRColor color)
    {
        return Rgb(BGRColor(BGRasRGBColor(color)).as_u32());
    }
}

RED_AUTO_TEST_CASE(TestLoadTheme_load_hardcoded_default_values)
{
    Theme colors;

    RED_CHECK(!colors.global.enable_theme);
    RED_CHECK_EQUAL(colors.global.bgcolor, DARK_BLUE_BIS);
    RED_CHECK_EQUAL(colors.global.fgcolor, WHITE);
    RED_CHECK_EQUAL(colors.global.separator_color, LIGHT_BLUE);
    RED_CHECK_EQUAL(colors.global.focus_color, WINBLUE);
    RED_CHECK_EQUAL(colors.global.error_color, YELLOW);
    RED_CHECK_EQUAL(colors.global.logo_path, "");

    RED_CHECK_EQUAL(colors.edit.bgcolor, WHITE);
    RED_CHECK_EQUAL(colors.edit.fgcolor, BLACK);
    RED_CHECK_EQUAL(colors.edit.focus_color, WINBLUE);

    RED_CHECK_EQUAL(colors.tooltip.bgcolor, LIGHT_YELLOW);
    RED_CHECK_EQUAL(colors.tooltip.fgcolor, BLACK);
    RED_CHECK_EQUAL(colors.tooltip.border_color, BLACK);

    RED_CHECK_EQUAL(colors.selector_line1.bgcolor, PALE_BLUE);
    RED_CHECK_EQUAL(colors.selector_line1.fgcolor, BLACK);

    RED_CHECK_EQUAL(colors.selector_line2.bgcolor, LIGHT_BLUE);
    RED_CHECK_EQUAL(colors.selector_line2.fgcolor, BLACK);

    RED_CHECK_EQUAL(colors.selector_selected.bgcolor, MEDIUM_BLUE);
    RED_CHECK_EQUAL(colors.selector_selected.fgcolor, WHITE);

    RED_CHECK_EQUAL(colors.selector_focus.bgcolor, WINBLUE);
    RED_CHECK_EQUAL(colors.selector_focus.fgcolor, WHITE);

    RED_CHECK_EQUAL(colors.selector_label.bgcolor, MEDIUM_BLUE);
    RED_CHECK_EQUAL(colors.selector_label.fgcolor, WHITE);
}

RED_AUTO_TEST_CASE(TestLoadTheme_load_hardcoded_default_values_even_if_inifile_is_set)
{
    Inifile ini;

    ini.set<cfg::theme::enable_theme>(false);
    ini.set<cfg::theme::bgcolor>(to_rgb(ORANGE));
    ini.set<cfg::theme::fgcolor>(to_rgb(WHITE));
    ini.set<cfg::theme::separator_color>(to_rgb(BROWN));
    ini.set<cfg::theme::focus_color>(to_rgb(DARK_RED));
    ini.set<cfg::theme::error_color>(to_rgb(RED));
    ini.set<cfg::theme::logo_path>(str_concat(app_path(AppPath::Cfg),
                                              "/themes/test_theme/logo.png"));

    ini.set<cfg::theme::edit_bgcolor>(to_rgb(YELLOW));
    ini.set<cfg::theme::edit_fgcolor>(to_rgb(WHITE));
    ini.set<cfg::theme::edit_focus_color>(to_rgb(DARK_RED));

    ini.set<cfg::theme::tooltip_bgcolor>(to_rgb(PALE_BLUE));
    ini.set<cfg::theme::tooltip_fgcolor>(to_rgb(DARK_BLUE));
    ini.set<cfg::theme::tooltip_border_color>(to_rgb(DARK_GREEN));

    ini.set<cfg::theme::selector_line1_bgcolor>(to_rgb(PALE_ORANGE));
    ini.set<cfg::theme::selector_line1_fgcolor>(to_rgb(WHITE));

    ini.set<cfg::theme::selector_line2_bgcolor>(to_rgb(LIGHT_ORANGE));
    ini.set<cfg::theme::selector_line2_fgcolor>(to_rgb(WHITE));

    ini.set<cfg::theme::selector_selected_bgcolor>(to_rgb(MEDIUM_RED));
    ini.set<cfg::theme::selector_selected_fgcolor>(to_rgb(BLACK));

    ini.set<cfg::theme::selector_focus_bgcolor>(to_rgb(DARK_RED));
    ini.set<cfg::theme::selector_focus_fgcolor>(to_rgb(BLACK));

    ini.set<cfg::theme::selector_label_bgcolor>(to_rgb(MEDIUM_RED));
    ini.set<cfg::theme::selector_label_fgcolor>(to_rgb(BLACK));

    Theme colors;

    load_theme(colors, ini);

    RED_CHECK(!colors.global.enable_theme);
    RED_CHECK_EQUAL(colors.global.bgcolor, DARK_BLUE_BIS);
    RED_CHECK_EQUAL(colors.global.fgcolor, WHITE);
    RED_CHECK_EQUAL(colors.global.separator_color, LIGHT_BLUE);
    RED_CHECK_EQUAL(colors.global.focus_color, WINBLUE);
    RED_CHECK_EQUAL(colors.global.error_color, YELLOW);
    RED_CHECK_EQUAL(colors.global.logo_path, "");

    RED_CHECK_EQUAL(colors.edit.bgcolor, WHITE);
    RED_CHECK_EQUAL(colors.edit.fgcolor, BLACK);
    RED_CHECK_EQUAL(colors.edit.focus_color, WINBLUE);

    RED_CHECK_EQUAL(colors.tooltip.bgcolor, LIGHT_YELLOW);
    RED_CHECK_EQUAL(colors.tooltip.fgcolor, BLACK);
    RED_CHECK_EQUAL(colors.tooltip.border_color, BLACK);

    RED_CHECK_EQUAL(colors.selector_line1.bgcolor, PALE_BLUE);
    RED_CHECK_EQUAL(colors.selector_line1.fgcolor, BLACK);

    RED_CHECK_EQUAL(colors.selector_line2.bgcolor, LIGHT_BLUE);
    RED_CHECK_EQUAL(colors.selector_line2.fgcolor, BLACK);

    RED_CHECK_EQUAL(colors.selector_selected.bgcolor, MEDIUM_BLUE);
    RED_CHECK_EQUAL(colors.selector_selected.fgcolor, WHITE);

    RED_CHECK_EQUAL(colors.selector_focus.bgcolor, WINBLUE);
    RED_CHECK_EQUAL(colors.selector_focus.fgcolor, WHITE);

    RED_CHECK_EQUAL(colors.selector_label.bgcolor, MEDIUM_BLUE);
    RED_CHECK_EQUAL(colors.selector_label.fgcolor, WHITE);
}

RED_AUTO_TEST_CASE(TestLoadTheme_load_from_inifile)
{
    Inifile ini;

    ini.set<cfg::theme::enable_theme>(true);
    ini.set<cfg::theme::bgcolor>(Rgb(0xDD8015));
    ini.set<cfg::theme::fgcolor>(Rgb(0xffffff));
    ini.set<cfg::theme::separator_color>(Rgb(0xC56A00));
    ini.set<cfg::theme::focus_color>(Rgb(0xAD1C22));
    ini.set<cfg::theme::error_color>(Rgb(0xff0000));
    ini.set<cfg::theme::logo_path>(str_concat(app_path(AppPath::Cfg),
                                              "/themes/test_theme/logo.png"));

    ini.set<cfg::theme::edit_bgcolor>(to_rgb(WHITE));
    ini.set<cfg::theme::edit_fgcolor>(to_rgb(BLACK));
    ini.set<cfg::theme::edit_focus_color>(to_rgb(DARK_RED));

    ini.set<cfg::theme::tooltip_bgcolor>(to_rgb(PALE_BLUE));
    ini.set<cfg::theme::tooltip_fgcolor>(to_rgb(DARK_BLUE));
    ini.set<cfg::theme::tooltip_border_color>(to_rgb(DARK_GREEN));

    ini.set<cfg::theme::selector_line1_bgcolor>(to_rgb(LIGHT_ORANGE));
    ini.set<cfg::theme::selector_line1_fgcolor>(to_rgb(BLACK));

    ini.set<cfg::theme::selector_line2_bgcolor>(to_rgb(LIGHT_ORANGE));
    ini.set<cfg::theme::selector_line2_fgcolor>(to_rgb(BLACK));

    ini.set<cfg::theme::selector_selected_bgcolor>(to_rgb(MEDIUM_RED));
    ini.set<cfg::theme::selector_selected_fgcolor>(to_rgb(WHITE));

    ini.set<cfg::theme::selector_focus_bgcolor>(to_rgb(DARK_RED));
    ini.set<cfg::theme::selector_focus_fgcolor>(to_rgb(WHITE));

    ini.set<cfg::theme::selector_label_bgcolor>(to_rgb(MEDIUM_RED));
    ini.set<cfg::theme::selector_label_fgcolor>(to_rgb(WHITE));

    Theme colors;

    load_theme(colors, ini);

    RED_CHECK(colors.global.enable_theme);
    RED_CHECK_EQUAL(colors.global.bgcolor, ORANGE);
    RED_CHECK_EQUAL(colors.global.fgcolor, WHITE);
    RED_CHECK_EQUAL(colors.global.separator_color, BROWN);
    RED_CHECK_EQUAL(colors.global.focus_color, DARK_RED);
    RED_CHECK_EQUAL(colors.global.error_color, RED);
    RED_CHECK_EQUAL(colors.global.logo_path,
                    str_concat(app_path(AppPath::Cfg),
                               "/themes/test_theme/logo.png"));

    RED_CHECK_EQUAL(colors.edit.bgcolor, WHITE);
    RED_CHECK_EQUAL(colors.edit.fgcolor, BLACK);
    RED_CHECK_EQUAL(colors.edit.focus_color, DARK_RED);

    RED_CHECK_EQUAL(colors.tooltip.bgcolor, PALE_BLUE);
    RED_CHECK_EQUAL(colors.tooltip.fgcolor, DARK_BLUE);
    RED_CHECK_EQUAL(colors.tooltip.border_color, DARK_GREEN);

    RED_CHECK_EQUAL(colors.selector_line1.bgcolor, LIGHT_ORANGE);
    RED_CHECK_EQUAL(colors.selector_line1.fgcolor, BLACK);

    RED_CHECK_EQUAL(colors.selector_line2.bgcolor, LIGHT_ORANGE);
    RED_CHECK_EQUAL(colors.selector_line2.fgcolor, BLACK);

    RED_CHECK_EQUAL(colors.selector_selected.bgcolor, MEDIUM_RED);
    RED_CHECK_EQUAL(colors.selector_selected.fgcolor, WHITE);

    RED_CHECK_EQUAL(colors.selector_focus.bgcolor, DARK_RED);
    RED_CHECK_EQUAL(colors.selector_focus.fgcolor, WHITE);

    RED_CHECK_EQUAL(colors.selector_label.bgcolor, MEDIUM_RED);
    RED_CHECK_EQUAL(colors.selector_label.fgcolor, WHITE);
}
