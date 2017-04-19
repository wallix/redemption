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
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/font.hpp"
#include "utils/log.hpp"
#include "mod/internal/widget2/flat_interactive_target.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "utils/png.hpp"
#include "system/ssl_calls.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceFlatInteractivePassword)
{
    {
        // ASK ALL (DEVICE + LOGIN + PASSWORD)
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
        parent.set_wh(800, 600);

        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                          true, true, true, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", font, extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target.png");

        RED_CHECK_SIG(drawable.gd, "\xaa\xa8\x12\x8e\xa4\x6b\xfc\xaf\xc8\x0b\xdc\xe0\xf7\x99\x87\xf6\x7e\x58\x19\x45");
    }
    {
        // ASK DEVICE
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
        parent.set_wh(800, 600);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                          true, false, false, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", font, extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-1.png");

        RED_CHECK_SIG(drawable.gd, "\xe9\x5b\xae\xe5\xec\x96\xcb\x00\xd1\x5a\x21\x25\xc0\xa7\xb9\xf4\xbd\x20\x99\xdc");
    }
    {
        // ASK PASSWORD
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
        parent.set_wh(800, 600);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                          false, false, true, colors, "Target Infos",
                                          "Host", "machinetruc", "Login", "user1",
                                          "Password", font, extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-2.png");

        RED_CHECK_SIG(drawable.gd, "\xcb\xcc\x5c\x60\x2a\x42\x5d\x88\x35\x7b\x58\x25\x80\x46\x5c\x7f\x88\x67\x3b\x28");
    }
    {
        // ASK LOGIN + PASSWORD
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
        parent.set_wh(800, 600);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                          false, true, true, colors, "Target Infos",
                                          "Host", "machinetruc", "Login", "user1",
                                          "Password", font, extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-3.png");

        RED_CHECK_SIG(drawable.gd, "\xf8\xdc\xd6\x82\x09\x02\xb1\x0b\xcd\x61\x49\x28\x90\x41\xa8\xd6\x8f\xad\xd0\x21");
    }
    {
        // ASK DEVICE + PASSWORD
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
        parent.set_wh(800, 600);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                          true, false, true, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", font, extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-4.png");

        RED_CHECK_SIG(drawable.gd, "\xca\x85\x6d\x9d\xb1\x80\xb8\xd2\xe9\x59\x88\xb6\xa0\x95\x20\xaf\xff\x96\xef\xd9");
    }

}
