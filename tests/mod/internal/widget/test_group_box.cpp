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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Meng Tan, Raphael Zhou
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/widget/button.hpp"
#include "mod/internal/widget/group_box.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/group_box/"

struct TestWidgetGroupBoxCtx
{
    TestGraphic drawable{800, 600};
    WidgetGroupBox wgroupbox;
    WidgetButton wbutton;

    TestWidgetGroupBoxCtx(const char * text)
    : wgroupbox(drawable, text,
                /*fg_color=*/RED, /*bg_color=*/YELLOW, global_font_deja_vu_14())
    , wbutton(drawable, "Button 1", WidgetEventNotifier(),
              /*fg_color=*/RED, /*bg_color=*/YELLOW, /*focuscolor*/LIGHT_YELLOW, 2,
              global_font_deja_vu_14(), 4, 1)
    {
        int16_t x = 200;
        int16_t y = 100;

        wgroupbox.set_wh(150, 200);
        wgroupbox.set_xy(x, y);

        wbutton.set_wh(wbutton.get_optimal_dim());
        wbutton.set_xy(x + 10, y + 20);

        wgroupbox.add_widget(wbutton);

        wgroupbox.rdp_input_invalidate(wgroupbox.get_rect());
    }

    void click()
    {
        wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN,
                                wbutton.x() + 1, wbutton.y() + 1);
        wgroupbox.rdp_input_invalidate(wgroupbox.get_rect());
    }
};

RED_AUTO_TEST_CASE(TraceWidgetGroupBox)
{
    TestWidgetGroupBoxCtx ctx("Group 1");

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "group_box_1.png");

    ctx.click();

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "group_box_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetGroupBoxMax)
{
    auto text =
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"_av;

    TestWidgetGroupBoxCtx ctx(text.data());

    RED_CHECK(std::string_view(ctx.wgroupbox.get_text()) == text.first(text.size()-2));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "group_box_3.png");

    ctx.click();

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "group_box_4.png");
}
