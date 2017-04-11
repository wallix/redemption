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

#define LOGNULL
//#define LOGPRINT

#include "mod/internal/widget2/scroll.hpp"
#include "mod/internal/widget2/screen.hpp"

#include "check_sig.hpp"
#include "fake_draw.hpp"

RED_AUTO_TEST_CASE(TestWidgetHScrollBar)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu_14.fv1");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int focus_color = WINBLUE;
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


    //drawable.save_to_png("scroll0.png");

    RED_CHECK_SIG(drawable.gd, "\xaf\x8d\xda\xea\xbe\x5c\x0b\x53\xf4\x28\xe3\xbc\x29\x49\xa1\x4c\x75\x7f\xc5\xbf");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    //drawable.save_to_png("scroll1.png");

    RED_CHECK_SIG(drawable.gd, "\x68\xec\xba\xe1\xbe\x41\x74\x31\x17\xd2\x89\x2c\x35\x1a\xa0\x6a\x10\x06\xa1\x9a");
}

RED_AUTO_TEST_CASE(TestWidgetVScrollBar)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu_14.fv1");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int focus_color = WINBLUE;
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


    //drawable.save_to_png("scroll2.png");

    RED_CHECK_SIG(drawable.gd, "\x35\x2b\x65\xcf\x60\x37\xc3\x4f\x19\x5e\xf5\xf7\x55\xb6\x87\x3b\x2b\x84\x52\x41");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    //drawable.save_to_png("scroll3.png");

    RED_CHECK_SIG(drawable.gd, "\x22\x74\x49\xb3\xe6\xc2\x89\x21\xf5\x98\xe0\x63\x21\xeb\x5f\xfd\xe2\xe4\x61\x02");
}

RED_AUTO_TEST_CASE(TestWidgetHScrollBarRail)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu_14.fv1");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int focus_color = WINBLUE;
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


    //drawable.save_to_png("scroll4.png");

    RED_CHECK_SIG(drawable.gd, "\x18\x49\xe8\x32\x2b\x96\x81\xd8\xc6\xd2\xf0\x56\xae\x03\x40\xb2\x18\xef\x04\x25");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    //drawable.save_to_png("scroll5.png");

    RED_CHECK_SIG(drawable.gd, "\x51\x9c\x6b\xd3\x11\x49\x22\x1c\x3e\x59\x41\x73\x93\xf7\x75\xe0\xae\x00\x09\xa8");
}

RED_AUTO_TEST_CASE(TestWidgetVScrollBarRail)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu_14.fv1");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int focus_color = WINBLUE;
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


    //drawable.save_to_png("scroll6.png");

    RED_CHECK_SIG(drawable.gd, "\x48\x75\xb9\x15\x1f\xe5\x3c\x10\x9b\x9e\x70\x42\xf4\xa4\xa2\x9f\xe1\x7b\x47\xd8");

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    //drawable.save_to_png("scroll7.png");

    RED_CHECK_SIG(drawable.gd, "\xb2\xb0\x07\x74\xcd\x55\xa1\x2d\xd2\x16\xc2\x74\x4f\xde\x50\x0c\x34\x37\x9b\x09");
}
