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

#include "mod/internal/widget/flat_button.hpp"
#include "mod/internal/widget/group_box.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/group_box/"

RED_AUTO_TEST_CASE(TraceWidgetGroupBox)
{
    TestGraphic drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "group_box_1.png");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN,
                            wbutton.x() + 1, wbutton.y() + 1,
                            nullptr);

    // ask to widget to redraw at it's current position
    wgroupbox.rdp_input_invalidate(Rect( wgroupbox.x()
                                       , wgroupbox.y()
                                       , wgroupbox.cx()
                                       , wgroupbox.cy()
                                       ));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "group_box_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetGroupBoxMax)
{
    TestGraphic drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor   fg_color = RED;
    BGRColor   bg_color = YELLOW;
    int         group_id = 0;
    int16_t     x        = 200;
    int16_t     y        = 100;
    uint16_t    cx       = 150;
    uint16_t    cy       = 200;

    auto text =
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"_av;

    /* TODO
     * I believe users of this widget may wish to control text position and behavior inside rectangle
     * ie: text may be centered, aligned left, aligned right, or even upside down, etc
     * these possibilities (and others) are supported in RDPGlyphIndex */
    WidgetGroupBox wgroupbox( drawable, parent, notifier, text.data()
                            , fg_color, bg_color, global_font_lato_light_16());
    wgroupbox.set_wh(cx, cy);
    wgroupbox.set_xy(x, y);

    RED_CHECK(std::string_view(wgroupbox.get_text()) == text.first(text.size()-2));

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "group_box_3.png");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN,
                            wbutton.x() + 1, wbutton.y() + 1,
                            nullptr);

    // ask to widget to redraw at it's current position
    wgroupbox.rdp_input_invalidate(Rect( wgroupbox.x()
                                       , wgroupbox.y()
                                       , wgroupbox.cx()
                                       , wgroupbox.cy()
                                       ));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "group_box_4.png");
}
