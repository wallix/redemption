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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

#include "mod/internal/widget/scroll.hpp"


struct TestScrollCtx
{
    struct ScrollNotify : NotifyApi
    {
        WidgetScrollBar* scroll;
        unsigned pos = 0;

        void notify(Widget & sender, notify_event_t /*event*/) override
        {
            RED_REQUIRE(scroll == &sender);

            auto new_pos = this->scroll->get_current_value();
            RED_TEST(this->pos != new_pos);
            this->pos = new_pos;
        }
    };

    TestGraphic drawable;
    ScrollNotify notifier;
    WidgetScrollBar scroll;

    TestScrollCtx(bool is_horizontal, bool rail_style = false, int16_t x =  0, int16_t y = 0)
    : drawable(1, 1)
    , scroll(
        this->drawable, this->scroll, &this->notifier, is_horizontal, /*id=*/0,
        /*fg_color=*/RED, /*bg_color=*/YELLOW, /*focus_color=*/WINBLUE,
        global_font_deja_vu_14(), rail_style, 50)
    {
        Dimension dim = this->scroll.get_optimal_dim();
        this->drawable.resize(
            align4(is_horizontal ? 200 : dim.w) + x*2,
            (is_horizontal ? dim.h : 200) + y*2);
        this->scroll.set_wh(this->drawable.width() - x*2, this->drawable.height() - y*2);
        this->scroll.set_xy(x, y);
        this->notifier.scroll = &this->scroll;
    }

    void draw()
    {
        this->scroll.rdp_input_invalidate(this->scroll.get_rect());
    }

    void down(int x, int y)
    {
        this->scroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x, y, nullptr);
        this->draw();
    }

    void up(int x, int y)
    {
        this->scroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
        this->draw();
    }
};

#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/scroll/"


RED_AUTO_TEST_CASE(TestWidgetHScrollBar)
{
    TestScrollCtx ctx(true);

    ctx.draw();
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_1.png");

    ctx.down(5, 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_2.png");

    ctx.up(5, 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_1.png");

    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_3.png");

    ctx.up(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_4.png");

    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 4);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_5.png");

    ctx.up(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 4);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_6.png");

    ctx.down(5, 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_7.png");

    for (unsigned pos = 4; pos < 50; pos += 2) {
        ctx.down(ctx.drawable.width() - 5, 5);
        RED_TEST(ctx.notifier.pos == pos);
    }
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_8.png");

    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 50);
    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 50);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_9.png");
}

RED_AUTO_TEST_CASE(TestWidgetVScrollBar)
{
    TestScrollCtx ctx(false);

    ctx.draw();
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_1.png");

    ctx.down(5, 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_2.png");

    ctx.up(5, 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_1.png");

    ctx.down(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_3.png");

    ctx.up(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_4.png");

    ctx.down(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 4);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_5.png");

    ctx.up(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 4);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_6.png");

    ctx.down(5, 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_7.png");

    for (unsigned pos = 4; pos < 50; pos += 2) {
        ctx.down(5, ctx.drawable.height() - 5);
        RED_TEST(ctx.notifier.pos == pos);
    }
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_8.png");

    ctx.down(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 50);
    ctx.down(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 50);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_9.png");
}

RED_AUTO_TEST_CASE(TestWidgetHScrollBarRail)
{
    int16_t x = 10;
    int16_t y = 10;
    TestScrollCtx ctx(true, true, x, y);

    ctx.draw();
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_rail_1.png");

    ctx.down(x + 5, y + 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "hscroll_rail_2.png");
}

RED_AUTO_TEST_CASE(TestWidgetVScrollBarRail)
{
    int16_t x = 10;
    int16_t y = 10;
    TestScrollCtx ctx(false, true, x, y);

    ctx.draw();
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_rail_1.png");

    ctx.down(x + 5, y + 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "vscroll_rail_2.png");
}
