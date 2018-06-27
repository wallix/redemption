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

#define RED_TEST_MODULE TestWidgetScroll
#include "system/redemption_unit_tests.hpp"


#include "mod/internal/widget/scroll.hpp"
#include "mod/internal/widget/screen.hpp"

#include "test_only/check_sig.hpp"
#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(TestWidgetHScrollBar)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor focus_color = WINBLUE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetScrollBar wscroll(drawable.gd, parent, notifier, true, id,
                          fg_color, bg_color, focus_color, font, false, 50);
    Dimension dim = wscroll.get_optimal_dim();
    wscroll.set_wh(200, dim.h);
    wscroll.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll0.png");

    RED_CHECK_SIG(drawable.gd, "\x83\x21\x77\x50\x83\x42\xea\x1f\xf9\x35\x0a\x76\x91\xc0\x75\x18\x60\x2a\xad\xc5");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll1.png");

    RED_CHECK_SIG(drawable.gd, "\xa2\x14\x83\xa8\xce\xb3\xba\xab\x10\x42\xd8\xf8\xdb\x41\x57\xad\x38\xd8\x73\xb8");
}

RED_AUTO_TEST_CASE(TestWidgetVScrollBar)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor focus_color = WINBLUE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetScrollBar wscroll(drawable.gd, parent, notifier, false, id,
                          fg_color, bg_color, focus_color, font, false, 50);
    Dimension dim = wscroll.get_optimal_dim();
    wscroll.set_wh(dim.w, 200);
    wscroll.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll2.png");

    RED_CHECK_SIG(drawable.gd, "\xeb\xf1\x30\x34\xcc\x5b\x67\x76\x4e\x93\x8e\xe4\xd0\xe4\x33\xb1\x58\xa6\x35\xae");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll3.png");

    RED_CHECK_SIG(drawable.gd, "\xa5\xc0\x2d\x25\xee\xd2\x84\x7a\x1e\x79\x09\x9b\x02\x4b\x80\xf0\x30\x6f\x17\xbe");
}

RED_AUTO_TEST_CASE(TestWidgetHScrollBarRail)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor focus_color = WINBLUE;
    int id = 0;
    int16_t x = 10;
    int16_t y = 10;

    WidgetScrollBar wscroll(drawable.gd, parent, notifier, true, id,
                          fg_color, bg_color, focus_color, font, true, 50);
    Dimension dim = wscroll.get_optimal_dim();
    wscroll.set_wh(200, dim.h);
    wscroll.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll4.png");

    RED_CHECK_SIG(drawable.gd, "\x05\x03\xd6\xd9\xd8\x54\xd5\x04\x76\xac\xa0\x10\xed\x0b\x9d\xa8\x45\x3f\x51\xb2");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll5.png");

    RED_CHECK_SIG(drawable.gd, "\x02\x3f\xf7\x34\x86\x04\x07\x02\x99\x1b\x4d\x41\x40\xf6\x96\xd1\xcf\x13\x5c\x70");
}

RED_AUTO_TEST_CASE(TestWidgetVScrollBarRail)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor focus_color = WINBLUE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetScrollBar wscroll(drawable.gd, parent, notifier, false, id,
                          fg_color, bg_color, focus_color, font, true, 50);
    Dimension dim = wscroll.get_optimal_dim();
    wscroll.set_wh(dim.w, 200);
    wscroll.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll6.png");

    RED_CHECK_SIG(drawable.gd, "\x5b\xfa\xfa\x3f\xb6\xd7\x8a\x15\xbe\x90\x74\xc0\x43\xa4\x65\x9d\xd9\xcc\x8f\xa8");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll7.png");

    RED_CHECK_SIG(drawable.gd, "\xb1\x8a\x53\x41\x25\x21\xb0\x34\x69\x69\xd9\xa0\x42\xd7\x12\xac\x38\x41\x91\x45");
}
