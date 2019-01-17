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

#define RED_TEST_MODULE TestWidgetLabel
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "keyboard/keymap2.hpp"
#include "mod/internal/widget/label.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TraceWidgetLabel)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    /* TODO
     * I believe users of this widget may wish to control text position and behavior inside rectangle
     * ie: text may be centered, aligned left, aligned right, or even upside down, etc
     * these possibilities (and others) are supported in RDPGlyphIndex */
    WidgetLabel wlabel(drawable, parent, notifier, "test1", id, fg_color, bg_color, global_font_lato_light_16(), xtext, ytext);

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    // drawable.save_to_png("label0.png");

    RED_CHECK_SIG(drawable, "\x1c\xa5\x6d\xfa\x82\xb1\x50\xf9\x35\x11\xd1\x45\xfa\x60\x51\x61\x79\x41\xe0\x65");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel2)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    WidgetLabel wlabel(drawable, parent, notifier, "test2", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    // drawable.save_to_png("label2.png");

    RED_CHECK_SIG(drawable, "\x99\x61\xf8\x93\x28\x4f\x7b\x21\xc2\xfb\x73\x45\xff\x2c\xfd\xc2\xf7\x4d\xaf\x5c");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel3)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;

    WidgetLabel wlabel(drawable, parent, notifier, "test3", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    // drawable.save_to_png("label3.png");

    RED_CHECK_SIG(drawable, "\x10\xb4\xcb\x22\x6b\x71\x3d\x83\x3d\x5f\x05\xe6\xf1\xc7\xb4\x25\x19\x90\xa4\xd2");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel4)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;

    WidgetLabel wlabel(drawable, parent, notifier, "test4", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    // drawable.save_to_png("label4.png");

    RED_CHECK_SIG(drawable, "\x8b\x75\xdc\x26\xd7\xc0\x59\x37\x5e\x4d\x82\x48\x1e\x74\xf7\x80\xf2\xbd\x51\x88");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel5)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;

    WidgetLabel wlabel(drawable, parent, notifier, "test5", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    // drawable.save_to_png("label5.png");

    RED_CHECK_SIG(drawable, "\x4a\x25\x32\xea\xe2\x7e\x69\xb0\xb5\x87\xad\x51\x77\x62\xf2\x36\x05\x5e\xde\x20");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel6)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetLabel wlabel(drawable, parent, notifier, "test6", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    // drawable.save_to_png("label6.png");

    RED_CHECK_SIG(drawable, "\x46\xcf\x3a\x61\x8c\x3d\xde\xe7\x51\xe2\xae\xd9\xa3\x17\x84\x74\x11\x4c\x25\xc5");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelClip)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetLabel wlabel(drawable, parent, notifier, "test6", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wlabel.rdp_input_invalidate(Rect(20 + wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    // drawable.save_to_png("label7.png");

    RED_CHECK_SIG(drawable, "\xc7\x5a\x05\x6b\x04\x7b\x9c\x23\xcb\xc6\x14\xcc\xc4\x24\xb5\x1a\x72\x66\x11\xf2");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelClip2)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetLabel wlabel(drawable, parent, notifier, "test6", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wlabel.rdp_input_invalidate(Rect(20 + wlabel.x(),
                                     5 + wlabel.y(),
                                     30,
                                     10));

    // drawable.save_to_png("label8.png");

    RED_CHECK_SIG(drawable, "\x29\x13\x71\xa7\x9f\x72\x9b\x97\x94\x24\x41\xe4\x38\x15\xde\x6c\x80\x79\x68\x24");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelEvent)
{
    TestGraphic drawable(800, 600);

    struct WidgetReceiveEvent : public Widget {
        Widget* sender = nullptr;
        NotifyApi::notify_event_t event = 0;

        WidgetReceiveEvent(TestGraphic& drawable)
        : Widget(drawable, *this, nullptr)
        {}

        void rdp_input_invalidate(Rect) override
        {}

        void notify(Widget* sender, NotifyApi::notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event(drawable);

    Widget& parent = widget_for_receive_event;
    NotifyApi * notifier = nullptr;
    int16_t x = 0;
    int16_t y = 0;

    WidgetLabel wlabel(drawable, parent, notifier, "", 0, BLACK, WHITE, global_font_lato_light_16());
    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    wlabel.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 0, 0, nullptr);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
    wlabel.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 0, 0, nullptr);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_char('a');

    wlabel.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
}

RED_AUTO_TEST_CASE(TraceWidgetLabelAndComposite)
{
    TestGraphic drawable(800, 600);

    //WidgetLabel is a label widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetLabel wlabel1(drawable, wcomposite, notifier,
                        "abababab", 0, YELLOW, BLACK, global_font_lato_light_16());
    Dimension dim = wlabel1.get_optimal_dim();
    wlabel1.set_wh(dim);
    wlabel1.set_xy(0, 0);

    WidgetLabel wlabel2(drawable, wcomposite, notifier,
                        "ggghdgh", 0, WHITE, BLUE, global_font_lato_light_16());
    dim = wlabel2.get_optimal_dim();
    wlabel2.set_wh(dim);
    wlabel2.set_xy(0, 100);

    WidgetLabel wlabel3(drawable, wcomposite, notifier,
                        "lldlslql", 0, BLUE, RED, global_font_lato_light_16());
    dim = wlabel3.get_optimal_dim();
    wlabel3.set_wh(dim);
    wlabel3.set_xy(100, 100);

    WidgetLabel wlabel4(drawable, wcomposite, notifier,
                        "LLLLMLLM", 0, PINK, DARK_GREEN, global_font_lato_light_16());
    dim = wlabel4.get_optimal_dim();
    wlabel4.set_wh(dim);
    wlabel4.set_xy(300, 300);

    WidgetLabel wlabel5(drawable, wcomposite, notifier,
                        "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, global_font_lato_light_16());
    dim = wlabel5.get_optimal_dim();
    wlabel5.set_wh(dim);
    wlabel5.set_xy(700, -10);

    WidgetLabel wlabel6(drawable, wcomposite, notifier,
                        "xxwwp", 0, ANTHRACITE, PALE_GREEN, global_font_lato_light_16());
    dim = wlabel6.get_optimal_dim();
    wlabel6.set_wh(dim);
    wlabel6.set_xy(-10, 550);

    wcomposite.add_widget(&wlabel1);
    wcomposite.add_widget(&wlabel2);
    wcomposite.add_widget(&wlabel3);
    wcomposite.add_widget(&wlabel4);
    wcomposite.add_widget(&wlabel5);
    wcomposite.add_widget(&wlabel6);

    //ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));
//    wcomposite.rdp_input_invalidate(Rect(0, 0, 800, 600));

    // drawable.save_to_png("label9.png");

    RED_CHECK_SIG(drawable, "\xa0\x72\xf8\x43\xa4\xdb\x86\x14\x27\x57\x0e\x60\x7e\x80\xde\xa1\xab\x33\xf2\x01");

    //ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png("label10.png");

    RED_CHECK_SIG(drawable, "\xcd\x6d\x51\x8b\x2e\xab\x2f\xdf\x48\x8c\x87\xde\xb9\x49\xc5\x64\xfc\xc7\xf0\xf5");
    wcomposite.clear();
}

/* TODO
 * the entry point exists in module: it's rdp_input_invalidate
 * je just have to change received values to widget messages */
RED_AUTO_TEST_CASE(TraceWidgetLabelMax)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    char text[] = "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
                  "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
                  "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
                  "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà";

    WidgetLabel wlabel(drawable, parent, notifier, text,
        id, fg_color, bg_color, global_font_lato_light_16());
    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);


    RED_CHECK_EQUAL(0, memcmp(wlabel.get_text(), text, sizeof(text) - 3));

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(0 + wlabel.x(),
                                     0 + wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    // drawable.save_to_png("label2.png");

    RED_CHECK_SIG(drawable, "\xd9\xd1\xc5\x5e\x61\x3a\x1c\x0c\xa5\xcf\x58\x33\x80\x66\x76\xe0\xbf\xdc\x6f\x58");
}
