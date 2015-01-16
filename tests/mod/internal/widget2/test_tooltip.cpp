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

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL

#include "config.hpp"
#include "internal/widget2/tooltip.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/label.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetTooltip)
{
    TestDraw drawable(800, 600);

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetTooltip is a tooltip widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.font);

    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int border_color = BLACK;
    int16_t x = 10;
    int16_t y = 10;
    const char * tooltiptext = "testémq";

    WidgetTooltip wtooltip(drawable, x, y, parent, notifier, tooltiptext, fg_color, bg_color, border_color, ini.font);

    // ask to widget to redraw
    wtooltip.rdp_input_invalidate(Rect(0, 0, 100, 100));

    // drawable.save_to_png(OUTPUT_FILE_PATH "tooltip.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x10\x6b\x61\x5c\x34\xa3\xf7\x26\xf7\x97"
        "\xe7\xfb\x46\x96\x79\x3d\x96\x64\xff\x65"
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

}

BOOST_AUTO_TEST_CASE(TraceWidgetTooltipScreen)
{
    TestDraw drawable(800, 600);
    int x = 50;
    int y = 20;

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetTooltip is a tooltip widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.font);

    WidgetLabel label(drawable, x, y, parent, &parent, "TOOLTIPTEST",
                      true, 0, BLACK, WHITE, ini.font);
    WidgetLabel label2(drawable, x + 50, y + 90, parent, &parent, "TOOLTIPTESTMULTI",
                      true, 0, BLACK, WHITE, ini.font);

    parent.add_widget(&label);
    parent.add_widget(&label2);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "tooltipscreen1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x99\x76\xb1\xfe\x46\x2b\xb6\x12\x84\x7e"
        "\xf8\x34\x09\x1a\xaf\x2f\xc3\x9c\x64\x04"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    rdp_input_mouse(MOUSE_FLAG_MOVE,
                    label.centerx(), label.centery(),
                    NULL, &parent, &label, "Test tooltip description");
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "tooltipscreen2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xa9\xc9\x59\x0c\xc2\xbe\x77\x9d\x43\xef"
        "\xac\x06\xe2\xc0\xac\xe8\xe7\x0a\xbf\x2c"
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

    if (!check_sig(drawable.gd.impl(), message,
        "\xac\x4c\xe6\x8f\x7d\xff\x72\x10\x31\x19"
        "\x9f\xcc\xe7\xba\x39\xf2\x33\x0d\xef\x93"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.tooltip->set_text("Test tooltip<br>"
                             "Text modification<br>"
                             "text has been changed !");
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "tooltipscreen4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x94\x59\x4f\xd6\x69\xe6\xea\xbb\x3c\x1c"
        "\xfa\x84\xbf\x0f\xbf\x38\x18\xd4\x9a\xef"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.clear();
}
