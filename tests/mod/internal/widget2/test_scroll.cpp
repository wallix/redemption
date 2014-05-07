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

#define LOGNULL
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "frame1.png");

    char message[1024];
    if (!check_sig( drawable.gd.drawable, message,
                    "\xf8\x30\xb3\xcc\xe7\xe6\x7d\x5e\x05\x91"
                    "\x94\x47\x57\xb6\xcf\x98\xff\xb0\xcb\xb1")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_vertical_widget_pos(50);
    // ask to widget to redraw at it's current position
    frame.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "frame2.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\xb5\x72\xb8\x01\x34\x0d\xed\x89\x70\x9a"
                    "\xfc\xad\x28\xe0\x60\x62\xc9\xbd\x72\x08")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
    frame.set_horizontal_widget_pos(70);
    // ask to widget to redraw at it's current position
    frame.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "frame3.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x1e\x7e\xd6\xfd\x56\xb0\x20\xab\x86\x2a"
                    "\x56\xbf\xe4\xe2\xb2\x96\x3d\x5d\x41\x5b")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_vertical_widget_pos(0);
    // frame.rdp_input_invalidate(parent.rect);
    frame.set_horizontal_widget_pos(100);
    // ask to widget to redraw at it's current position
    frame.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "frame4.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x12\x05\xf5\x72\x72\x8e\xcd\x45\xf7\xdd"
                    "\x08\xa7\x73\x53\xad\x87\x60\x24\x7c\xa4")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_vertical_widget_pos(100);
    frame.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "frame5.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x11\x47\x9e\xb4\xc6\x81\xe5\xfe\x35\x16"
                    "\xd7\x02\xa3\x01\x2b\x65\x17\x70\x83\x63")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
    frame.set_horizontal_widget_pos(20);
    frame.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "frame6.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x2c\xb3\xac\x67\x1a\xcc\xac\x6c\x16\x75"
                    "\x55\xa0\x4d\x66\x4c\x80\x89\xa5\xce\x76")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_horizontal_widget_pos(0);
    // frame.rdp_input_invalidate(parent.rect);
    frame.set_vertical_widget_pos(95);
    frame.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "frame7.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x91\x21\x67\xa4\xc6\x04\xc0\x62\xa2\x0c"
                    "\xcd\x32\xfa\xc1\x88\x22\x27\x06\x5b\x59")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFrameScrollbar)
{
    TestDraw drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier    = NULL;
    int         fg_color    = MEDIUM_BLUE;
    int         bg_color    = LIGHT_BLUE;
    int         focus_color = WINBLUE;
    int         group_id    = 0;
    int16_t     x           = 0;
    int16_t     y           = 0;
    uint16_t    cx          = 750;
    uint16_t    cy          = 267;

    WidgetFrame frame(drawable, Rect(x, y, cx, cy), parent, notifier, group_id);
    WidgetImage wimage(drawable, 0, 0, FIXTURES_PATH"/win2008capture10.png", parent, notifier);
    WidgetVScrollBar bar(drawable, parent, notifier, fg_color, bg_color, focus_color,
                         group_id);

    parent.add_widget(&frame);
    parent.add_widget(&bar);
    frame.set_widget(&wimage);
    bar.set_frame(&frame);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar1.png");

    char message[1024];
    if (!check_sig( drawable.gd.drawable, message,
                    "\xed\x43\x4b\x4d\xa8\x3d\x99\xc2\x73\xaa"
                    "\x29\xf4\xb8\x8c\x10\x2a\xde\xf1\x79\xee")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(30);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar2.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x52\x61\xcd\x0d\x63\xa6\xc0\xe7\xe2\xee"
                    "\x89\xa3\x7b\x89\x14\x33\x67\xf3\x8c\x7a")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(60);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar3.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\xb5\xe6\x94\xba\x12\xdd\x08\x82\xe8\xdc"
                    "\x76\xe7\xe2\x78\x61\xa2\x32\xf1\xc9\x37")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(100);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar4.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x20\x99\x11\xa0\x76\x08\x85\xc0\x17\xe3"
                    "\xe4\x52\x67\x9b\xb9\x7d\xf9\x5f\x6d\xa0")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(40);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar5.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\xdb\x0f\x0c\x59\xa0\x57\x9a\x18\x74\x93"
                    "\x0d\x0f\x82\x6f\x95\x26\x8c\x56\x11\x39")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(5);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar6.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\xc7\xc7\x60\x9b\x89\x45\x5d\x6b\x76\x96"
                    "\xdd\x95\x83\x48\x59\xfa\x0e\xe3\x0d\xc5")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(0);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar7.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\xed\x43\x4b\x4d\xa8\x3d\x99\xc2\x73\xaa"
                    "\x29\xf4\xb8\x8c\x10\x2a\xde\xf1\x79\xee")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
}
