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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/widget/flat_button.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/button/"

RED_AUTO_TEST_CASE(TraceWidgetFlatButton)
{
    TestGraphic drawable(800, 600);

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton2)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_2.png");
}


RED_AUTO_TEST_CASE(TraceWidgetFlatButton3)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton4)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton5)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButton6)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonClip)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonClip2)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_8.png");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonDownAndUp)
{
    TestGraphic drawable(800, 600);


    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_9.png");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
    wbutton.rdp_input_invalidate(wbutton.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_10.png");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 15, 15, nullptr);
    wbutton.rdp_input_invalidate(wbutton.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_9.png");
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

        void rdp_input_invalidate(Rect /*r*/) override
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
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_12.png");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_13.png");

    wcomposite.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonFocus)
{
    TestGraphic drawable(72, 40);


    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_14.png");

    wbutton.focus(Widget::focus_reason_tabkey);

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_15.png");

    wbutton.blur();

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_14.png");

    wbutton.focus(Widget::focus_reason_tabkey);

    wbutton.rdp_input_invalidate(wbutton.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_15.png");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonLite)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button18.png");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, wbutton.x() + 2, wbutton.y() + 2, nullptr);

    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button19.png");
}

RED_AUTO_TEST_CASE(TraceWidgetFlatButtonStrong)
{
    TestGraphic drawable(800, 600);


    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button20.png");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, wbutton.x() + 2, wbutton.y() + 2, nullptr);

    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button21.png");
}
