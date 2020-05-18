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

RED_AUTO_TEST_CASE(TestLoadTheme_load_hardcoded_default_values)
{    
    Theme colors;

    RED_CHECK_EQUAL(colors.global.bgcolor, DARK_BLUE_BIS);
    RED_CHECK_EQUAL(colors.global.fgcolor, WHITE);
    RED_CHECK_EQUAL(colors.global.separator_color, LIGHT_BLUE);
    RED_CHECK_EQUAL(colors.global.focus_color, WINBLUE);
    RED_CHECK_EQUAL(colors.global.error_color, YELLOW);
    RED_CHECK(!colors.global.logo);
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
    
    ini.set<cfg::theme::bgcolor>("orange");
    ini.set<cfg::theme::fgcolor>("white");
    ini.set<cfg::theme::separator_color>("brown");
    ini.set<cfg::theme::focus_color>("dark_red");
    ini.set<cfg::theme::error_color>("red");
    ini.set<cfg::theme::logo>(true);
    ini.set<cfg::theme::logo_path>(str_concat(app_path(AppPath::Cfg),
                                              "/themes/test_theme/logo.png"));

    ini.set<cfg::theme::edit_bgcolor>("yellow");
    ini.set<cfg::theme::edit_fgcolor>("white");
    ini.set<cfg::theme::edit_focus_color>("dark_red");

    ini.set<cfg::theme::tooltip_bgcolor>("pale_blue");
    ini.set<cfg::theme::tooltip_fgcolor>("dark_blue");
    ini.set<cfg::theme::tooltip_border_color>("dark_green");

    ini.set<cfg::theme::selector_line1_bgcolor>("pale_orange");
    ini.set<cfg::theme::selector_line1_fgcolor>("white");
    
    ini.set<cfg::theme::selector_line2_bgcolor>("light_orange");
    ini.set<cfg::theme::selector_line2_fgcolor>("white");

    ini.set<cfg::theme::selector_selected_bgcolor>("medium_red");
    ini.set<cfg::theme::selector_selected_fgcolor>("black");

    ini.set<cfg::theme::selector_focus_bgcolor>("dark_red");
    ini.set<cfg::theme::selector_focus_fgcolor>("black");

    ini.set<cfg::theme::selector_label_bgcolor>("medium_red");
    ini.set<cfg::theme::selector_label_fgcolor>("black");
    
    Theme colors;

    load_theme(colors, ini);

    RED_CHECK_EQUAL(colors.global.bgcolor, DARK_BLUE_BIS);
    RED_CHECK_EQUAL(colors.global.fgcolor, WHITE);
    RED_CHECK_EQUAL(colors.global.separator_color, LIGHT_BLUE);
    RED_CHECK_EQUAL(colors.global.focus_color, WINBLUE);
    RED_CHECK_EQUAL(colors.global.error_color, YELLOW);
    RED_CHECK(!colors.global.logo);
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
    
    ini.set<cfg::theme::bgcolor>("0xDD8015");
    ini.set<cfg::theme::fgcolor>("0xffffff");
    ini.set<cfg::theme::separator_color>("0xC56A00");
    ini.set<cfg::theme::focus_color>("#0xAD1C22");
    ini.set<cfg::theme::error_color>("#ff0000");
    ini.set<cfg::theme::logo>(true);
    ini.set<cfg::theme::logo_path>(str_concat(app_path(AppPath::Cfg),
                                              "/themes/test_theme/logo.png"));

    ini.set<cfg::theme::edit_bgcolor>("WHITE");
    ini.set<cfg::theme::edit_fgcolor>("BLACK");
    ini.set<cfg::theme::edit_focus_color>("DARK_RED");

    ini.set<cfg::theme::tooltip_bgcolor>("PaLe_BlUe");
    ini.set<cfg::theme::tooltip_fgcolor>("DaRk_BlUe");
    ini.set<cfg::theme::tooltip_border_color>("DaRk_GrEeN");

    ini.set<cfg::theme::selector_line1_bgcolor>("lIgHt_oRaNgE");
    ini.set<cfg::theme::selector_line1_fgcolor>("bLaCk");
    
    ini.set<cfg::theme::selector_line2_bgcolor>("LIght_ORange");
    ini.set<cfg::theme::selector_line2_fgcolor>("BLack");

    ini.set<cfg::theme::selector_selected_bgcolor>("mediUM_rED");
    ini.set<cfg::theme::selector_selected_fgcolor>("whiTE");

    ini.set<cfg::theme::selector_focus_bgcolor>("dARk_rEd");
    ini.set<cfg::theme::selector_focus_fgcolor>("wHITe");

    ini.set<cfg::theme::selector_label_bgcolor>("medium_red");
    ini.set<cfg::theme::selector_label_fgcolor>("white");

    Theme colors;
    
    load_theme(colors, ini);
    
    RED_CHECK_EQUAL(colors.global.bgcolor, ORANGE);
    RED_CHECK_EQUAL(colors.global.fgcolor, WHITE);
    RED_CHECK_EQUAL(colors.global.separator_color, BROWN);
    RED_CHECK_EQUAL(colors.global.focus_color, DARK_RED);
    RED_CHECK_EQUAL(colors.global.error_color, RED);
    RED_CHECK(colors.global.logo);
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

RED_AUTO_TEST_CASE(TestLoadTheme_load_from_inifile_with_wrong_values)
{    
    Inifile ini;

    ini.set<cfg::theme::enable_theme>(true);
    
    ini.set<cfg::theme::bgcolor>("ABCDEFGH0xDD8015");
    ini.set<cfg::theme::fgcolor>("AC#0xffffffABCDEFGH");
    ini.set<cfg::theme::separator_color>("#0xC56A00MOP");
    ini.set<cfg::theme::focus_color>("K0xAD1C22");
    ini.set<cfg::theme::error_color>("G#0xff0000");
    ini.set<cfg::theme::logo_path>("/bad_path");

    ini.set<cfg::theme::edit_bgcolor>("WHITE123456#FF");
    ini.set<cfg::theme::edit_fgcolor>("123456BLACK");
    ini.set<cfg::theme::edit_focus_color>("DARK_123456_RED");

    ini.set<cfg::theme::tooltip_bgcolor>("PPPPPaLeEEE_BBBlUeEEEE");
    ini.set<cfg::theme::tooltip_fgcolor>("DDDDDaRkKKK_BBBBlUeeeee");
    ini.set<cfg::theme::tooltip_border_color>("DDDDDaRkKKK_GGGGrEeNNNN");

    ini.set<cfg::theme::selector_line1_bgcolor>("lIgHt_oRaNgE_lIgHt_oRaNgE");
    ini.set<cfg::theme::selector_line1_fgcolor>("bLaCk_bLaCk");
    
    ini.set<cfg::theme::selector_line2_bgcolor>("LIghtLIght_ORangeORange");
    ini.set<cfg::theme::selector_line2_fgcolor>("BLackBLack");

    ini.set<cfg::theme::selector_selected_bgcolor>("rED_mediUM_mediUM_rED");
    ini.set<cfg::theme::selector_selected_fgcolor>("#wHITe0xDD8015");

    ini.set<cfg::theme::selector_focus_bgcolor>("0xDD8015dARk_rEd");
    ini.set<cfg::theme::selector_focus_fgcolor>("");

    ini.set<cfg::theme::selector_label_bgcolor>("0x");
    ini.set<cfg::theme::selector_label_fgcolor>("#");

    Theme colors;
    
    load_theme(colors, ini);

    constexpr BGRColor null_bgr;
    
    RED_CHECK_EQUAL(colors.global.bgcolor, null_bgr);
    RED_CHECK_EQUAL(colors.global.fgcolor, null_bgr);
    RED_CHECK_EQUAL(colors.global.separator_color, null_bgr);
    RED_CHECK_EQUAL(colors.global.focus_color, null_bgr);
    RED_CHECK_EQUAL(colors.global.error_color, null_bgr);
    RED_CHECK_EQUAL(colors.global.logo_path, "/bad_path");

    RED_CHECK_EQUAL(colors.edit.bgcolor, null_bgr);
    RED_CHECK_EQUAL(colors.edit.fgcolor, null_bgr);
    RED_CHECK_EQUAL(colors.edit.focus_color, null_bgr);

    RED_CHECK_EQUAL(colors.tooltip.bgcolor, null_bgr);
    RED_CHECK_EQUAL(colors.tooltip.fgcolor, null_bgr);
    RED_CHECK_EQUAL(colors.tooltip.border_color, null_bgr);

    RED_CHECK_EQUAL(colors.selector_line1.bgcolor, null_bgr);
    RED_CHECK_EQUAL(colors.selector_line1.fgcolor, null_bgr);
    
    RED_CHECK_EQUAL(colors.selector_line2.bgcolor, null_bgr);
    RED_CHECK_EQUAL(colors.selector_line2.fgcolor, null_bgr);
    
    RED_CHECK_EQUAL(colors.selector_selected.bgcolor, null_bgr);
    RED_CHECK_EQUAL(colors.selector_selected.fgcolor, null_bgr);
    
    RED_CHECK_EQUAL(colors.selector_focus.bgcolor, null_bgr);
    RED_CHECK_EQUAL(colors.selector_focus.fgcolor, null_bgr);
    
    RED_CHECK_EQUAL(colors.selector_label.bgcolor, null_bgr);
    RED_CHECK_EQUAL(colors.selector_label.fgcolor, null_bgr);
}
