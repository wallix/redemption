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

#define RED_TEST_MODULE TestFlatInteractiveTarget
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/flat_interactive_target.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


RED_AUTO_TEST_CASE(TraceFlatInteractivePassword)
{
    {
        // ASK ALL (DEVICE + LOGIN + PASSWORD)
        TestGraphic drawable(800, 600);

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
        parent.set_wh(800, 600);

        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          true, true, true, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", global_font_lato_light_16(), extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png("interactive_target.png");

        RED_CHECK_SIG(drawable, "\x27\xac\x46\x35\xd5\x1c\x3e\xbc\x64\xd5\xe7\x22\x35\x71\x51\x93\x04\x4d\x26\x1e");
    }
    {
        // ASK DEVICE
        TestGraphic drawable(800, 600);

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
        parent.set_wh(800, 600);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          true, false, false, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", global_font_lato_light_16(), extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png("interactive_target-1.png");

        RED_CHECK_SIG(drawable, "\x13\xcc\x44\x3d\xa9\xcb\x5d\xd4\xe6\x4c\x3c\x45\x38\xd6\x45\xbf\x55\x8c\x12\x5f");
    }
    {
        // ASK PASSWORD
        TestGraphic drawable(800, 600);

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
        parent.set_wh(800, 600);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          false, false, true, colors, "Target Infos",
                                          "Host", "machinetruc", "Login", "user1",
                                          "Password", global_font_lato_light_16(), extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png("interactive_target-2.png");

        RED_CHECK_SIG(drawable, "\x54\x66\x24\xdc\x10\x61\x14\x04\x9a\xc7\x79\xe0\xbb\x1e\xed\x2d\x98\x07\x20\x21");
    }
    {
        // ASK LOGIN + PASSWORD
        TestGraphic drawable(800, 600);

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
        parent.set_wh(800, 600);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          false, true, true, colors, "Target Infos",
                                          "Host", "machinetruc", "Login", "user1",
                                          "Password", global_font_lato_light_16(), extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png("interactive_target-3.png");

        RED_CHECK_SIG(drawable, "\xd2\x35\xde\x65\xfd\xdc\x48\x32\x9e\x28\xf4\x79\x05\x7f\xda\xb1\x1b\x7d\x55\xf7");
    }
    {
        // ASK DEVICE + PASSWORD
        TestGraphic drawable(800, 600);

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
        parent.set_wh(800, 600);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          true, false, true, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", global_font_lato_light_16(), extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png("interactive_target-4.png");

        RED_CHECK_SIG(drawable, "\x7e\x0d\xb7\xe9\x4d\xb7\xf7\x2b\x9f\x80\x3b\x5f\x8a\x99\x9f\xe5\x26\x94\xfb\x92");
    }
}
