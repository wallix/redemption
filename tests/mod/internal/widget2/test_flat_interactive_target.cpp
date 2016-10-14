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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFlatInteractiveTarget
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/font.hpp"
#include "utils/log.hpp"
#include "mod/internal/widget2/flat_interactive_target.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "utils/png.hpp"
#include "system/ssl_calls.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceFlatInteractivePassword)
{
    {
        // ASK ALL (DEVICE + LOGIN + PASSWORD)
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
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
        interactive.rdp_input_invalidate(interactive.rect);

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
            "\x5b\x12\x73\x34\x4c\x7e\xb3\xd2\xa8\x06\xb1\x03\xe9\x8f\xf7\xcf\x24\x40\x90\x62"
        )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }
    {
        // ASK DEVICE
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
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
        interactive.rdp_input_invalidate(interactive.rect);

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-1.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
            "\x3e\x8c\x9b\x27\x32\x3a\xb9\xe5\xdd\x91\xf2\xdf\x46\x6a\x0f\x3c\xa7\xfd\xea\x3b"
        )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }
    {
        // ASK PASSWORD
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
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
        interactive.rdp_input_invalidate(interactive.rect);

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-2.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
            "\xbb\x37\x56\x8c\x63\x4d\xad\xab\xfd\x47\x25\xed\x10\x9b\x05\x0b\x2f\x3b\x8f\xcf"
       )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }
    {
        // ASK LOGIN + PASSWORD
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
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
        interactive.rdp_input_invalidate(interactive.rect);

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-3.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
            "\x64\x96\x0a\xf7\xce\x87\xcf\xf8\x80\x88\xaa\x9d\xa1\x28\xf9\xe6\x50\x63\x1f\xe5"
       )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }
    {
        // ASK DEVICE + PASSWORD
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
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
        interactive.rdp_input_invalidate(interactive.rect);

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-4.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
            "\x6a\xb7\xdb\x5a\x9d\xfd\xed\xb5\xa1\x77\x1b\xfe\x2e\xee\xe7\xd6\xec\x2d\xb4\xa2"
                       )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

}
