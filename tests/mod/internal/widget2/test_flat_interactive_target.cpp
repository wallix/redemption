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
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
            "\x3a\x52\x44\x57\x26\xe9\x5d\x16\xbe\x43\x52\xd1\x9d\x6b\x71\x9e\x1c\xa0\xb7\x79"
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
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-1.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
            "\xb5\xce\x4e\xca\x2d\x3d\x41\x54\x51\x9d\x8a\x62\x4e\x1a\x6d\x31\xa0\x98\x31\x0c"
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
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-2.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
            "\x58\xdd\x6d\x5e\x91\xd1\x45\x81\x08\x35\x05\x89\x05\xce\x70\xff\xf0\x26\xfb\xf8"
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
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-3.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
            "\x85\xdd\x9e\x08\x09\xcd\xe4\x3f\x4c\xcc\xff\x9f\x88\xe3\x51\xdf\x2f\x4c\xbd\x1f"
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
        interactive.rdp_input_invalidate(interactive.get_rect());

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-4.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
            "\xb7\xb4\x21\x96\x88\xd7\xae\xd2\x34\x16\x73\xb0\x9d\xfb\x7f\xbc\x9e\x4d\xd8\x65"
                       )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

}
