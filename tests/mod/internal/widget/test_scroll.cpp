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
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/scroll.hpp"
#include "mod/internal/widget/screen.hpp"

#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TestWidgetHScrollBar)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor focus_color = WINBLUE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetScrollBar wscroll(drawable, parent, notifier, true, id,
                          fg_color, bg_color, focus_color, global_font_deja_vu_14(), false, 50);
    Dimension dim = wscroll.get_optimal_dim();
    wscroll.set_wh(200, dim.h);
    wscroll.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll0.png");

    RED_CHECK_SIG(drawable, "\xc8\x76\xf6\xc8\x85\x66\x7f\x41\x67\x75\x9f\xeb\x9a\xb0\x3a\x64\x7a\xab\xc8\x8e");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    // drawable.save_to_png("scroll1.png");

    RED_CHECK_SIG(drawable, "\x16\xf5\xef\x5c\x43\x84\x7d\x3c\x4d\x6d\x33\x3e\xb1\x47\x5f\x58\xf0\x43\x86\xab");
}

RED_AUTO_TEST_CASE(TestWidgetVScrollBar)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

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
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

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
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

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
