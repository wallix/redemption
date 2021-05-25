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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Jonathan Poelen, Meng Tan, Raphael Zhou,
 *              Jennifer Inthavong
 *
 */

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"


#include "mod/internal/widget/interactive_target.hpp"
#include "mod/internal/widget/screen.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/interactive_target/"

RED_AUTO_TEST_CASE(TraceWidgetInteractivePassword)
{
    {
        // ASK ALL (DEVICE + LOGIN + PASSWORD)
        TestGraphic drawable(800, 600);

        // WidgetDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetButton * extra_button = nullptr;
        WidgetInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          true, true, true, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", global_font_lato_light_16(), extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        RED_CHECK_IMG(drawable, IMG_TEST_PATH "interactive_target_1.png");
    }
    {
        // ASK DEVICE
        TestGraphic drawable(800, 600);

        // WidgetDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetButton * extra_button = nullptr;
        WidgetInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          true, false, false, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", global_font_lato_light_16(), extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        RED_CHECK_IMG(drawable, IMG_TEST_PATH "interactive_target_2.png");
    }
    {
        // ASK PASSWORD
        TestGraphic drawable(800, 600);

        // WidgetDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetButton * extra_button = nullptr;
        WidgetInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          false, false, true, colors, "Target Infos",
                                          "Host", "machinetruc", "Login", "user1",
                                          "Password", global_font_lato_light_16(), extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        RED_CHECK_IMG(drawable, IMG_TEST_PATH "interactive_target_3.png");
    }
    {
        // ASK LOGIN + PASSWORD
        TestGraphic drawable(800, 600);

        // WidgetDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetButton * extra_button = nullptr;
        WidgetInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          false, true, true, colors, "Target Infos",
                                          "Host", "machinetruc", "Login", "user1",
                                          "Password", global_font_lato_light_16(), extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        RED_CHECK_IMG(drawable, IMG_TEST_PATH "interactive_target_4.png");
    }
    {
        // ASK DEVICE + PASSWORD
        TestGraphic drawable(800, 600);

        // WidgetDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetButton * extra_button = nullptr;
        WidgetInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          true, false, true, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", global_font_lato_light_16(), extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        RED_CHECK_IMG(drawable, IMG_TEST_PATH "interactive_target_5.png");
    }
}
