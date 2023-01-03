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

#include "mod/internal/widget/widget_rect.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/rect/"


struct WidgetRectFixture
{
    TestGraphic drawable{800, 600};
    WidgetScreen parent{drawable, 800, 600, global_font(), nullptr, Theme{}};
    WidgetRect wrect{drawable, parent, nullptr, /*id=*/0, BGRColor(0xCCF604)};
};


RED_FIXTURE_TEST_CASE(TraceWidgetRect, WidgetRectFixture)
{
    wrect.set_wh(800, 600);
    wrect.set_xy(0, 0);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(wrect.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_1.png");
}

RED_FIXTURE_TEST_CASE(TraceWidgetRect2, WidgetRectFixture)
{
    wrect.set_wh(200, 200);
    wrect.set_xy(-100, -100);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(wrect.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_2.png");
}

RED_FIXTURE_TEST_CASE(TraceWidgetRect3, WidgetRectFixture)
{
    wrect.set_wh(200, 200);
    wrect.set_xy(-100, 500);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(wrect.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_3.png");
}

RED_FIXTURE_TEST_CASE(TraceWidgetRect4, WidgetRectFixture)
{
    wrect.set_wh(200, 200);
    wrect.set_xy(700, 500);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(wrect.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_4.png");
}

RED_FIXTURE_TEST_CASE(TraceWidgetRect5, WidgetRectFixture)
{
    wrect.set_wh(200, 200);
    wrect.set_xy(700, -100);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(wrect.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_5.png");
}

RED_FIXTURE_TEST_CASE(TraceWidgetRect6, WidgetRectFixture)
{
    wrect.set_wh(200, 200);
    wrect.set_xy(300, 200);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(wrect.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_6.png");
}

RED_FIXTURE_TEST_CASE(TraceWidgetRectClip, WidgetRectFixture)
{
    wrect.set_wh(200, 200);
    wrect.set_xy(300, 200);

    // ask to widget to redraw at position 400,300 and of size 100x100. After clip the size is of 100x50
    wrect.rdp_input_invalidate(Rect(150 + wrect.x(),
                                    100 + wrect.y(),
                                    100,
                                    100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_7.png");
}

RED_FIXTURE_TEST_CASE(TraceWidgetRectClip2, WidgetRectFixture)
{
    wrect.set_wh(200, 200);
    wrect.set_xy(700, -100);

    // ask to widget to redraw at position 720,20 and of size 50x50
    wrect.rdp_input_invalidate(Rect(20 + wrect.x(),
                                    120 + wrect.y(),
                                    50,
                                    50));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_8.png");
}
