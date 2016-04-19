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
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL

#include "core/font.hpp"
#include "utils/log.hpp"
#include "mod/internal/widget2/flat_interactive_target.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "utils/png.hpp"
#include "system/ssl_calls.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "./"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceFlatInteractivePassword)
{
    {
        // ASK ALL (DEVICE + LOGIN + PASSWORD)
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, 800, 600, font);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          0, true, true, true, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", font, extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.rect);

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
                       "\x25\xc5\xa1\x0d\x93\x28\xd3\xb9\x57\x6a"
                       "\x81\xf1\xb8\x41\x2f\x60\xec\x9c\x20\x05"
                       )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }
    {
        // ASK DEVICE
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, 800, 600, font);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          0, true, false, false, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", font, extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.rect);

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-1.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
                       "\xe4\x52\x9f\x8b\x68\x95\x07\xe3\x76\x51"
                       "\x3e\x16\x1b\xa9\x18\x39\xd0\xa9\x54\x02"
                       )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }
    {
        // ASK PASSWORD
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, 800, 600, font);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          0, false, false, true, colors, "Target Infos",
                                          "Host", "machinetruc", "Login", "user1",
                                          "Password", font, extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.rect);

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-2.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
                       "\x93\xff\xee\x90\x7c\xa5\xdf\xb2\x96\xc9"
                       "\x5a\x45\x0c\xf0\x65\xce\x01\x31\x10\x47"
                       )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }
    {
        // ASK LOGIN + PASSWORD
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, 800, 600, font);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          0, false, true, true, colors, "Target Infos",
                                          "Host", "machinetruc", "Login", "user1",
                                          "Password", font, extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.rect);

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-3.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
                       "\x42\xd3\xf0\x98\x85\x21\x03\xc1\x52\x10"
                       "\xd9\xcc\x05\x92\xbb\x2e\x91\xf2\x66\x45"
                       )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }
    {
        // ASK DEVICE + PASSWORD
        TestDraw drawable(800, 600);

        Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

        // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
        WidgetScreen parent(drawable, 800, 600, font);
        NotifyApi * notifier = nullptr;
        Theme colors;
        colors.global.bgcolor = DARK_BLUE_BIS;
        colors.global.fgcolor = WHITE;
        WidgetFlatButton * extra_button = nullptr;
        FlatInteractiveTarget interactive(drawable, 0, 0, 800, 600, parent, notifier,
                                          0, true, false, true, colors, "Target Infos",
                                          "Host", "in 192.168.16.0/24 subnet", "Login",
                                          "user1", "Password", font, extra_button);
        // ask to widget to redraw at it's current position
        interactive.rdp_input_invalidate(interactive.rect);

        // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_target-4.png");

        char message[1024];
        if (!check_sig(drawable.gd.impl(), message,
                       "\x3d\xa8\x6b\x66\x58\x82\xb3\xa3\x19\x03"
                       "\x9c\xf8\x7c\xfc\xcd\x70\x8d\xc6\x33\xf9"
                       )){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

}
