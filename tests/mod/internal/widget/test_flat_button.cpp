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
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/flat_button.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/core/font.hpp"


RED_AUTO_TEST_CASE(TraceWidgetFlatButton)
{
    TestGraphic drawable(800, 600);

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
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

    WidgetFlatButton wbutton(drawable, parent, notifier, "test1", id,
                             fg_color, bg_color, fc_color, 2, global_font_deja_vu_14(), xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0, 0, wbutton.cx(), wbutton.cy()));

    // drawable.save_to_png("button.png");

    RED_CHECK_SIG(drawable, "\xdd\xc2\xb2\x1e\x8d\x45\x6c\xd9\xef\x05\x26\xd5\x4a\x80\xec\xaf\x9c\x1b\xb1\xaf");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton2)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    WidgetFlatButton wbutton(drawable, parent, notifier, "test2", id,
                             fg_color, bg_color, fc_color, 2, global_font_deja_vu_14());
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button2.png");

    RED_CHECK_SIG(drawable, "\xca\x1b\x47\xa5\x8f\xf0\x39\xad\xb6\x3c\x75\x13\xe5\x83\x4c\xdf\xe9\x57\x80\x59");
}


RED_AUTO_TEST_CASE(TraceWidgetFlatButton3)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;

    WidgetFlatButton wbutton(drawable, parent, notifier, "test3", id,
                             fg_color, bg_color, fc_color, 2, global_font_deja_vu_14());
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button3.png");

    RED_CHECK_SIG(drawable, "\x3e\x75\x28\xf9\x54\x99\x36\x1e\xb9\x87\x54\xd9\xfa\x07\xcc\x15\x81\xec\xd9\xd3");

}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton4)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;

    WidgetFlatButton wbutton(drawable, parent, notifier, "test4", id,
                             fg_color, bg_color, fc_color, 2, global_font_deja_vu_14());
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button4.png");

    RED_CHECK_SIG(drawable, "\x0b\x33\xf4\xd2\xc9\xa7\x79\x0f\xac\xbe\x3c\xa8\xa6\xde\x5d\x64\x4b\xe3\xf0\x2c");

}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton5)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, parent, notifier, "test5", id,
                             fg_color, bg_color, fc_color, 2, global_font_deja_vu_14());
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button5.png");

    RED_CHECK_SIG(drawable, "\x39\x29\xce\xbf\x2e\xde\xcb\x24\x8d\x0c\xd3\xc3\xd5\xea\x51\x2e\xee\x47\x14\x68");

}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton6)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, parent, notifier, "test6", id,
                             fg_color, bg_color, fc_color, 2, global_font_deja_vu_14());
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button6.png");

    RED_CHECK_SIG(drawable, "\x8a\xe6\xed\xdf\x9d\xad\x44\x2a\xef\x53\x79\xf3\x33\x5a\xfe\xdc\x43\xb8\xf7\x22");

}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonClip)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    BGRColor fc_color = WINBLUE;
    int16_t x = 760;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, parent, notifier, "test6", id,
                             fg_color, bg_color, fc_color, 2, global_font_deja_vu_14());
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.x(),
                                      wbutton.y(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png("button7.png");

    RED_CHECK_SIG(drawable, "\x15\x28\xe8\x21\x36\x3f\xb1\x94\x39\xa9\xfc\xe7\xc5\x6a\xc7\x90\xef\x77\x3c\x14");

}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonClip2)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetFlatButton wbutton(drawable, parent, notifier, "test6", id,
                             fg_color, bg_color, fc_color, 2, global_font_deja_vu_14());
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.x(),
                                      5 + wbutton.y(),
                                      30,
                                      10));

    // drawable.save_to_png("button8.png");

    RED_CHECK_SIG(drawable, "\xf7\x36\xd7\x86\xbb\x33\x70\x0b\x90\x3a\xc0\xfb\x03\x9d\xdf\xf9\xc3\x99\x67\x10");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonDownAndUp)
{
    TestGraphic drawable(800, 600);


    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
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

    WidgetFlatButton wbutton(drawable, parent, notifier, "test6", id,
                             fg_color, bg_color, fc_color, 2, global_font_deja_vu_14(), xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button9.png");

    RED_CHECK_SIG(drawable, "\x72\xac\x38\xdf\x59\x84\x3b\x80\xf5\x0a\xc8\xcc\xe5\x97\xbd\xc9\x2f\x16\x89\x0c");


    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button10.png");

    RED_CHECK_SIG(drawable, "\xa6\x46\x23\xf9\x18\xb6\x48\x06\x1c\xdd\x09\x27\x39\xcd\x99\xd0\x28\xe7\xda\xb9");


    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 15, 15, nullptr);
    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button11.png");

    RED_CHECK_SIG(drawable, "\x72\xac\x38\xdf\x59\x84\x3b\x80\xf5\x0a\xc8\xcc\xe5\x97\xbd\xc9\x2f\x16\x89\x0c");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonEvent)
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


    WidgetFlatButton wbutton(drawable, parent, &notifier, "", 0, WHITE,
                             DARK_BLUE_BIS, WINBLUE, 2, global_font_deja_vu_14());
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
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetFlatButton wbutton1(drawable, wcomposite, notifier,
                              "abababab", 0, YELLOW, BLACK, WINBLUE, 2, global_font_deja_vu_14());
    Dimension dim = wbutton1.get_optimal_dim();
    wbutton1.set_wh(dim);
    wbutton1.set_xy(0, 0);

    WidgetFlatButton wbutton2(drawable, wcomposite, notifier,
                              "ggghdgh", 0, WHITE, RED, WINBLUE, 2, global_font_deja_vu_14());
    dim = wbutton2.get_optimal_dim();
    wbutton2.set_wh(dim);
    wbutton2.set_xy(0, 100);

    WidgetFlatButton wbutton3(drawable, wcomposite, notifier,
                              "lldlslql", 0, BLUE, RED, WINBLUE, 2, global_font_deja_vu_14());
    dim = wbutton3.get_optimal_dim();
    wbutton3.set_wh(dim);
    wbutton3.set_xy(100, 100);

    WidgetFlatButton wbutton4(drawable, wcomposite, notifier,
                              "LLLLMLLM", 0, PINK, DARK_GREEN, WINBLUE, 2, global_font_deja_vu_14());
    dim = wbutton4.get_optimal_dim();
    wbutton4.set_wh(dim);
    wbutton4.set_xy(300, 300);

    WidgetFlatButton wbutton5(drawable, wcomposite, notifier,
                              "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, WINBLUE, 2, global_font_deja_vu_14());
    dim = wbutton5.get_optimal_dim();
    wbutton5.set_wh(dim);
    wbutton5.set_xy(700, -10);

    WidgetFlatButton wbutton6(drawable, wcomposite, notifier,
                              "xxwwp", 0, ANTHRACITE, PALE_GREEN, WINBLUE, 2, global_font_deja_vu_14());
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

    RED_CHECK_SIG(drawable, "\xe0\xe5\x89\x9a\x4f\xc8\x5c\x95\x4a\x46\x26\x7f\xac\x44\x5b\xc1\x57\x6d\x06\x84");


    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png("button13.png");

    RED_CHECK_SIG(drawable, "\xdb\xfd\x24\x61\xa3\xd4\x82\x64\x5a\x61\xed\x1b\xbc\x32\xd8\xdf\x8d\x7f\x5c\x5c");


    wcomposite.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonFocus)
{
    TestGraphic drawable(72, 40);


    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
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

    WidgetFlatButton wbutton(drawable, parent, notifier, "test7", id,
                             fg_color, bg_color, fc_color, 2, global_font_deja_vu_14(), xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button14.png");

    RED_CHECK_SIG(drawable, "\x36\x23\xb8\x1b\x58\x75\x3e\x8c\x38\x1b\x4b\xba\x67\x19\x72\x85\x1d\xce\xe0\xc1");


    wbutton.focus(Widget::focus_reason_tabkey);

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button15.png");

    RED_CHECK_SIG(drawable, "\xfc\x69\x29\x22\xe8\xac\xfb\x2b\x26\x02\x12\x9b\x99\x2d\xec\xde\x56\x03\x02\xaa");



    wbutton.blur();

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button16.png");

    RED_CHECK_SIG(drawable, "\x36\x23\xb8\x1b\x58\x75\x3e\x8c\x38\x1b\x4b\xba\x67\x19\x72\x85\x1d\xce\xe0\xc1");


    wbutton.focus(Widget::focus_reason_tabkey);

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    // drawable.save_to_png("button17.png");


    RED_CHECK_SIG(drawable, "\xfc\x69\x29\x22\xe8\xac\xfb\x2b\x26\x02\x12\x9b\x99\x2d\xec\xde\x56\x03\x02\xaa");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonLite)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable, parent, notifier, "test1", id,
                             fg_color, bg_color, fc_color, 1, global_font_deja_vu_14(), xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(0, 0);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));

    // drawable.save_to_png("button_lite.png");

    RED_CHECK_SIG(drawable, "\x15\xa0\xce\x65\x61\xe4\x3a\x59\x40\xbb\xee\x09\xf3\x24\xc3\x96\x53\x4c\xab\x87");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, wbutton.x() + 2, wbutton.y() + 2, nullptr);

    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));

    // drawable.save_to_png("button_lite_down.png");

    RED_CHECK_SIG(drawable, "\x45\x12\x72\xb4\xc2\xba\xc7\x79\xf8\xbb\x73\x58\x57\x4a\x5e\xeb\xf4\xce\xfc\x37");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonStrong)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor fc_color = WINBLUE;
    int id = 0;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable, parent, notifier, "test1", id,
                             fg_color, bg_color, fc_color, 5, global_font_deja_vu_14(), xtext, ytext);
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(0, 0);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));

    // drawable.save_to_png("button_lite.png");

    RED_CHECK_SIG(drawable, "\x96\xd0\x45\xec\x88\xda\xa5\x78\xc1\x8b\x6a\x81\xa7\x06\xb4\xab\xb9\x2a\x4e\x9d");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, wbutton.x() + 2, wbutton.y() + 2, nullptr);

    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));

    // drawable.save_to_png("button_lite_down.png");

    RED_CHECK_SIG(drawable, "\x5f\x79\x1f\x43\xd0\x7c\x0b\x24\x8d\xd7\x7f\xde\xbe\xa1\x7d\xcb\x06\xd8\x37\xcd");
}
