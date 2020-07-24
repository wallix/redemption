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

#include "mod/internal/widget/scroll.hpp"
#include "mod/internal/widget/screen.hpp"

#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

#include "utils/png.hpp"


struct TestScrollCtx
{
    struct ScrollNotify : NotifyApi
    {
        WidgetScrollBar* scroll;
        unsigned pos = 0;

        void notify(Widget * sender, notify_event_t /*event*/) override
        {
            RED_REQUIRE(scroll == sender);

            auto new_pos = this->scroll->get_current_value();
            RED_TEST(this->pos != new_pos);
            this->pos = new_pos;
        }
    };

    TestGraphic drawable;
    ScrollNotify notifier;
    WidgetScrollBar scroll;

    TestScrollCtx(bool is_horizontal)
    : drawable(1, 1)
    , scroll(
        this->drawable, this->scroll, &this->notifier, is_horizontal, /*id=*/0,
        /*fg_color=*/RED, /*bg_color=*/YELLOW, /*focus_color=*/WINBLUE,
        global_font_deja_vu_14(), false, 50)
    {
        Dimension dim = this->scroll.get_optimal_dim();
        this->drawable.resize(
            is_horizontal ? 200 : align4(dim.w),
            is_horizontal ? dim.h : 200);
        this->scroll.set_wh(this->drawable.width(), this->drawable.height());
        // wscroll.set_xy(0, 0);
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


RED_AUTO_TEST_CASE(TestWidgetHScrollBar)
{
    TestScrollCtx ctx(true);

    ctx.draw();
    // dump_png24("/tmp/scroll0.png", ctx.drawable, false);

    RED_CHECK_SIG(ctx.drawable,
        "\x2b\xef\x28\x3f\xac\x7f\x07\xcd\x14\x32\x2f\x60\xaa\x54\x52\x85\xb0\x65\x0f\x3d");

    ctx.down(5, 5);
    RED_TEST(ctx.notifier.pos == 0);
    // dump_png24("/tmp/scroll1.png", ctx.drawable, false);
    RED_CHECK_SIG(ctx.drawable,
        "\xae\x0f\xb8\x17\x9c\x21\xe4\x35\x5f\x93\xcd\xcd\x24\x31\x0a\x91\xb7\xa8\x38\xf9");

    ctx.up(5, 5);
    RED_TEST(ctx.notifier.pos == 0);
    // dump_png24("/tmp/scroll2.png", ctx.drawable, false);
    RED_CHECK_SIG(ctx.drawable,
        "\x2b\xef\x28\x3f\xac\x7f\x07\xcd\x14\x32\x2f\x60\xaa\x54\x52\x85\xb0\x65\x0f\x3d");

    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 2);
    // dump_png24("/tmp/scroll3.png", ctx.drawable, false);
    RED_CHECK_SIG(ctx.drawable,
        "\x9c\x4f\x61\xc1\xe5\xe7\xa2\x66\x18\xab\xbe\xe6\x1f\xab\x9c\x73\xda\x39\xa9\xc6");

    ctx.up(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 2);
    // dump_png24("/tmp/scroll4.png", ctx.drawable, false);
    RED_CHECK_SIG(ctx.drawable,
        "\x63\xfd\xd9\xd9\xa0\x70\x97\x5d\xfe\xca\x01\xa7\x15\x80\x3c\x06\xdc\x40\xdc\x4a");

    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 4);
    // dump_png24("/tmp/scroll5.png", ctx.drawable, false);
    RED_CHECK_SIG(ctx.drawable,
        "\x68\xee\xa2\x52\xc6\xa5\x04\x99\x47\x1c\xa7\xc6\x8f\x92\x79\x9a\x69\x91\x17\x92");

    ctx.up(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 4);
    // dump_png24("/tmp/scroll6.png", ctx.drawable, false);
    RED_CHECK_SIG(ctx.drawable,
        "\x27\x5c\xe0\x16\x76\x5d\x47\x5f\x63\xe8\xaa\xb8\x74\xec\xe2\x80\x4d\x02\x31\x38");

    ctx.down(5, 5);
    RED_TEST(ctx.notifier.pos == 2);
    // dump_png24("/tmp/scroll7.png", ctx.drawable, false);
    RED_CHECK_SIG(ctx.drawable,
        "\x79\x09\x8c\x02\x8f\x62\xa3\x87\x1a\xef\x1a\x5c\x7b\x76\x5f\x6f\x60\xd1\x83\xf3");

    for (unsigned pos = 4; pos < 50; pos += 2) {
        ctx.down(ctx.drawable.width() - 5, 5);
        RED_TEST(ctx.notifier.pos == pos);
    }
    // dump_png24("/tmp/scroll9.png", ctx.drawable, false);
    RED_CHECK_SIG(ctx.drawable,
        "\x71\x42\x92\x05\xcb\xad\x24\x2d\x68\xc7\x05\x91\x4c\xf8\x5c\x52\x63\xaa\x21\xb3");
    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 50);
    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 50);
    // dump_png24("/tmp/scroll8.png", ctx.drawable, false);
    RED_CHECK_SIG(ctx.drawable,
        "\x21\xf1\x45\x38\x77\x7f\x9b\x91\x4f\x46\x19\x9b\xc4\x65\x94\xef\x71\xbf\x1a\x9a");
}

RED_AUTO_TEST_CASE(TestWidgetVScrollBar)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor focus_color = WINBLUE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetScrollBar wscroll(drawable, parent, notifier, false, id,
                          fg_color, bg_color, focus_color, global_font_deja_vu_14(), false, 50);
    Dimension dim = wscroll.get_optimal_dim();
    wscroll.set_wh(dim.w, 200);
    wscroll.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll2.png");

    RED_CHECK_SIG(drawable, "\x93\x86\xcf\xe2\xc6\x80\xd5\xc5\x2f\x86\x88\xe6\x84\xe5\xa7\xf5\xd6\x73\x48\xdd");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll3.png");

    RED_CHECK_SIG(drawable, "\xcb\x83\xa8\x83\x17\x87\x58\x7a\xd0\x1f\xff\x5f\x4c\x66\x14\xc7\x9f\x3a\x08\xd7");
}

RED_AUTO_TEST_CASE(TestWidgetHScrollBarRail)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor focus_color = WINBLUE;
    int id = 0;
    int16_t x = 10;
    int16_t y = 10;

    WidgetScrollBar wscroll(drawable, parent, notifier, true, id,
                          fg_color, bg_color, focus_color, global_font_deja_vu_14(), true, 50);
    Dimension dim = wscroll.get_optimal_dim();
    wscroll.set_wh(200, dim.h);
    wscroll.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll4.png");

    RED_CHECK_SIG(drawable, "\x4f\x41\xcc\x17\xc7\x5a\x34\xe4\x3e\x66\x8b\xba\xc8\xad\xb2\xa4\xbb\x1f\x94\x14");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll5.png");

    RED_CHECK_SIG(drawable, "\x7a\x63\x82\x5d\xae\xea\x55\x19\x22\xea\xc8\xb3\xe8\x17\xae\xda\x47\x3a\x33\x43");
}

RED_AUTO_TEST_CASE(TestWidgetVScrollBarRail)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor focus_color = WINBLUE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetScrollBar wscroll(drawable, parent, notifier, false, id,
                          fg_color, bg_color, focus_color, global_font_deja_vu_14(), true, 50);
    Dimension dim = wscroll.get_optimal_dim();
    wscroll.set_wh(dim.w, 200);
    wscroll.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll6.png");

    RED_CHECK_SIG(drawable, "\x7c\x69\xca\xee\x27\xa0\xba\x49\xbd\xc6\xac\x77\x25\xeb\x10\x3e\x35\xc6\xea\xe0");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll7.png");

    RED_CHECK_SIG(drawable, "\x8b\x25\x23\xc5\x06\xab\x0c\xcd\x7e\xd0\x18\xde\x8b\x49\xa1\x8b\xb6\xad\x6b\x73");
}
