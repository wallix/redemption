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
#include "test_only/test_framework/img_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

#include "mod/internal/widget/scroll.hpp"


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


RED_AUTO_TEST_CASE(TestWidgetHScrollBar)
{
    TestScrollCtx ctx(true);

    ctx.draw();
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x2b\xef\x28\x3f\xac\x7f\x07\xcd\x14\x32\x2f\x60\xaa\x54\x52\x85\xb0\x65\x0f\x3d");

    ctx.down(5, 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\xae\x0f\xb8\x17\x9c\x21\xe4\x35\x5f\x93\xcd\xcd\x24\x31\x0a\x91\xb7\xa8\x38\xf9");

    ctx.up(5, 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x2b\xef\x28\x3f\xac\x7f\x07\xcd\x14\x32\x2f\x60\xaa\x54\x52\x85\xb0\x65\x0f\x3d");

    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x9c\x4f\x61\xc1\xe5\xe7\xa2\x66\x18\xab\xbe\xe6\x1f\xab\x9c\x73\xda\x39\xa9\xc6");

    ctx.up(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x63\xfd\xd9\xd9\xa0\x70\x97\x5d\xfe\xca\x01\xa7\x15\x80\x3c\x06\xdc\x40\xdc\x4a");

    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 4);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x68\xee\xa2\x52\xc6\xa5\x04\x99\x47\x1c\xa7\xc6\x8f\x92\x79\x9a\x69\x91\x17\x92");

    ctx.up(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 4);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x27\x5c\xe0\x16\x76\x5d\x47\x5f\x63\xe8\xaa\xb8\x74\xec\xe2\x80\x4d\x02\x31\x38");

    ctx.down(5, 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x79\x09\x8c\x02\x8f\x62\xa3\x87\x1a\xef\x1a\x5c\x7b\x76\x5f\x6f\x60\xd1\x83\xf3");

    for (unsigned pos = 4; pos < 50; pos += 2) {
        ctx.down(ctx.drawable.width() - 5, 5);
        RED_TEST(ctx.notifier.pos == pos);
    }
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x71\x42\x92\x05\xcb\xad\x24\x2d\x68\xc7\x05\x91\x4c\xf8\x5c\x52\x63\xaa\x21\xb3");

    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 50);
    ctx.down(ctx.drawable.width() - 5, 5);
    RED_TEST(ctx.notifier.pos == 50);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x21\xf1\x45\x38\x77\x7f\x9b\x91\x4f\x46\x19\x9b\xc4\x65\x94\xef\x71\xbf\x1a\x9a");
}

RED_AUTO_TEST_CASE(TestWidgetVScrollBar)
{
    TestScrollCtx ctx(false);

    ctx.draw();
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x69\xfd\x98\xbb\x00\x03\xc1\x50\x36\xcb\x21\xb7\x63\x21\x87\x2d\xe3\xb9\x8f\x7a");

    ctx.down(5, 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x75\x8c\x1a\x5f\xe3\xb9\xcf\xa0\xe0\xe3\x33\xa9\x45\xd7\x88\x0d\xfa\x24\xef\xd7");

    ctx.up(5, 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x69\xfd\x98\xbb\x00\x03\xc1\x50\x36\xcb\x21\xb7\x63\x21\x87\x2d\xe3\xb9\x8f\x7a");

    ctx.down(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\xb1\x12\xfc\x0a\xe0\xe8\xc5\x7e\xe9\x06\xe5\xc4\x61\x4c\x8e\x2c\x13\x8e\xd5\x72");

    ctx.up(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\xbf\x9d\xc8\x70\x3a\x94\x1b\x04\xb7\x59\x5e\x47\x2e\xc8\xfa\xcd\x2f\x6b\xea\xc6");

    ctx.down(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 4);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\xef\xb5\x33\x5d\x21\x05\x25\x62\xa8\x24\x7f\x87\xd8\x87\xdb\xf6\xd4\xc0\xcf\x54");

    ctx.up(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 4);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x9e\x51\x4f\xda\xcf\x85\x26\x9e\x31\xe2\x46\xea\x00\x21\xf0\x91\x55\x91\xc6\x93");

    ctx.down(5, 5);
    RED_TEST(ctx.notifier.pos == 2);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x7b\x9b\xfd\x33\x2f\x6a\x94\xc7\x10\x50\xe3\x73\x7c\x53\x45\x63\x67\x9a\x5e\xe2");

    for (unsigned pos = 4; pos < 50; pos += 2) {
        ctx.down(5, ctx.drawable.height() - 5);
        RED_TEST(ctx.notifier.pos == pos);
    }
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\xca\xb6\x49\x84\x04\xfa\xfe\xfc\x6a\xcd\x20\x98\x06\xea\x1d\x8c\x52\x3e\xd1\x7a");

    ctx.down(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 50);
    ctx.down(5, ctx.drawable.height() - 5);
    RED_TEST(ctx.notifier.pos == 50);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x5a\x85\xb9\x28\x70\xa8\x67\xe7\xd0\xa7\x1f\xe4\xe8\x44\x95\x5a\x0f\x31\x9e\x0d");
}

RED_AUTO_TEST_CASE(TestWidgetHScrollBarRail)
{
    int16_t x = 10;
    int16_t y = 10;
    TestScrollCtx ctx(true, true, x, y);

    ctx.draw();
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x88\x1c\x38\x58\x5b\x37\xcb\x8d\x30\x54\x55\x7e\x5c\x6b\x88\x59\x7e\xfb\xe6\x6b");

    ctx.down(x + 5, y + 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\xad\xc4\x95\xf9\xd9\x1d\xf3\xce\x5f\x78\xb4\x97\xe1\x6a\xa5\x73\x0a\xd0\x27\x64");
}

RED_AUTO_TEST_CASE(TestWidgetVScrollBarRail)
{
    int16_t x = 10;
    int16_t y = 10;
    TestScrollCtx ctx(false, true, x, y);

    ctx.draw();
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x0b\x36\x24\x7d\xe3\x81\x13\x15\xf3\x59\xae\xdf\x22\x6c\x74\xcc\xcf\x72\x10\xba");

    ctx.down(x + 5, y + 5);
    RED_TEST(ctx.notifier.pos == 0);
    RED_CHECK_IMG_SIG(ctx.drawable,
        "\x60\xbd\x14\x27\x42\xff\xa5\xbd\x0b\x03\xc7\x6a\xd4\x50\x7b\x6d\x2b\xc4\x65\xaa");
}
