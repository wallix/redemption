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

#define UNIT_TEST_MODULE TestWidgetMultiLine
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget2/multiline.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#include "fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceWidgetMultiLine)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    /* TODO
     * I believe users of this widget may wish to control text position and behavior inside rectangle
     * ie: text may be centered, aligned left, aligned right, or even upside down, etc
     * these possibilities (and others) are supported in RDPGlyphIndex */
    WidgetMultiLine wmultiline(drawable.gd, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, font, xtext, ytext);
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);


    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "multiline.png");

    RED_CHECK_SIG(drawable.gd, "\x72\x15\x18\x3f\xdf\xc5\x0c\xce\xb5\x7e\x35\xc7\xee\xee\xd6\x9b\x6a\xd0\x7b\xe4");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    WidgetMultiLine wmultiline(drawable.gd, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, font);
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline2.png");

    RED_CHECK_SIG(drawable.gd, "\x40\x55\x16\x14\x94\xe6\x87\xe4\xe9\xda\xe9\x4a\x1e\x7a\x9a\x9f\x37\x95\x83\x8a");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;

    WidgetMultiLine wmultiline(drawable.gd, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, font);
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline3.png");

    RED_CHECK_SIG(drawable.gd, "\xe4\x4e\x51\x1a\xfc\x29\xf8\x6e\xb5\xf1\xb4\x5e\x6a\xb5\x27\xad\x10\x57\x28\xc5");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;

    WidgetMultiLine wmultiline(drawable.gd, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, font);
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline4.png");

    RED_CHECK_SIG(drawable.gd, "\x9a\x2d\xc2\x29\xac\x20\xa1\xa4\x08\xea\x94\x51\x7d\x67\x84\xf9\x7d\x28\x29\xb7");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine5)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable.gd, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, font);
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline5.png");

    RED_CHECK_SIG(drawable.gd, "\xd6\xb8\x09\x56\xfa\xde\xd0\xd4\xe0\x14\xe1\x5e\x2a\x3c\x8d\x79\x65\xee\xc3\xcf");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine6)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable.gd, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, font);
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline6.png");

    RED_CHECK_SIG(drawable.gd, "\x4b\xcc\xd8\xb0\x23\x6d\xa6\x2e\x79\x2b\xab\x81\x20\x82\x8e\x8c\x68\x94\x10\x8f");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable.gd, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, font);
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline7.png");

    RED_CHECK_SIG(drawable.gd, "\x6e\xd9\xe7\x6d\x10\x0b\x6f\x62\x93\xd0\x55\xf3\xb1\x20\x06\x57\xdb\x27\x25\x78");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(drawable.gd, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, font);
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.x(),
                                         5 + wmultiline.y(),
                                         30,
                                         10));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline8.png");

    RED_CHECK_SIG(drawable.gd, "\xc5\x52\xb0\x08\x97\xa8\x7b\x4c\x83\x8f\x57\xb9\xdd\xde\xf4\x4f\x4b\xcf\x54\x4e");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineTooLong)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(drawable.gd, parent, notifier,
                               "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                               "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                               "Nam non magna sit amet dui vestibulum feugiat.<br>"
                               "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                               "Nam lacinia purus luctus ante congue facilisis.<br>"
                               "Donec sodales mauris luctus ante ultrices blandit.",
                               id, fg_color, bg_color, font);
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(wmultiline.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline9.png");

    RED_CHECK_SIG(drawable.gd, "\x2f\xbb\xe7\xbc\xd2\xcb\x0d\x46\xb6\x27\x24\xd4\x61\x9a\xd4\xc3\x2c\x37\x82\x0c");
}
