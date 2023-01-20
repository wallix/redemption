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

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "mod/internal/widget/label.hpp"
#include "mod/internal/widget/composite.hpp"

#include <string_view>


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/label/"

struct TestWidgetLabelCtx
{
    struct Colors
    {
        BGRColor fg = RED;
        BGRColor bg = YELLOW;
    };

    TestGraphic drawable{800, 600};
    WidgetLabel wlabel;

    TestWidgetLabelCtx(
        char const* text, Colors colors = Colors{RED, YELLOW}, int xtext = 0, int ytext = 0)
    : wlabel(
        drawable, text,
        colors.fg, colors.bg, global_font_lato_light_16(), xtext, ytext)
    {}
};

RED_AUTO_TEST_CASE(TraceWidgetLabel)
{
    TestWidgetLabelCtx ctx("test1", {}, 4, 1);

    ctx.wlabel.set_wh(ctx.wlabel.get_optimal_dim());
    ctx.wlabel.set_xy(0, 0);

    ctx.wlabel.rdp_input_invalidate(ctx.wlabel.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "label_.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel2)
{
    TestWidgetLabelCtx ctx("test2");

    ctx.wlabel.set_wh(ctx.wlabel.get_optimal_dim());
    ctx.wlabel.set_xy(10, 100);

    ctx.wlabel.rdp_input_invalidate(ctx.wlabel.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "label_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel3)
{
    TestWidgetLabelCtx ctx("test3");

    ctx.wlabel.set_wh(ctx.wlabel.get_optimal_dim());
    ctx.wlabel.set_xy(-10, 500);

    ctx.wlabel.rdp_input_invalidate(ctx.wlabel.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "label_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel4)
{
    TestWidgetLabelCtx ctx("test4");

    ctx.wlabel.set_wh(ctx.wlabel.get_optimal_dim());
    ctx.wlabel.set_xy(770, 500);

    ctx.wlabel.rdp_input_invalidate(ctx.wlabel.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "label_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel5)
{
    TestWidgetLabelCtx ctx("test5");

    ctx.wlabel.set_wh(ctx.wlabel.get_optimal_dim());
    ctx.wlabel.set_xy(-20, -7);

    ctx.wlabel.rdp_input_invalidate(ctx.wlabel.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "label_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel6)
{
    TestWidgetLabelCtx ctx("test6");

    ctx.wlabel.set_wh(ctx.wlabel.get_optimal_dim());
    ctx.wlabel.set_xy(760, -7);

    ctx.wlabel.rdp_input_invalidate(ctx.wlabel.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "label_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelClip)
{
    TestWidgetLabelCtx ctx("test6");

    ctx.wlabel.set_wh(ctx.wlabel.get_optimal_dim());
    ctx.wlabel.set_xy(760, -7);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    ctx.wlabel.rdp_input_invalidate(Rect(
        20 + ctx.wlabel.x(),
        ctx.wlabel.y(),
        ctx.wlabel.cx(),
        ctx.wlabel.cy()
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "label_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelClip2)
{
    TestWidgetLabelCtx ctx("test6");

    ctx.wlabel.set_wh(ctx.wlabel.get_optimal_dim());
    ctx.wlabel.set_xy(0, 0);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    ctx.wlabel.rdp_input_invalidate(Rect(
        20 + ctx.wlabel.x(),
        5 + ctx.wlabel.y(),
        30,
        10
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "label_8.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelAndComposite)
{
    TestGraphic drawable(800, 600);

    WidgetComposite wcomposite(drawable);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetLabel wlabel1(drawable,
                        "abababab", YELLOW, BLACK, global_font_lato_light_16());
    wlabel1.set_wh(wlabel1.get_optimal_dim());
    wlabel1.set_xy(0, 0);

    WidgetLabel wlabel2(drawable,
                        "ggghdgh", WHITE, BLUE, global_font_lato_light_16());
    wlabel2.set_wh(wlabel2.get_optimal_dim());
    wlabel2.set_xy(0, 100);

    WidgetLabel wlabel3(drawable,
                        "lldlslql", BLUE, RED, global_font_lato_light_16());
    wlabel3.set_wh(wlabel3.get_optimal_dim());
    wlabel3.set_xy(100, 100);

    WidgetLabel wlabel4(drawable,
                        "LLLLMLLM", PINK, DARK_GREEN, global_font_lato_light_16());
    wlabel4.set_wh(wlabel4.get_optimal_dim());
    wlabel4.set_xy(300, 300);

    WidgetLabel wlabel5(drawable,
                        "dsdsdjdjs", LIGHT_GREEN, DARK_BLUE, global_font_lato_light_16());
    wlabel5.set_wh(wlabel5.get_optimal_dim());
    wlabel5.set_xy(700, -10);

    WidgetLabel wlabel6(drawable,
                        "xxwwp", ANTHRACITE, PALE_GREEN, global_font_lato_light_16());
    wlabel6.set_wh(wlabel6.get_optimal_dim());
    wlabel6.set_xy(-10, 550);

    wcomposite.add_widget(&wlabel1);
    wcomposite.add_widget(&wlabel2);
    wcomposite.add_widget(&wlabel3);
    wcomposite.add_widget(&wlabel4);
    wcomposite.add_widget(&wlabel5);
    wcomposite.add_widget(&wlabel6);

    //ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_9.png");

    //ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_10.png");

    wcomposite.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetLabelMax)
{
    auto text =
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"_av;

    TestWidgetLabelCtx ctx(text.data());

    ctx.wlabel.set_wh(ctx.wlabel.get_optimal_dim());
    ctx.wlabel.set_xy(10, 100);

    RED_CHECK(std::string_view(ctx.wlabel.get_text()) == text.subarray(0, text.size()-2));

    // ask to widget to redraw at it's current position
    ctx.wlabel.rdp_input_invalidate(ctx.wlabel.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "label_11.png");
}
