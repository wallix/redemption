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
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/multiline.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TraceWidgetMultiLine)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
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
    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, global_font_deja_vu_14(), xtext, ytext);
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);


    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    // drawable.save_to_png("multiline.png");

    RED_CHECK_SIG(drawable, "\x9b\x92\x55\x24\xaa\xb1\x63\xd8\x6b\xb9\xd4\x7d\xd2\x08\xe9\xff\xf0\x95\x73\x47");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine2)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, global_font_deja_vu_14());
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    // drawable.save_to_png("multiline2.png");

    RED_CHECK_SIG(drawable, "\x91\xc0\xda\xdb\x61\x7c\x9e\x5d\x76\x1b\x81\x88\x1d\xa8\x3e\xeb\x3a\x71\x5c\xf3");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine3)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, global_font_deja_vu_14());
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    // drawable.save_to_png("multiline3.png");

    RED_CHECK_SIG(drawable, "\x20\x2b\xdf\xe4\x2d\xfe\x75\x7c\x52\x45\x8b\x41\x01\x41\x70\x20\x90\x0c\x5d\x53");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine4)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, global_font_deja_vu_14());
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    // drawable.save_to_png("multiline4.png");

    RED_CHECK_SIG(drawable, "\xc0\xb3\xff\x4b\x68\xb8\x8c\xf9\x58\x19\x44\x72\x6a\x7c\xd3\x18\x55\xf6\x40\x82");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine5)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, global_font_deja_vu_14());
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    // drawable.save_to_png("multiline5.png");

    RED_CHECK_SIG(drawable, "\x23\x1b\xda\x2a\x8d\x74\x75\x34\x4c\xa2\x4e\xf2\x4c\xa8\x2c\xc2\x27\x8d\x49\xe7");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine6)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, global_font_deja_vu_14());
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    // drawable.save_to_png("multiline6.png");

    RED_CHECK_SIG(drawable, "\xaf\xef\x9e\xf5\x72\xe1\xb2\x81\x1f\xf7\xb9\x55\xf3\x93\x00\x79\xba\x71\x85\xdc");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineClip)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, global_font_deja_vu_14());
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.x(),
                                         wmultiline.y(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    // drawable.save_to_png("multiline7.png");

    RED_CHECK_SIG(drawable, "\x13\xc4\x5b\xc6\x2f\x90\x1c\x29\xa9\x1d\x22\x77\x95\x0a\xfe\xa6\x04\xce\x19\xfb");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineClip2)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               id, fg_color, bg_color, global_font_deja_vu_14());
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.x(),
                                         5 + wmultiline.y(),
                                         30,
                                         10));

    // drawable.save_to_png("multiline8.png");

    RED_CHECK_SIG(drawable, "\x44\x6c\xa2\x38\x99\xa8\xe0\x3d\xd7\xc8\xf7\x1b\xdf\xfe\x3b\xe5\x7e\x28\xc2\xea");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineTooLong)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                               "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                               "Nam non magna sit amet dui vestibulum feugiat.<br>"
                               "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                               "Nam lacinia purus luctus ante congue facilisis.<br>"
                               "Donec sodales mauris luctus ante ultrices blandit.",
                               id, fg_color, bg_color, global_font_deja_vu_14());
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(wmultiline.get_rect());

    // drawable.save_to_png("multiline9.png");

    RED_CHECK_SIG(drawable, "\x3d\x88\xa2\x81\xdd\x9b\xbb\xcc\xee\xa7\xde\x22\x17\x29\x73\xa9\x47\x59\x90\x27");
}
