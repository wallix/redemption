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
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/flat_button.hpp"
#include "mod/internal/widget/group_box.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TraceWidgetGroupBox)
{
    TestGraphic drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
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
    WidgetGroupBox wgroupbox( drawable, parent, notifier, "Group 1"
                            , fg_color, bg_color, global_font_lato_light_16());
    wgroupbox.set_wh(cx, cy);
    wgroupbox.set_xy(x, y);

    BGRColor focuscolor  = LIGHT_YELLOW;
    int  xtext       = 4;
    int  ytext       = 1;
    WidgetFlatButton wbutton(drawable, wgroupbox, notifier, "Button 1",
                             group_id, fg_color, bg_color, focuscolor, 2, global_font_lato_light_16(),
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

    // drawable.save_to_png("group_box_0.png");

    RED_CHECK_SIG(drawable, "\x09\x6c\x32\xc4\x31\xe6\x53\x33\x8b\x19\x56\x34\x5e\xe3\xc5\x63\xbf\x1e\x8a\xf2");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN,
                            wbutton.x() + 1, wbutton.y() + 1,
                            nullptr);

    // ask to widget to redraw at it's current position
    wgroupbox.rdp_input_invalidate(Rect( wgroupbox.x()
                                       , wgroupbox.y()
                                       , wgroupbox.cx()
                                       , wgroupbox.cy()
                                       ));

    // drawable.save_to_png("group_box_1.png");

    RED_CHECK_SIG(drawable, "\x57\x7c\x4a\xcf\x93\x04\x71\xec\x56\x1c\xed\x4f\x65\x94\x03\x51\x44\x5d\x23\x31");
}

RED_AUTO_TEST_CASE(TraceWidgetGroupBoxMax)
{
    TestGraphic drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
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
    WidgetGroupBox wgroupbox( drawable, parent, notifier, text
                            , fg_color, bg_color, global_font_lato_light_16());
    wgroupbox.set_wh(cx, cy);
    wgroupbox.set_xy(x, y);

    RED_CHECK_EQUAL(0, memcmp(wgroupbox.get_text(), text, sizeof(text) - 3));

    BGRColor focuscolor  = LIGHT_YELLOW;
    int  xtext       = 4;
    int  ytext       = 1;
    WidgetFlatButton wbutton(drawable, wgroupbox, notifier, "Button 1",
                             group_id, fg_color, bg_color, focuscolor, 2, global_font_lato_light_16(),
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

    // drawable.save_to_png("group_box_2.png");

    RED_CHECK_SIG(drawable, "\xcf\x03\x06\xa1\x36\x78\x17\x19\x41\xa8\x0e\x15\xdb\x68\xdf\x5e\x15\x63\xf5\x56");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN,
                            wbutton.x() + 1, wbutton.y() + 1,
                            nullptr);

    // ask to widget to redraw at it's current position
    wgroupbox.rdp_input_invalidate(Rect( wgroupbox.x()
                                       , wgroupbox.y()
                                       , wgroupbox.cx()
                                       , wgroupbox.cy()
                                       ));

    // drawable.save_to_png("group_box_3.png");

    RED_CHECK_SIG(drawable, "\x12\xc4\x69\xb5\xe0\x87\xed\x94\x44\xbf\x6a\x07\x41\x6a\x45\xc8\x23\x70\xc8\xab");
}
