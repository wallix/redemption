/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Meng Tan, Raphael Zhou
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetScroll
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "internal/widget2/scroll.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/composite.hpp"
#include "internal/widget2/image.hpp"

#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"
#include "fake_draw.hpp"

#ifndef FIXTURES_PATH
#define FIXTURES_PATH
#endif

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "./"

BOOST_AUTO_TEST_CASE(TraceWidgetFrame)
{
    TestDraw drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier = NULL;
    // int         fg_color = RED;
    // int         bg_color = YELLOW;
    int         group_id = 0;
    int16_t     x        = 0;
    int16_t     y        = 0;
    uint16_t    cx       = 400;
    uint16_t    cy       = 300;

    WidgetFrame frame(drawable, Rect(x, y, cx, cy), parent, notifier, group_id);
    // WidgetImage wimage(drawable, 0, 0, FIXTURES_PATH"/Philips_PM5544_192.bmp", parent, notifier);
    // WidgetImage wimage(drawable, 0, 0, FIXTURES_PATH"/color_image.png", parent, notifier);
    WidgetImage wimage(drawable, 0, 0, FIXTURES_PATH"/win2008capture10.png", parent, notifier);

    frame.set_widget(&wimage);

    // ask to widget to redraw at it's current position
    frame.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "frame1.png");

    char message[1024];
    if (!check_sig( drawable.gd.drawable, message,
                    "\xf8\x30\xb3\xcc\xe7\xe6\x7d\x5e\x05\x91"
                    "\x94\x47\x57\xb6\xcf\x98\xff\xb0\xcb\xb1")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_vertical_widget_pos(50);
    // ask to widget to redraw at it's current position
    frame.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "frame2.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\xb5\x72\xb8\x01\x34\x0d\xed\x89\x70\x9a"
                    "\xfc\xad\x28\xe0\x60\x62\xc9\xbd\x72\x08")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
    frame.set_horizontal_widget_pos(70);
    // ask to widget to redraw at it's current position
    frame.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "frame3.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x1e\x7e\xd6\xfd\x56\xb0\x20\xab\x86\x2a"
                    "\x56\xbf\xe4\xe2\xb2\x96\x3d\x5d\x41\x5b")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_vertical_widget_pos(0);
    frame.set_horizontal_widget_pos(100);
    // ask to widget to redraw at it's current position
    frame.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "frame4.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x12\x05\xf5\x72\x72\x8e\xcd\x45\xf7\xdd"
                    "\x08\xa7\x73\x53\xad\x87\x60\x24\x7c\xa4")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_vertical_widget_pos(100);
    frame.rdp_input_invalidate(parent.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "frame5.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x11\x47\x9e\xb4\xc6\x81\xe5\xfe\x35\x16"
                    "\xd7\x02\xa3\x01\x2b\x65\x17\x70\x83\x63")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
    frame.set_horizontal_widget_pos(20);
    frame.rdp_input_invalidate(parent.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "frame6.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x2c\xb3\xac\x67\x1a\xcc\xac\x6c\x16\x75"
                    "\x55\xa0\x4d\x66\x4c\x80\x89\xa5\xce\x76")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_horizontal_widget_pos(0);
    frame.set_vertical_widget_pos(95);
    frame.rdp_input_invalidate(parent.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "frame7.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x91\x21\x67\xa4\xc6\x04\xc0\x62\xa2\x0c"
                    "\xcd\x32\xfa\xc1\x88\x22\x27\x06\x5b\x59")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
}
