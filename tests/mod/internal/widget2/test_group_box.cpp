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
#define BOOST_TEST_MODULE TestWidgetGroupBox
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/group_box.hpp"
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

BOOST_AUTO_TEST_CASE(TraceWidgetGroupBox)
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
    uint16_t    cx       = 150;
    uint16_t    cy       = 100;

    TODO("I believe users of this widget may wish to control text position and behavior inside rectangle"
         "ie: text may be centered, aligned left, aligned right, or even upside down, etc"
         "these possibilities (and others) are supported in RDPGlyphIndex")
    WidgetGroupBox wgroupbox(drawable, x, y, cx, cy, parent, notifier, "Group 1", group_id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wgroupbox.rdp_input_invalidate(Rect( wgroupbox.dx()
                                       , wgroupbox.dy()
                                       , wgroupbox.cx()
                                       , wgroupbox.cy()
                                       ));

    //drawable.save_to_png(OUTPUT_FILE_PATH "group_box.png");

    char message[1024];
    if (!check_sig( drawable.gd.drawable, message
                  , "\x04\xbc\x9c\x07\x23\x7e\x03\x2d\x28\x80"
                    "\x50\x26\x24\x95\xc9\x45\x28\x5c\x33\x7d")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
}
