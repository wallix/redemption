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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetMultiLine
#include "system/redemption_unit_tests.hpp"

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget2/multiline.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    TODO("I believe users of this widget may wish to control text position and behavior inside rectangle"
         "ie: text may be centered, aligned left, aligned right, or even upside down, etc"
         "these possibilities (and others) are supported in RDPGlyphIndex")
    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color, font, xtext, ytext);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "multiline.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x79\x04\xc6\xe6\x88\x42\xfc\x31\xb5\x18"
        "\xf9\xb8\xce\xca\x8b\x75\xb6\xf0\xee\x96"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 100;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color, font);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline2.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc6\x2e\xbe\xa7\xa6\x8b\x95\x65\x10\xc4"
        "\x4d\x8c\x25\x54\xcf\xd9\x7e\x0a\x19\xc9"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -10;
    int16_t y = 500;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color, font);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline3.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x7b\x75\x7c\x63\x30\x5c\x76\xb8\x48\xa7"
        "\xaa\x60\xee\x6b\x6a\x42\xcb\x56\xa4\xec"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 770;
    int16_t y = 500;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color, font);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline4.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x39\xf3\xcb\x87\x56\x8a\x6e\xe1\xd2\xc4"
        "\xab\x2a\x52\x12\xb2\xbf\x16\x53\x0f\xf6"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine5)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -20;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color, font);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline5.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc3\x12\x01\x1b\x1b\xa3\x51\xc6\x79\x05"
        "\xa9\x89\x30\xda\x24\xe9\x49\x9d\x27\x06"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine6)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color, font);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline6.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x86\x81\xe2\x98\x84\xae\xdb\xa1\xf6\xf0"
        "\xf6\x89\x5a\x18\x28\x61\x35\xac\x13\xa9"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLineClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color, font);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline7.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xb3\xe6\x76\x89\x65\x50\xc0\x73\x86\xcd"
        "\x27\x3d\x5a\xe2\x46\x23\xb2\x39\xd4\xa5"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLineClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color, font);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.dx(),
                                         5 + wmultiline.dy(),
                                         30,
                                         10));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline8.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x14\xe3\x00\xef\xb0\xe4\x12\x06\xf1\xbf"
        "\x3a\x59\xe0\x57\x6d\xea\x71\x97\xd3\xd3"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLineTooLong)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                               "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                               "Nam non magna sit amet dui vestibulum feugiat.<br>"
                               "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                               "Nam lacinia purus luctus ante congue facilisis.<br>"
                               "Donec sodales mauris luctus ante ultrices blandit.",
                               auto_resize, id, fg_color, bg_color, font);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(wmultiline.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline9.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xa6\x30\xe2\xd0\xa4\x03\xd1\x30\xdc\xdb"
        "\x2b\xa2\xf8\xa3\xe8\x40\xad\x7c\xb8\x18"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}
