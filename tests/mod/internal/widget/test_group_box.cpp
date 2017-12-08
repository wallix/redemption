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

#define RED_TEST_MODULE TestWidgetGroupBox
#include "system/redemption_unit_tests.hpp"


#include "core/font.hpp"
#include "mod/internal/widget/flat_button.hpp"
#include "mod/internal/widget/group_box.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceWidgetGroupBox)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor   fg_color = RED;
    BGRColor   bg_color = YELLOW;
    int         group_id = 0;
    int16_t     x        = 200;
    int16_t     y        = 100;
    uint16_t    cx       = 150;
    uint16_t    cy       = 200;

    /* TODO
     * I believe users of this widget may wish to control text position and behavior inside rectangle
     * ie: text may be centered, aligned left, aligned right, or even upside down, etc
     * these possibilities (and others) are supported in RDPGlyphIndex */
    WidgetGroupBox wgroupbox( drawable.gd, parent, notifier, "Group 1"
                            , fg_color, bg_color, font);
    wgroupbox.set_wh(cx, cy);
    wgroupbox.set_xy(x, y);

    BGRColor focuscolor  = LIGHT_YELLOW;
    int  xtext       = 4;
    int  ytext       = 1;
    WidgetFlatButton wbutton(drawable.gd, wgroupbox, notifier, "Button 1",
                             group_id, fg_color, bg_color, focuscolor, 2, font,
                             xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x + 10, y + 20);

    wgroupbox.add_widget(&wbutton);

    // ask to widget to redraw at it's current position
    wgroupbox.rdp_input_invalidate(Rect( wgroupbox.x()
                                       , wgroupbox.y()
                                       , wgroupbox.cx()
                                       , wgroupbox.cy()
                                       ));

    // drawable.save_to_png(OUTPUT_FILE_PATH "group_box_0.png");

    RED_CHECK_SIG(drawable.gd, "\xb3\x0e\x54\x67\x9d\xfd\x8b\x9b\x15\x83\x31\xa7\x89\x30\x95\x96\x4d\xfb\x55\x5a");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN,
                            wbutton.x() + 1, wbutton.y() + 1,
                            nullptr);

    // ask to widget to redraw at it's current position
    wgroupbox.rdp_input_invalidate(Rect( wgroupbox.x()
                                       , wgroupbox.y()
                                       , wgroupbox.cx()
                                       , wgroupbox.cy()
                                       ));

    // drawable.save_to_png(OUTPUT_FILE_PATH "group_box_1.png");

    RED_CHECK_SIG(drawable.gd, "\xe4\xa4\xa1\xb8\x91\x71\x0e\xf4\xcd\xe5\xe2\x5e\x79\xce\xb7\x4f\x50\xf8\xd6\xe9");
}

RED_AUTO_TEST_CASE(TraceWidgetGroupBoxMax)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor   fg_color = RED;
    BGRColor   bg_color = YELLOW;
    int         group_id = 0;
    int16_t     x        = 200;
    int16_t     y        = 100;
    uint16_t    cx       = 150;
    uint16_t    cy       = 200;

    char text[] = "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
                  "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
                  "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
                  "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà";

    /* TODO
     * I believe users of this widget may wish to control text position and behavior inside rectangle
     * ie: text may be centered, aligned left, aligned right, or even upside down, etc
     * these possibilities (and others) are supported in RDPGlyphIndex */
    WidgetGroupBox wgroupbox( drawable.gd, parent, notifier, text
                            , fg_color, bg_color, font);
    wgroupbox.set_wh(cx, cy);
    wgroupbox.set_xy(x, y);

    RED_CHECK_EQUAL(0, memcmp(wgroupbox.get_text(), text, sizeof(text) - 3));

    BGRColor focuscolor  = LIGHT_YELLOW;
    int  xtext       = 4;
    int  ytext       = 1;
    WidgetFlatButton wbutton(drawable.gd, wgroupbox, notifier, "Button 1",
                             group_id, fg_color, bg_color, focuscolor, 2, font,
                             xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x + 10, y + 20);

    wgroupbox.add_widget(&wbutton);

    // ask to widget to redraw at it's current position
    wgroupbox.rdp_input_invalidate(Rect( wgroupbox.x()
                                       , wgroupbox.y()
                                       , wgroupbox.cx()
                                       , wgroupbox.cy()
                                       ));

    // drawable.save_to_png(OUTPUT_FILE_PATH "group_box_2.png");

    RED_CHECK_SIG(drawable.gd, "\x3d\x38\x8c\x1e\x0f\x38\x18\xa1\x4e\x40\xe8\x9f\x1c\xe9\x80\x0b\x60\x60\x39\xb8");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN,
                            wbutton.x() + 1, wbutton.y() + 1,
                            nullptr);

    // ask to widget to redraw at it's current position
    wgroupbox.rdp_input_invalidate(Rect( wgroupbox.x()
                                       , wgroupbox.y()
                                       , wgroupbox.cx()
                                       , wgroupbox.cy()
                                       ));

    // drawable.save_to_png(OUTPUT_FILE_PATH "group_box_3.png");

    RED_CHECK_SIG(drawable.gd, "\x20\x96\x42\x3e\xfa\xff\x1a\x98\x75\x2a\x5c\x0a\x41\x6d\xe8\xa1\x66\x4e\x93\xe3");
}
