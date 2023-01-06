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

constexpr char const* short_message_ml =
    "line 1\n"
    "line 2\n"
    "\n"
    "line 3, blah blah\n"
    "line 4";

struct TestWidgetMultiLineCtx
{
    TestGraphic drawable{800, 600};
    WidgetScreen parent{drawable, 800, 600, global_font_deja_vu_14(), Theme{}};
    WidgetMultiLine wmultiline;

    TestWidgetMultiLineCtx(char const* text, int xtext = 0, int ytext = 0)
    : wmultiline(
        drawable, parent, text, BLUE, CYAN,
        global_font_deja_vu_14(), xtext, ytext)
    {
        wmultiline.set_wh(wmultiline.get_optimal_dim());
    }
};

RED_AUTO_TEST_CASE(TraceWidgetMultiLine)
{
    TestWidgetMultiLineCtx ctx(short_message_ml, 4, 2);

    ctx.wmultiline.set_xy(0, 0);
    ctx.wmultiline.rdp_input_invalidate(ctx.wmultiline.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "multiline_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine2)
{
    TestWidgetMultiLineCtx ctx(short_message_ml);

    ctx.wmultiline.set_xy(10, 100);
    ctx.wmultiline.rdp_input_invalidate(ctx.wmultiline.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "multiline_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine3)
{
    TestWidgetMultiLineCtx ctx(short_message_ml);

    ctx.wmultiline.set_xy(-10, 500);
    ctx.wmultiline.rdp_input_invalidate(ctx.wmultiline.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "multiline_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine4)
{
    TestWidgetMultiLineCtx ctx(short_message_ml);

    ctx.wmultiline.set_xy(770, 500);
    ctx.wmultiline.rdp_input_invalidate(ctx.wmultiline.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "multiline_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine5)
{
    TestWidgetMultiLineCtx ctx(short_message_ml);

    ctx.wmultiline.set_xy(-20, -7);
    ctx.wmultiline.rdp_input_invalidate(ctx.wmultiline.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "multiline_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLine6)
{
    TestWidgetMultiLineCtx ctx(short_message_ml);

    ctx.wmultiline.set_xy(760, -7);
    ctx.wmultiline.rdp_input_invalidate(ctx.wmultiline.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "multiline_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineClip)
{
    TestWidgetMultiLineCtx ctx(short_message_ml);

    ctx.wmultiline.set_xy(760, -7);
    ctx.wmultiline.rdp_input_invalidate(Rect(
        20 + ctx.wmultiline.x(),
        ctx.wmultiline.y(),
        ctx.wmultiline.cx(),
        ctx.wmultiline.cy()
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "multiline_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineClip2)
{
    TestWidgetMultiLineCtx ctx(short_message_ml);

    ctx.wmultiline.set_xy(0, 0);
    ctx.wmultiline.rdp_input_invalidate(Rect(
        20 + ctx.wmultiline.x(),
        5 + ctx.wmultiline.y(),
        30,
        10
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "multiline_8.png");
}

RED_AUTO_TEST_CASE(TraceWidgetMultiLineTooLong)
{
    TestWidgetMultiLineCtx ctx(
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"
        "Curabitur sit amet eros rutrum mi ultricies tempor.\n"
        "Nam non magna sit amet dui vestibulum feugiat.\n"
        "Praesent vitae purus et lacus tincidunt lobortis.\n"
        "Nam lacinia purus luctus ante congue facilisis.\n"
        "Donec sodales mauris luctus ante ultrices blandit.");

    ctx.wmultiline.set_xy(0, 0);
    ctx.wmultiline.rdp_input_invalidate(ctx.wmultiline.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "multiline_9.png");
}
