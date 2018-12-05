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

#define RED_TEST_MODULE TestWidgetPassword
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/password.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TraceWidgetPassword)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;
    int xtext = 4;
    int ytext = 1;
    size_t password_pos = 2;

    WidgetPassword wpassword(drawable, parent, notifier, "test1", id,
                     fg_color, bg_color, BLACK, global_font_lato_light_16(), password_pos, xtext, ytext);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.x(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png("password.png");

    RED_CHECK_SIG(drawable, "\x55\x68\x7f\x73\x0e\xbb\x81\x14\xaf\xec\x50\x8e\x45\x1c\xbf\xfa\x3d\x7c\x94\x34");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword2)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test2", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png("password2.png");

    RED_CHECK_SIG(drawable, "\x4c\xce\x8c\xe2\x96\x04\x33\x8e\x05\x37\x8b\x37\xb1\x84\x65\x8a\x14\xa4\xe4\xe0");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword3)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test3", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png("password3.png");

    RED_CHECK_SIG(drawable, "\x99\xa8\xb7\x5d\xc8\xf7\x21\x7a\xfd\x43\xda\x6f\xa6\x22\x0b\xb1\x98\x94\xbe\x01");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword4)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test4", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png("password4.png");

    RED_CHECK_SIG(drawable, "\xbc\xe3\x4f\xed\x92\x61\x9f\xce\x1c\x30\x89\x3b\x2c\x03\xfc\x57\x35\x3b\x58\xc4");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword5)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test5", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png("password5.png");

    RED_CHECK_SIG(drawable, "\x84\x87\xdd\xb9\x3b\x4b\xde\x71\x62\x39\x57\x69\xcc\x01\x08\xaf\x4d\x1e\xf4\x4a");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword6)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png("password6.png");

    RED_CHECK_SIG(drawable, "\xa6\x7d\xf0\xea\x02\x7a\x83\xf1\x97\x99\xf6\xac\xce\x88\x30\x8c\x03\x79\xfe\xe0");
}

RED_AUTO_TEST_CASE(TraceWidgetPasswordClip)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png("password7.png");

    RED_CHECK_SIG(drawable, "\xa1\x4c\x85\x5d\xe1\x28\x6a\xca\x28\x5d\xec\xab\x78\xdd\x9c\xf9\xf4\x96\xd5\x18");
}

RED_AUTO_TEST_CASE(TraceWidgetPasswordClip2)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.x(),
                                    5 + wpassword.y(),
                                    30,
                                    10));

    // drawable.save_to_png("password8.png");

    RED_CHECK_SIG(drawable, "\xbd\x31\x54\x64\x79\xfe\x2c\x43\xd9\xb7\x0d\x4b\x5e\x61\xec\xab\x04\xdc\x3e\x28");
}

RED_AUTO_TEST_CASE(EventWidgetPassword)
{
    TestGraphic drawable(800, 600);

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

    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    // Widget* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, parent, &notifier, "abcdef", 0, YELLOW, RED, RED, global_font_lato_light_16());
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit1-e1.png");
    RED_CHECK_SIG(drawable, "\x22\x5a\x9f\x46\x5a\x65\xce\x21\xb5\xb2\x58\x8e\xad\x5d\x27\x21\x34\x29\xfa\xfc");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    bool    ctrl_alt_delete;

    keymap.event(0, 16, ctrl_alt_delete); // 'a'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit1-e2-1.png");
    RED_CHECK_SIG(drawable, "\x3f\x66\xd4\xcb\x95\x88\x04\xb2\xc2\xa8\x17\x0b\x68\x7e\xdc\xb3\xea\x76\xe9\xe8");
    RED_CHECK(notifier.sender == &wpassword);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = nullptr;

    keymap.event(0, 17, ctrl_alt_delete); // 'z'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit1-e2-2.png");
    RED_CHECK_SIG(drawable, "\xb8\xcf\x40\x2c\xac\x7d\x2a\xd2\x21\x4f\xfa\x9c\xca\xc6\xfa\xc1\x64\xf8\x6b\x4e");
    RED_CHECK(notifier.sender == &wpassword);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = nullptr;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit1-e3.png");
    RED_CHECK_SIG(drawable, "\x2f\x86\xa1\xaf\x12\xdb\xf4\x76\x83\xd7\xd0\x0b\x9f\x70\xc8\x65\x5b\x34\x58\x5f");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit1-e4.png");
    RED_CHECK_SIG(drawable, "\xb8\xcf\x40\x2c\xac\x7d\x2a\xd2\x21\x4f\xfa\x9c\xca\xc6\xfa\xc1\x64\xf8\x6b\x4e");

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit1-e5.png");
    RED_CHECK_SIG(drawable, "\x3f\x66\xd4\xcb\x95\x88\x04\xb2\xc2\xa8\x17\x0b\x68\x7e\xdc\xb3\xea\x76\xe9\xe8");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit1-e6.png");
    RED_CHECK_SIG(drawable, "\xa2\x75\xbb\x0d\x83\x29\x03\xbd\x42\x54\x8f\x6c\x4a\xb4\x2f\x56\x93\x2f\xef\xc7");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit1-e7.png");
    RED_CHECK_SIG(drawable, "\x8b\x61\x8d\x2d\x47\xcd\x9b\x1e\x9e\xaf\xe4\x26\x00\xbc\x38\xd0\xd5\x3f\xec\x2e");

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit1-e8.png");
    RED_CHECK_SIG(drawable, "\xcf\x22\x3d\x44\xf7\x03\x3e\x29\x66\xa0\x3b\xe9\x83\x46\x95\xb0\x4e\x3b\x92\x93");

    keymap.push_kevent(Keymap2::KEVENT_END);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit1-e9.png");
    RED_CHECK_SIG(drawable, "\x22\x5a\x9f\x46\x5a\x65\xce\x21\xb5\xb2\x58\x8e\xad\x5d\x27\x21\x34\x29\xfa\xfc");

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit1-e10.png");
    RED_CHECK_SIG(drawable, "\xee\x0f\x21\x73\xfb\x7f\xf8\x54\xf2\x24\x00\xe3\x6e\xec\x9d\x23\x69\xee\xf8\x66");

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == &wpassword);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

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

    wpassword.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 10, 3, nullptr);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    notifier.sender = nullptr;
    notifier.event = 0;
    widget_for_receive_event.sender = nullptr;
    widget_for_receive_event.event = 0;

    wpassword.rdp_input_invalidate(Rect(0, 0, wpassword.cx(), wpassword.cx()));
    // drawable.save_to_png("password-edit-e11.png");
    RED_CHECK_SIG(drawable, "\xd4\xfd\xab\x94\xcc\x8b\xa1\xba\xa1\xaa\xa2\x2d\xe0\xe2\x9b\xb9\xeb\xde\xba\x40");
}

RED_AUTO_TEST_CASE(TraceWidgetPasswordAndComposite)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetPassword wpassword1(drawable, wcomposite, notifier,
                              "abababab", 4, YELLOW, BLACK, BLACK, global_font_lato_light_16());
    Dimension dim = wpassword1.get_optimal_dim();
    wpassword1.set_wh(50, dim.h);
    wpassword1.set_xy(0, 0);

    WidgetPassword wpassword2(drawable, wcomposite, notifier,
                              "ggghdgh", 2, WHITE, RED, RED, global_font_lato_light_16());
    dim = wpassword2.get_optimal_dim();
    wpassword2.set_wh(50, dim.h);
    wpassword2.set_xy(0, 100);

    WidgetPassword wpassword3(drawable, wcomposite, notifier,
                              "lldlslql", 1, BLUE, RED, RED, global_font_lato_light_16());
    dim = wpassword3.get_optimal_dim();
    wpassword3.set_wh(50, dim.h);
    wpassword3.set_xy(100, 100);

    WidgetPassword wpassword4(drawable, wcomposite, notifier,
                              "LLLLMLLM", 20, PINK, DARK_GREEN, DARK_GREEN, global_font_lato_light_16());
    dim = wpassword4.get_optimal_dim();
    wpassword4.set_wh(50, dim.h);
    wpassword4.set_xy(300, 300);

    WidgetPassword wpassword5(drawable, wcomposite, notifier,
                              "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, DARK_BLUE, global_font_lato_light_16());
    dim = wpassword5.get_optimal_dim();
    wpassword5.set_wh(50, dim.h);
    wpassword5.set_xy(700, -10);

    WidgetPassword wpassword6(drawable, wcomposite, notifier,
                              "xxwwp", 2, ANTHRACITE, PALE_GREEN, PALE_GREEN, global_font_lato_light_16());
    dim = wpassword6.get_optimal_dim();
    wpassword6.set_wh(50, dim.h);
    wpassword6.set_xy(-10, 550);

    wcomposite.add_widget(&wpassword1);
    wcomposite.add_widget(&wpassword2);
    wcomposite.add_widget(&wpassword3);
    wcomposite.add_widget(&wpassword4);
    wcomposite.add_widget(&wpassword5);
    wcomposite.add_widget(&wpassword6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    // drawable.save_to_png("password-compo1.png");

    RED_CHECK_SIG(drawable, "\xe9\xaa\xc3\x82\x4d\xe1\xa4\x16\xcf\xe6\x4f\xa5\xe2\xcf\x8c\xdd\xd3\xa9\xf9\x81");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png("password-compo2.png");

    RED_CHECK_SIG(drawable, "\x18\x07\x10\x6e\xe6\x16\x7b\x05\xe1\x57\x8b\x99\x96\x39\x53\x43\x12\x0e\x6f\x2e");
    wcomposite.clear();
}

RED_AUTO_TEST_CASE(DataWidgetPassword)
{
    TestGraphic drawable(800, 600);

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

    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    // Widget* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, parent, &notifier, "aurélie", 0, YELLOW, RED, RED, global_font_lato_light_16());
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit2-e1.png");
    RED_CHECK_SIG(drawable, "\x3f\x66\xd4\xcb\x95\x88\x04\xb2\xc2\xa8\x17\x0b\x68\x7e\xdc\xb3\xea\x76\xe9\xe8");
    RED_CHECK(notifier.sender == &wpassword);
    RED_CHECK(notifier.event == 0);
    notifier.sender = nullptr;
    notifier.event = 0;

    RED_CHECK_EQUAL("aurélie", wpassword.get_text());

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit2-e2.png");
    RED_CHECK_SIG(drawable, "\xa2\x75\xbb\x0d\x83\x29\x03\xbd\x42\x54\x8f\x6c\x4a\xb4\x2f\x56\x93\x2f\xef\xc7");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit2-e3.png");
    RED_CHECK_SIG(drawable, "\x8b\x61\x8d\x2d\x47\xcd\x9b\x1e\x9e\xaf\xe4\x26\x00\xbc\x38\xd0\xd5\x3f\xec\x2e");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit2-e4.png");
    RED_CHECK_SIG(drawable, "\x58\x53\x33\x1b\x93\x41\x5f\xa2\xbd\x7b\x5b\xed\x90\x8a\x1d\x0b\x16\x68\xb0\x06");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit2-e5.png");
    RED_CHECK_SIG(drawable, "\x8f\x3c\xea\x45\x08\xd1\x31\x33\x7a\x74\x20\x30\xdf\xb7\xbd\xe6\x49\xc7\x53\xfd");

    RED_CHECK_EQUAL("aurlie", wpassword.get_text());
}

RED_AUTO_TEST_CASE(DataWidgetPassword2)
{
    TestGraphic drawable(800, 600);

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

    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    // Widget* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, parent, &notifier, "aurélie", 0, YELLOW, RED, RED, global_font_lato_light_16());
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit3-e1.png");
    RED_CHECK_SIG(drawable, "\x3f\x66\xd4\xcb\x95\x88\x04\xb2\xc2\xa8\x17\x0b\x68\x7e\xdc\xb3\xea\x76\xe9\xe8");
    RED_CHECK(notifier.sender == &wpassword);
    RED_CHECK(notifier.event == 0);
    notifier.sender = nullptr;
    notifier.event = 0;

    RED_CHECK_EQUAL("aurélie", wpassword.get_text());

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit3-e2.png");
    RED_CHECK_SIG(drawable, "\xa2\x75\xbb\x0d\x83\x29\x03\xbd\x42\x54\x8f\x6c\x4a\xb4\x2f\x56\x93\x2f\xef\xc7");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit3-e3.png");
    RED_CHECK_SIG(drawable, "\x8b\x61\x8d\x2d\x47\xcd\x9b\x1e\x9e\xaf\xe4\x26\x00\xbc\x38\xd0\xd5\x3f\xec\x2e");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit3-e4.png");
    RED_CHECK_SIG(drawable, "\x58\x53\x33\x1b\x93\x41\x5f\xa2\xbd\x7b\x5b\xed\x90\x8a\x1d\x0b\x16\x68\xb0\x06");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit3-e5.png");
    RED_CHECK_SIG(drawable, "\xc4\x51\xf7\x3b\xcb\x9a\x0e\x44\x9c\xa2\x96\x0d\x90\xaf\x9c\xba\x8f\x7b\x61\x6b");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit3-e6.png");
    RED_CHECK_SIG(drawable, "\x8f\x3c\xea\x45\x08\xd1\x31\x33\x7a\x74\x20\x30\xdf\xb7\xbd\xe6\x49\xc7\x53\xfd");

    RED_CHECK_EQUAL("aurlie", wpassword.get_text());
}

RED_AUTO_TEST_CASE(DataWidgetPassword3)
{
    TestGraphic drawable(800, 600);

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

    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    // Widget* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, parent, &notifier, "aurélie", 0, YELLOW, RED, RED, global_font_lato_light_16());
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit4-e1.png");
    RED_CHECK_SIG(drawable, "\x3f\x66\xd4\xcb\x95\x88\x04\xb2\xc2\xa8\x17\x0b\x68\x7e\xdc\xb3\xea\x76\xe9\xe8");
    RED_CHECK(notifier.sender == &wpassword);
    RED_CHECK(notifier.event == 0);
    notifier.sender = nullptr;
    notifier.event = 0;

    RED_CHECK_EQUAL("aurélie", wpassword.get_text());

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit4-e2.png");
    RED_CHECK_SIG(drawable, "\xa2\x75\xbb\x0d\x83\x29\x03\xbd\x42\x54\x8f\x6c\x4a\xb4\x2f\x56\x93\x2f\xef\xc7");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit4-e3.png");
    RED_CHECK_SIG(drawable, "\x8b\x61\x8d\x2d\x47\xcd\x9b\x1e\x9e\xaf\xe4\x26\x00\xbc\x38\xd0\xd5\x3f\xec\x2e");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit4-e4.png");
    RED_CHECK_SIG(drawable, "\x58\x53\x33\x1b\x93\x41\x5f\xa2\xbd\x7b\x5b\xed\x90\x8a\x1d\x0b\x16\x68\xb0\x06");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit4-e5.png");
    RED_CHECK_SIG(drawable, "\xc4\x51\xf7\x3b\xcb\x9a\x0e\x44\x9c\xa2\x96\x0d\x90\xaf\x9c\xba\x8f\x7b\x61\x6b");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit4-e6.png");
    RED_CHECK_SIG(drawable, "\x58\x53\x33\x1b\x93\x41\x5f\xa2\xbd\x7b\x5b\xed\x90\x8a\x1d\x0b\x16\x68\xb0\x06");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);


    bool    ctrl_alt_delete;

    keymap.event(0, 17, ctrl_alt_delete); // 'z'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png("password-edit4-e7.png");
    RED_CHECK_SIG(drawable, "\xa9\x1c\x46\xea\x9e\x44\xc3\xd8\xc4\x55\xb5\xf5\x2f\x16\x17\x99\xbc\xf1\x5f\x0d");
    RED_CHECK(notifier.sender == &wpassword);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    RED_CHECK_EQUAL("aurézlie", wpassword.get_text());
}
