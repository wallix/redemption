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

#define RED_TEST_MODULE TestWidgetFlatButton
#include "system/redemption_unit_tests.hpp"


#include "core/font.hpp"
#include "mod/internal/widget/flat_button.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceWidgetFlatButton)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test1", id,
                             fg_color, bg_color, fc_color, 2, font, xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0, 0, wbutton.cx(), wbutton.cy()));

    // drawable.save_to_png("button.png");

    RED_CHECK_SIG(drawable.gd, "\x9f\x79\x9b\x1f\x1d\x60\xd9\xab\x92\x57\xc9\x12\x4e\x21\xde\xf2\xaf\x4f\x5d\x72");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test2", id,
                             fg_color, bg_color, fc_color, 2, font);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button2.png");

    RED_CHECK_SIG(drawable.gd, "\x6b\xeb\xa6\x54\x2d\x52\xbb\xca\x6a\x87\x2e\x90\x02\xd5\xe0\xca\x36\xc1\xc2\x6c");
}


RED_AUTO_TEST_CASE(TraceWidgetFlatButton3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test3", id,
                             fg_color, bg_color, fc_color, 2, font);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button3.png");

    RED_CHECK_SIG(drawable.gd, "\x5f\x45\xd3\x91\xc9\xbd\x7e\xb8\x2b\x35\x55\xc6\x9d\xeb\x1c\x01\x14\x66\x9e\x09");

}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test4", id,
                             fg_color, bg_color, fc_color, 2, font);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button4.png");

    RED_CHECK_SIG(drawable.gd, "\x73\xdc\x68\x7e\xdc\x25\x6d\x2d\x0f\xf3\x5d\x5a\x38\xec\x66\xb4\x6b\xb6\x20\xb4");

}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton5)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test5", id,
                             fg_color, bg_color, fc_color, 2, font);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button5.png");

    RED_CHECK_SIG(drawable.gd, "\xb9\x0b\xd7\x91\xc8\x40\x96\xd4\xbb\x4b\x80\xa7\xc4\xa4\xdb\x5c\x11\xe2\x9b\xa0");

}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton6)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test6", id,
                             fg_color, bg_color, fc_color, 2, font);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button6.png");

    RED_CHECK_SIG(drawable.gd, "\x2d\x7b\xd6\x62\x67\x59\x8d\xa0\xae\xe3\x6a\x15\xe6\xc0\x6c\x50\x03\x40\x5f\x32");

}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    BGRColor fc_color = WINBLUE;
    int16_t x = 760;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test6", id,
                             fg_color, bg_color, fc_color, 2, font);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button7.png");

    RED_CHECK_SIG(drawable.gd, "\xed\x80\x64\xa0\x50\xdf\x5d\x3c\x41\xcf\x9f\x8b\x46\x83\xe0\x5c\x1e\x4c\xd8\x37");

}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test6", id,
                             fg_color, bg_color, fc_color, 2, font);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.x(),
                                      5 + wbutton.y(),
                                      30,
                                      10));

    // drawable.save_to_png("button8.png");

    RED_CHECK_SIG(drawable.gd, "\xac\x4e\xac\xc9\xf3\x30\xaf\xdc\x58\x62\x10\xd4\xf2\x3d\xe0\x5c\xf7\x65\x46\x63");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonDownAndUp)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = 10;
    int16_t y = 10;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test6", id,
                             fg_color, bg_color, fc_color, 2, font, xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button9.png");

    RED_CHECK_SIG(drawable.gd, "\xbb\x40\x9e\x73\xc8\x3a\xfb\xa6\x4c\x89\x88\xbc\x86\x00\xfc\x31\x69\xb1\xa9\x42");


    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button10.png");

    RED_CHECK_SIG(drawable.gd, "\xad\x52\x7a\xa1\xb8\xe3\x74\x2f\x02\x01\x4e\x56\x89\x65\x5b\x82\xb3\xe9\xae\xf5");


    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 15, 15, nullptr);
    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button11.png");

    RED_CHECK_SIG(drawable.gd, "\xbb\x40\x9e\x73\xc8\x3a\xfb\xa6\x4c\x89\x88\xbc\x86\x00\xfc\x31\x69\xb1\xa9\x42");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonEvent)
{
    TestDraw drawable(800, 600);

    struct WidgetReceiveEvent : public Widget {
        Widget* sender = nullptr;
        NotifyApi::notify_event_t event = 0;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget(drawable.gd, *this, nullptr)
        {}

        void rdp_input_invalidate(Rect) override
        {}

        void notify(Widget* sender, NotifyApi::notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event(drawable);

    struct Notify : public NotifyApi {
        Widget* sender = nullptr;
        notify_event_t event = 0;

        Notify() = default;
        void notify(Widget* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Widget& parent = widget_for_receive_event;
    int16_t x = 0;
    int16_t y = 0;

    Font font;

    WidgetFlatButton wbutton(drawable.gd, parent, &notifier, "", 0, WHITE,
                             DARK_BLUE_BIS, WINBLUE, 2, font);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
    RED_CHECK(notifier.sender == &wbutton);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_KEY);
    keymap.push_char('a');
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_KEY);
    keymap.push_char(' ');
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
    RED_CHECK(notifier.sender == &wbutton);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
    RED_CHECK(notifier.sender == &wbutton);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonAndComposite)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable.gd, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetFlatButton wbutton1(drawable.gd, wcomposite, notifier,
                              "abababab", 0, YELLOW, BLACK, WINBLUE, 2, font);
    Dimension dim = wbutton1.get_optimal_dim();
    wbutton1.set_wh(dim);
    wbutton1.set_xy(0, 0);

    WidgetFlatButton wbutton2(drawable.gd, wcomposite, notifier,
                              "ggghdgh", 0, WHITE, RED, WINBLUE, 2, font);
    dim = wbutton2.get_optimal_dim();
    wbutton2.set_wh(dim);
    wbutton2.set_xy(0, 100);

    WidgetFlatButton wbutton3(drawable.gd, wcomposite, notifier,
                              "lldlslql", 0, BLUE, RED, WINBLUE, 2, font);
    dim = wbutton3.get_optimal_dim();
    wbutton3.set_wh(dim);
    wbutton3.set_xy(100, 100);

    WidgetFlatButton wbutton4(drawable.gd, wcomposite, notifier,
                              "LLLLMLLM", 0, PINK, DARK_GREEN, WINBLUE, 2, font);
    dim = wbutton4.get_optimal_dim();
    wbutton4.set_wh(dim);
    wbutton4.set_xy(300, 300);

    WidgetFlatButton wbutton5(drawable.gd, wcomposite, notifier,
                              "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, WINBLUE, 2, font);
    dim = wbutton5.get_optimal_dim();
    wbutton5.set_wh(dim);
    wbutton5.set_xy(700, -10);

    WidgetFlatButton wbutton6(drawable.gd, wcomposite, notifier,
                              "xxwwp", 0, ANTHRACITE, PALE_GREEN, WINBLUE, 2, font);
    dim = wbutton6.get_optimal_dim();
    wbutton6.set_wh(dim);
    wbutton6.set_xy(-10, 550);

    wcomposite.add_widget(&wbutton1);
    wcomposite.add_widget(&wbutton2);
    wcomposite.add_widget(&wbutton3);
    wcomposite.add_widget(&wbutton4);
    wcomposite.add_widget(&wbutton5);
    wcomposite.add_widget(&wbutton6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    // drawable.save_to_png("button12.png");

    RED_CHECK_SIG(drawable.gd, "\xbf\x5d\xc8\x01\x45\xfd\xe6\x51\x99\x50\x14\xc6\xa8\x51\x5a\xc0\x76\x6a\x09\x01");


    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png("button13.png");

    RED_CHECK_SIG(drawable.gd, "\x93\x44\x63\x92\x90\x23\x59\x45\x6e\x39\x2f\xb3\xd4\x96\xb5\x77\x5a\x08\xad\x76");


    wcomposite.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonFocus)
{
    TestDraw drawable(72, 40);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = 10;
    int16_t y = 10;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test7", id,
                             fg_color, bg_color, fc_color, 2, font, xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button14.png");

    RED_CHECK_SIG(drawable.gd, "\xa6\x44\x08\x07\x9b\x3b\xd8\xb3\xda\xe9\xd5\x34\x80\x45\x06\xa4\xd7\xdb\xc2\x73");


    wbutton.focus(Widget::focus_reason_tabkey);

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button15.png");

    RED_CHECK_SIG(drawable.gd, "\x7d\xb2\x38\x56\x95\x27\xc7\x66\x00\xf0\x46\x6a\x25\x13\x32\x33\x35\xf1\xb1\x74");



    wbutton.blur();

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button16.png");

    RED_CHECK_SIG(drawable.gd, "\xa6\x44\x08\x07\x9b\x3b\xd8\xb3\xda\xe9\xd5\x34\x80\x45\x06\xa4\xd7\xdb\xc2\x73");


    wbutton.focus(Widget::focus_reason_tabkey);

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button17.png");


    RED_CHECK_SIG(drawable.gd, "\x7d\xb2\x38\x56\x95\x27\xc7\x66\x00\xf0\x46\x6a\x25\x13\x32\x33\x35\xf1\xb1\x74");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonLite)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test1", id,
                             fg_color, bg_color, fc_color, 1, font, xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(0, 0);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));

    // drawable.save_to_png("button_lite.png");

    RED_CHECK_SIG(drawable.gd, "\xcf\xf2\x06\x4d\xf4\xd6\xff\x68\x0d\x1d\xf1\x76\x11\x47\xfc\x77\xa3\xfa\xaa\x6f");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, wbutton.x() + 2, wbutton.y() + 2, nullptr);

    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));

    // drawable.save_to_png("button_lite_down.png");

    RED_CHECK_SIG(drawable.gd, "\x0d\xd2\x95\x8b\x12\x8b\xcd\x69\xbb\xab\x63\xa8\x34\x67\x31\x26\xf6\xa3\xe0\x94");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonStrong)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable.gd, parent, notifier, "test1", id,
                             fg_color, bg_color, fc_color, 5, font, xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(0, 0);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));

    // drawable.save_to_png("button_lite.png");

    RED_CHECK_SIG(drawable.gd, "\x1d\xb7\x0f\x89\x54\x13\xd8\x7a\x43\x3c\xa8\x31\xfc\x32\x59\x58\x0e\xaa\x00\x0d");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, wbutton.x() + 2, wbutton.y() + 2, nullptr);

    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));

    // drawable.save_to_png("button_lite_down.png");

    RED_CHECK_SIG(drawable.gd, "\x46\x5e\x2c\x13\xf5\xd5\x44\xdf\x56\x3f\xc5\x78\x0b\x5f\x37\x6f\xe3\x1f\xa9\xd5");
}
