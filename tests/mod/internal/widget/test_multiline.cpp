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

#define RED_TEST_MODULE TestWidgetMultiLine
#include "system/redemption_unit_tests.hpp"


#include "core/font.hpp"
#include "mod/internal/widget/multiline.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceWidgetMultiLine)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetMultiLine is a multiline widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 2;

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

    // drawable.save_to_png("multiline.png");

    RED_CHECK_SIG(drawable.gd, "\x88\xaf\xf5\x1d\xc0\x9e\xbf\xf2\xa4\x1c\x81\xa4\x03\x7b\xf9\xa0\x19\xd9\xf4\x3d");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
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

    // drawable.save_to_png("multiline2.png");

    RED_CHECK_SIG(drawable.gd, "\xa9\x61\x87\xc1\xe0\x19\xb9\x37\xed\x1f\xd3\xd7\x07\xac\x57\xaa\x6a\xdb\x1b\xb6");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetMultiLine is a multiline widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
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

    // drawable.save_to_png("multiline3.png");

    RED_CHECK_SIG(drawable.gd, "\x06\xf6\xb0\x76\xd5\x6e\x03\x32\xb7\xcc\xce\x33\xc0\x74\x28\x1c\xb6\xaf\xd8\x1b");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
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

    // drawable.save_to_png("multiline4.png");

    RED_CHECK_SIG(drawable.gd, "\x96\x15\x38\x52\x69\xfb\x96\x18\x6c\x5a\x2e\xa9\x4f\x3a\xf3\x9b\xb8\x4a\x41\x8d");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine5)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetMultiLine is a multiline widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
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

    // drawable.save_to_png("multiline5.png");

    RED_CHECK_SIG(drawable.gd, "\x81\x82\x6f\xee\xcb\xc4\xe0\x61\xb7\x5b\x4d\xe4\x36\x3e\x81\x08\x49\x97\xb9\x65");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine6)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
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

    // drawable.save_to_png("multiline6.png");

    RED_CHECK_SIG(drawable.gd, "\x33\x35\x72\x13\x3c\xa1\x2a\x56\x95\x88\x20\x49\x94\x77\xa8\xdd\x83\x3a\x43\xc3");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
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

    // drawable.save_to_png("multiline7.png");

    RED_CHECK_SIG(drawable.gd, "\xcc\xde\x9f\xc2\x35\xb9\xcd\x22\x82\x00\xd6\x0a\x44\xa3\xf1\xaa\xa9\x08\x63\xa5");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
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

    // drawable.save_to_png("multiline8.png");

    RED_CHECK_SIG(drawable.gd, "\x72\xee\xc6\x3f\x30\x7f\xe1\x55\xd7\x4f\x17\x22\xca\xf3\x5e\xef\xf9\x7e\x1a\x7b");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineTooLong)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
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

    // drawable.save_to_png("multiline9.png");

    RED_CHECK_SIG(drawable.gd, "\x6d\x9c\x92\xc2\xdf\xa4\x3f\x4a\x90\x6a\x1d\x85\x3f\x07\x93\xaf\x70\x46\xab\xf8");
}
