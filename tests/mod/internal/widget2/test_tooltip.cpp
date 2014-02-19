/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetTooltip
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/tooltip.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/label.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif
#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetTooltip)
{
    TestDraw drawable(800, 600);

    // WidgetTooltip is a tooltip widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int16_t x = 10;
    int16_t y = 10;
    const char * tooltiptext = "testémq";

    WidgetTooltip wtooltip(drawable, x, y, parent, notifier, tooltiptext, fg_color, bg_color);

    // ask to widget to redraw
    wtooltip.rdp_input_invalidate(Rect(0, 0, 100, 100));

    // drawable.save_to_png(OUTPUT_FILE_PATH "tooltip.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x31\x91\x81\x07\x31\x0b\x13\x90\x2c\x97"
                   "\x98\xb3\x52\x65\x92\xf8\x08\x42\xc2\xd5"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap, WidgetScreen * parent, Widget2 * w, const char * text)
{
    parent->rdp_input_mouse(device_flags, x, y, keymap);
    if (device_flags == MOUSE_FLAG_MOVE) {
        Widget2 * wid = parent->widget_at_pos(x, y);
        if (wid == w) {
            parent->show_tooltip(w, text, x, y);
        }
    }

};

BOOST_AUTO_TEST_CASE(TraceWidgetTooltipScreen)
{
    TestDraw drawable(800, 600);
    int x = 50;
    int y = 20;
    // WidgetTooltip is a tooltip widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    WidgetLabel label(drawable, x, y, parent, &parent, "TOOLTIPTEST",
                      true, 0, BLACK, WHITE);
    WidgetLabel label2(drawable, x + 50, y + 90, parent, &parent, "TOOLTIPTESTMULTI",
                      true, 0, BLACK, WHITE);

    parent.add_widget(&label);
    parent.add_widget(&label2);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "tooltipscreen1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x87\x22\x76\x5e\xc5\x6b\x02\x4f\xa6\xc9"
                   "\x60\x32\xbd\x64\x6e\x5c\xbd\x80\x16\xa2"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    rdp_input_mouse(MOUSE_FLAG_MOVE,
                    label.centerx(), label.centery(),
                    NULL, &parent, &label, "Test tooltip description");
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "tooltipscreen2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x38\x6d\x83\x65\xc3\x76\x03\x32\xb2\x18"
                   "\xe8\x9c\x7d\xd1\xcb\xa1\xed\xa8\x61\x6a"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    rdp_input_mouse(MOUSE_FLAG_MOVE,
                    label2.centerx(), label2.centery(),
                    NULL, &parent, &label2,
                    "Test tooltip<br>"
                    "description in<br>"
                    "multilines !");

    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "tooltipscreen3.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x9d\x9a\xca\x54\xc2\x12\x21\x7d\x04\x94"
                   "\xa5\x4d\x42\x0c\x42\xd6\xd2\x7c\x3f\x26"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.tooltip->set_text("Test tooltip<br>"
                             "Text modification<br>"
                             "text has been changed !");
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "tooltipscreen4.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xf1\x95\xb1\x6f\x27\xaf\x57\xa5\xa1\x2a"
                   "\xb8\x93\x59\x98\xdb\x02\x23\x64\x74\x50"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.clear();
}
