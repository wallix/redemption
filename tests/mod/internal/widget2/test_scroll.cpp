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

    NotifyApi * notifier = NULL;
    int         fg_color = MEDIUM_BLUE;
    int         bg_color = LIGHT_BLUE;
    int         group_id = 0;
    int16_t     x        = 0;
    int16_t     y        = 0;
    uint16_t    cx       = 750;
    uint16_t    cy       = 267;

    WidgetFrame frame(drawable, Rect(x, y, cx, cy), parent, notifier, group_id);
    WidgetImage wimage(drawable, 0, 0, FIXTURES_PATH"/win2008capture10.png", parent, notifier);
    WidgetVScrollBar bar(drawable, parent, notifier, fg_color, bg_color, group_id);

    parent.add_widget(&frame);
    parent.add_widget(&bar);
    frame.set_widget(&wimage);
    bar.set_frame(&frame);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar1.png");

    char message[1024];
    if (!check_sig( drawable.gd.drawable, message,
                    "\x7b\x4b\x06\x75\xc5\xd0\x1a\x7c\xcd\x6a"
                    "\x07\x16\xc7\x30\x90\x18\x42\x6e\xc4\x05")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(30);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar2.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\xd0\x94\x1b\x8c\x9e\x94\x38\x38\xf2\xd1"
                    "\xdb\x12\xe2\xc1\xae\x7a\x44\xc1\x75\x9f")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(60);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar3.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x4d\x94\x2b\x51\xd7\x0a\x77\xc9\x08\x1a"
                    "\x97\x2b\xc0\xe1\xc7\x3f\xaa\x71\x41\x5e")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(100);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar4.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\xce\x0d\xb8\xaa\x16\x59\x16\x66\xcf\x3e"
                    "\xed\x3e\x3c\xa2\x1c\xb6\x53\xd7\x4e\x97")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(40);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar5.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x30\xb2\x15\xa2\xf3\xad\xc1\x65\xf1\x69"
                    "\x92\x3c\x73\x93\x17\x3a\xc0\x3b\x8a\x29")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(5);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar6.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\xde\x0d\x45\x04\xb7\xdc\x3f\xc6\x14\xda"
                    "\xf9\xe1\x93\x64\x0f\x29\xc2\x6a\x5e\x44")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(0);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebar7.png");

    if (!check_sig( drawable.gd.drawable, message,
                    "\x7b\x4b\x06\x75\xc5\xd0\x1a\x7c\xcd\x6a"
                    "\x07\x16\xc7\x30\x90\x18\x42\x6e\xc4\x05")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
}
