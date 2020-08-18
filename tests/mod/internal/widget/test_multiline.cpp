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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/widget/multiline.hpp"
#include "mod/internal/widget/screen.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/multiline/"


RED_AUTO_TEST_CASE(TraceWidgetMultiLine)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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
                               "line 1\n"
                               "line 2\n"
                               "\n"
                               "line 3, blah blah\n"
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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "multiline_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine2)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1\n"
                               "line 2\n"
                               "\n"
                               "line 3, blah blah\n"
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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "multiline_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine3)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1\n"
                               "line 2\n"
                               "\n"
                               "line 3, blah blah\n"
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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "multiline_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine4)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1\n"
                               "line 2\n"
                               "\n"
                               "line 3, blah blah\n"
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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "multiline_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine5)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1\n"
                               "line 2\n"
                               "\n"
                               "line 3, blah blah\n"
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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "multiline_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine6)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1\n"
                               "line 2\n"
                               "\n"
                               "line 3, blah blah\n"
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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "multiline_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineClip)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1\n"
                               "line 2\n"
                               "\n"
                               "line 3, blah blah\n"
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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "multiline_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineClip2)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "line 1\n"
                               "line 2\n"
                               "\n"
                               "line 3, blah blah\n"
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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "multiline_8.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineTooLong)
{
    TestGraphic drawable(800, 600);


    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = CYAN;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(drawable, parent, notifier,
                               "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"
                               "Curabitur sit amet eros rutrum mi ultricies tempor.\n"
                               "Nam non magna sit amet dui vestibulum feugiat.\n"
                               "Praesent vitae purus et lacus tincidunt lobortis.\n"
                               "Nam lacinia purus luctus ante congue facilisis.\n"
                               "Donec sodales mauris luctus ante ultrices blandit.",
                               id, fg_color, bg_color, global_font_deja_vu_14());
    Dimension dim = wmultiline.get_optimal_dim();
    wmultiline.set_wh(dim);
    wmultiline.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(wmultiline.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "multiline_9.png");
}
