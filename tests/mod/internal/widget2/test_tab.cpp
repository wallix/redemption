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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetTab
#include <boost/test/auto_unit_test.hpp>

//#define LOGNULL
#define LOGPRINT
#include "log.hpp"

#include "internal/widget2/flat_button.hpp"
#include "internal/widget2/tab.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/composite.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"
#include "fake_draw.hpp"

#ifndef FIXTURES_PATH
#define FIXTURES_PATH
#endif

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "./"

BOOST_AUTO_TEST_CASE(TraceWidgetTab)
{
    TestDraw drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);


    NotifyApi * notifier = NULL;
    int         fg_color = RED;
    int         bg_color = YELLOW;
    int         group_id = 0;
    int16_t     x        = 200;
    int16_t     y        = 100;
    uint16_t    cx       = 300;
    uint16_t    cy       = 200;

    WidgetTabDPDefault drawing_policy(drawable);

    TODO("I believe users of this widget may wish to control text position and behavior inside rectangle"
         "ie: text may be centered, aligned left, aligned right, or even upside down, etc"
         "these possibilities (and others) are supported in RDPGlyphIndex")
    WidgetTab wtab( drawable, drawing_policy, x, y, cx, cy, parent, notifier, group_id
                  , fg_color, bg_color);

    size_t tab_0_index = static_cast<size_t>(-1);
    size_t tab_1_index = static_cast<size_t>(-1);

    BOOST_CHECK((tab_0_index = wtab.add_item("First tab")) == 0);
    BOOST_CHECK((tab_1_index = wtab.add_item("Second tab")) == 1);

    bool auto_resize = true;
    int  focuscolor  = LIGHT_YELLOW;
    int  xtext       = 4;
    int  ytext       = 1;

    WidgetFlatButton wbutton_on_first_tab( drawable, 10, 20, wtab.get_item(tab_0_index), notifier
                            , "Button on First tab", auto_resize, group_id, fg_color, bg_color
                            , focuscolor, xtext, ytext);
    wtab.add_widget(tab_0_index, &wbutton_on_first_tab);

    WidgetFlatButton wbutton_on_second_tab( drawable, 10, 20, wtab.get_item(tab_1_index), notifier
                            , "Button on second tab", auto_resize, group_id, fg_color, bg_color
                            , focuscolor, xtext, ytext);
    wtab.add_widget(tab_1_index, &wbutton_on_second_tab);


    wtab.set_current_item(tab_1_index);

    // ask to widget to redraw at it's current position
    wtab.rdp_input_invalidate(Rect( wtab.dx()
                                  , wtab.dy()
                                  , wtab.cx()
                                  , wtab.cy()
                                  ));

    drawable.save_to_png(OUTPUT_FILE_PATH "tab_0.png");

    char message[1024];
    if (!check_sig( drawable.gd.drawable, message
                  , "\x3a\x68\x71\xa0\x6d\x23\xc2\xd5\xf8\xd5"
                    "\x41\xdf\x41\x2f\x60\xbc\x03\x1e\xaf\xd7")) {
//        BOOST_CHECK_MESSAGE(false, message);
    }


    int16_t mouse_x;
    int16_t mouse_y;


    // mouse_x = x + 2;
    // mouse_y = y;
    mouse_x = x + 62;
    mouse_y = y + 22;
    wtab.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                        , mouse_x + 2
                        , mouse_y
                        , NULL);

    // ask to widget to redraw at it's current position
    wtab.rdp_input_invalidate(Rect( wtab.dx()
                                  , wtab.dy()
                                  , wtab.cx()
                                  , wtab.cy()
                                  ));

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "tab_1.png");

    if (!check_sig( drawable.gd.drawable, message
                  , "\x2d\xb0\xd3\x13\xc9\x3e\xc1\x62\xa7\x43"
                    "\x0c\x1c\x3a\x7a\x7e\xb1\x54\x22\x49\xd1")) {
//        BOOST_CHECK_MESSAGE(false, message);
    }


    mouse_x = x + 144;
    mouse_y = y;
    wtab.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                        , mouse_x + 2
                        , mouse_y
                        , NULL);

    // ask to widget to redraw at it's current position
    wtab.rdp_input_invalidate(Rect( wtab.dx()
                                  , wtab.dy()
                                  , wtab.cx()
                                  , wtab.cy()
                                  ));

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "tab_2.png");

    if (!check_sig( drawable.gd.drawable, message
                  , "\x2d\xb0\xd3\x13\xc9\x3e\xc1\x62\xa7\x43"
                    "\x0c\x1c\x3a\x7a\x7e\xb1\x54\x22\x49\xd1")) {
//        BOOST_CHECK_MESSAGE(false, message);
    }
}
