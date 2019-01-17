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

#define RED_TEST_MODULE TestWidgetTooltip
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/tooltip.hpp"
#include "mod/internal/widget/screen.hpp"
#include "mod/internal/widget/label.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TraceWidgetTooltip)
{
    TestGraphic drawable(800, 600);

    // WidgetTooltip is a tooltip widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor border_color = BLACK;
    int16_t x = 10;
    int16_t y = 10;
    const char * tooltiptext = "testémq";

    WidgetTooltip wtooltip(drawable, parent, notifier, tooltiptext, fg_color, bg_color, border_color, global_font_lato_light_16());
    Dimension dim = wtooltip.get_optimal_dim();
    wtooltip.set_wh(dim);
    wtooltip.set_xy(x, y);

    // ask to widget to redraw
    wtooltip.rdp_input_invalidate(Rect(0, 0, 100, 100));

    // drawable.save_to_png("tooltip.png");

    RED_CHECK_SIG(drawable, "\xb8\x16\x68\x4c\x37\x49\x4b\xca\xea\x46\xc9\x95\x75\x82\x2b\xd4\xa9\x4a\x92\x7b");
}

inline
void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap, WidgetScreen * parent, Widget * w, const char * text)
{
    parent->rdp_input_mouse(device_flags, x, y, keymap);
    if (device_flags == MOUSE_FLAG_MOVE) {
        Widget * wid = parent->widget_at_pos(x, y);
        if (wid == w) {
            parent->show_tooltip(w, text, x, y, Rect(0, 0, 0, 0));
        }
    }

}

RED_AUTO_TEST_CASE(TraceWidgetTooltipScreen)
{
    TestGraphic drawable(800, 600);
    int x = 50;
    int y = 20;

    // WidgetTooltip is a tooltip widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    WidgetLabel label(drawable, parent, &parent, "TOOLTIPTEST",
                      0, BLACK, WHITE, global_font_lato_light_16());
    Dimension dim = label.get_optimal_dim();
    label.set_wh(dim);
    label.set_xy(x, y);

    WidgetLabel label2(drawable, parent, &parent, "TOOLTIPTESTMULTI",
                      0, BLACK, WHITE, global_font_lato_light_16());
    dim = label2.get_optimal_dim();
    label2.set_wh(dim);
    label2.set_xy(x + 50, y + 90);

    parent.add_widget(&label);
    parent.add_widget(&label2);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));

    // drawable.save_to_png("tooltipscreen1.png");

    RED_CHECK_SIG(drawable, "\xb9\x45\xcd\x0a\xb0\xe2\x0a\x5f\x0b\xc4\xed\x1c\x99\x57\x5c\xbb\xbc\x4b\x2b\xaf");

    rdp_input_mouse(MOUSE_FLAG_MOVE,
                    label.x() + label.cx() / 2, label.y() + label.cy() / 2,
                    nullptr, &parent, &label, "Test tooltip description");
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("tooltipscreen2.png");

    RED_CHECK_SIG(drawable, "\x73\x16\xd3\xa6\xa9\x00\xaa\xac\x08\xce\xd1\x9f\xf7\xf1\x48\x91\xcc\x42\xcd\x66");

    rdp_input_mouse(MOUSE_FLAG_MOVE,
                    label2.x() + label2.cx() / 2, label2.y() + label2.cy() / 2,
                    nullptr, &parent, &label2,
                    "Test tooltip<br>"
                    "description in<br>"
                    "multilines !");

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("tooltipscreen3.png");

    RED_CHECK_SIG(drawable, "\x10\x22\xd9\xed\x87\x2a\x02\xbf\x64\x3d\xcf\x89\x3a\x65\xab\x53\xf9\x2c\x25\x43");

    parent.tooltip->set_text("Test tooltip<br>"
                             "Text modification<br>"
                             "text has been changed !");
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("tooltipscreen4.png");

    RED_CHECK_SIG(drawable, "\x40\x98\x69\xe9\xd8\xb9\x15\x60\x8f\xc1\xee\x0e\xd4\x2f\xb3\x63\xd9\xf8\xdf\x09");

    parent.clear();
}
