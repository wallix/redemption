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
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget2/label.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceWidgetLabel)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetLabel is a label widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
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
    WidgetLabel wlabel(drawable.gd, parent, notifier, "test1", id, fg_color, bg_color, font, xtext, ytext);

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png("/tmp/label0.png");

    RED_CHECK_SIG(drawable.gd, "\x95\x82\x5d\x61\x2b\x30\xe5\x65\x2f\x7e\x77\xb6\x66\x0b\x1c\xd2\x50\x01\x3f\x1d");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetLabel is a label widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    WidgetLabel wlabel(drawable.gd, parent, notifier, "test2", id, fg_color, bg_color, font);

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png("/tmp/label2.png");

    RED_CHECK_SIG(drawable.gd, "\xe5\x01\x49\x35\x8d\x89\xbb\xd5\xd3\x61\xab\x3a\x03\x97\x2c\x03\xf5\xa3\x12\x83");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetLabel is a label widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;

    WidgetLabel wlabel(drawable.gd, parent, notifier, "test3", id, fg_color, bg_color, font);

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png("/tmp/label3.png");

    RED_CHECK_SIG(drawable.gd, "\x57\x4f\x69\x68\x76\xe8\xd5\xd4\x4f\xfd\xa0\x39\x8e\xd9\x6f\x83\x4d\x21\x71\xb7");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetLabel is a label widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;

    WidgetLabel wlabel(drawable.gd, parent, notifier, "test4", id, fg_color, bg_color, font);

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png("/tmp/label4.png");

    RED_CHECK_SIG(drawable.gd, "\x40\xc5\x45\x15\xf4\x30\xad\x65\xf4\x88\x3c\xec\xdf\xdf\xf3\xcb\xd6\x5d\x34\x86");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel5)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetLabel is a label widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;

    WidgetLabel wlabel(drawable.gd, parent, notifier, "test5", id, fg_color, bg_color, font);

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png("/tmp/label5.png");

    RED_CHECK_SIG(drawable.gd, "\x3a\x3d\xc2\x64\xad\x03\x20\x54\xb5\x2e\x4c\xd9\x5a\x1b\x43\x6c\x91\xa7\x4c\xc1");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel6)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetLabel is a label widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetLabel wlabel(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, font);

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png("/tmp/label6.png");

    RED_CHECK_SIG(drawable.gd, "\xad\xb7\x7e\x8b\xd1\xc3\x99\x8d\xc7\xae\x41\x68\xb8\x67\x3e\x11\xfe\x78\x5f\xfd");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetLabel is a label widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetLabel wlabel(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, font);

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wlabel.rdp_input_invalidate(Rect(20 + wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png("/tmp/label7.png");

    RED_CHECK_SIG(drawable.gd, "\x08\x53\x1a\x07\xeb\x45\x84\xf8\x62\x54\x8e\x45\xe1\xa4\xde\x60\xe5\xf4\xd6\x5c");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetLabel is a label widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetLabel wlabel(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, font);

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wlabel.rdp_input_invalidate(Rect(20 + wlabel.x(),
                                     5 + wlabel.y(),
                                     30,
                                     10));

    //drawable.save_to_png("/tmp/label8.png");

    RED_CHECK_SIG(drawable.gd, "\x9e\x7a\x67\xb5\x0b\x65\x85\x15\x2e\xe6\xf5\x5d\x48\xd5\xbd\x92\x04\xa8\x61\x91");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelEvent)
{
    TestDraw drawable(800, 600);

    struct WidgetReceiveEvent : public Widget2 {
        Widget2* sender = nullptr;
        NotifyApi::notify_event_t event = 0;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget2(drawable.gd, *this, nullptr)
        {}

        void rdp_input_invalidate(Rect) override
        {}

        void notify(Widget2* sender, NotifyApi::notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event(drawable);

    Widget2& parent = widget_for_receive_event;
    NotifyApi * notifier = nullptr;
    int16_t x = 0;
    int16_t y = 0;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetLabel wlabel(drawable.gd, parent, notifier, "", 0, BLACK, WHITE, font);
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
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    //WidgetLabel is a label widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable.gd, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetLabel wlabel1(drawable.gd, wcomposite, notifier,
                        "abababab", 0, YELLOW, BLACK, font);
    Dimension dim = wlabel1.get_optimal_dim();
    wlabel1.set_wh(dim);
    wlabel1.set_xy(0, 0);

    WidgetLabel wlabel2(drawable.gd, wcomposite, notifier,
                        "ggghdgh", 0, WHITE, BLUE, font);
    dim = wlabel2.get_optimal_dim();
    wlabel2.set_wh(dim);
    wlabel2.set_xy(0, 100);

    WidgetLabel wlabel3(drawable.gd, wcomposite, notifier,
                        "lldlslql", 0, BLUE, RED, font);
    dim = wlabel3.get_optimal_dim();
    wlabel3.set_wh(dim);
    wlabel3.set_xy(100, 100);

    WidgetLabel wlabel4(drawable.gd, wcomposite, notifier,
                        "LLLLMLLM", 0, PINK, DARK_GREEN, font);
    dim = wlabel4.get_optimal_dim();
    wlabel4.set_wh(dim);
    wlabel4.set_xy(300, 300);

    WidgetLabel wlabel5(drawable.gd, wcomposite, notifier,
                        "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, font);
    dim = wlabel5.get_optimal_dim();
    wlabel5.set_wh(dim);
    wlabel5.set_xy(700, -10);

    WidgetLabel wlabel6(drawable.gd, wcomposite, notifier,
                        "xxwwp", 0, ANTHRACITE, PALE_GREEN, font);
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

   //drawable.save_to_png("/tmp/label9.png");

    RED_CHECK_SIG(drawable.gd, "\xe0\x70\x43\x8a\x56\xfb\x53\xb4\xa7\xab\x15\xda\x6c\x6d\x7e\xc2\xc8\xd4\xca\xac");

    //ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png("/tmp/label10.png");

    RED_CHECK_SIG(drawable.gd, "\x04\x53\x86\xe9\xcd\xab\x74\xa4\x8f\x7c\x35\x56\xd8\xb4\xb2\x85\x48\x6f\x20\x92");
    wcomposite.clear();
}

/* TODO
 * the entry point exists in module: it's rdp_input_invalidate
 * je just have to change received values to widget messages */
RED_AUTO_TEST_CASE(TraceWidgetLabelMax)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetLabel is a label widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
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

    WidgetLabel wlabel(drawable.gd, parent, notifier, text,
        id, fg_color, bg_color, font);
    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);


    RED_CHECK_EQUAL(0, memcmp(wlabel.get_text(), text, sizeof(text) - 3));

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(0 + wlabel.x(),
                                     0 + wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png("/tmp/label2.png");

    RED_CHECK_SIG(drawable.gd, "\x9c\x1b\x03\xb9\xc2\xf2\xf8\x77\xa8\x14\x50\x2c\x23\xc1\x0d\x36\xb6\x20\x10\xee");
}
