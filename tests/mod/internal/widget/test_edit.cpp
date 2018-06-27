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

#define RED_TEST_MODULE TestWidgetEdit
#include "system/redemption_unit_tests.hpp"


#include "core/font.hpp"
#include "mod/internal/widget/edit.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/mod/fake_draw.hpp"
#include "test_only/check_sig.hpp"


RED_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;
    int xtext = 4;
    int ytext = 1;
    size_t edit_pos = 2;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test1", id,
                     fg_color, bg_color, bg_color, font, edit_pos, xtext, ytext);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.x(),
                                    wedit.cx(),
                                    wedit.cy()));

    // // drawable.save_to_png("edit1.png");

    RED_CHECK_SIG(drawable.gd, "\xc7\x99\x1f\x40\xc9\x3b\x94\x44\x24\xb0\x14\x05\xdb\x3f\xdd\xfd\x6c\x04\x91\x28");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetEdit is a edit widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test2", id, fg_color, bg_color, BLACK, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // // drawable.save_to_png("edit2.png");

    RED_CHECK_SIG(drawable.gd, "\x80\xc0\x5a\x0f\x50\xf0\x7b\xca\x19\x9c\xc3\x04\xf7\x3e\xa2\xe3\xe6\xe9\xdc\xae");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetEdit is a edit widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test3", id, fg_color, bg_color, BLACK, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // // drawable.save_to_png("edit3.png");

    RED_CHECK_SIG(drawable.gd, "\x8e\xe4\xdf\xf4\x84\x5a\x15\x77\x13\x5e\xef\xd5\xd1\x9b\x16\x9b\x78\x0c\x2b\xb6");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test4", id, fg_color, bg_color, BLACK, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // // drawable.save_to_png("edit4.png");

    RED_CHECK_SIG(drawable.gd, "\xa2\x51\x8a\xa7\xa9\xfa\x36\x55\xb1\xec\x58\x80\x7a\x6d\xc0\x80\xd8\xc4\xf5\xe7");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit5)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetEdit is a edit widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test5", id, fg_color, bg_color, BLACK, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // // drawable.save_to_png("edit5.png");

    RED_CHECK_SIG(drawable.gd, "\xca\xce\x31\x98\x72\xa9\x48\x02\xcc\xaa\x62\x98\x8f\x13\x20\xa5\x38\x2c\x7c\xcf");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit6)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, BLACK, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // // drawable.save_to_png("edit6.png");

    RED_CHECK_SIG(drawable.gd, "\x8d\xf4\xcc\xf4\x21\x8d\xc8\x8c\x52\x9f\x94\xc2\x90\xfd\x9a\xe7\x3e\x7c\x86\xc8");
}

RED_AUTO_TEST_CASE(TraceWidgetEditClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, BLACK, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wedit.rdp_input_invalidate(Rect(20 + wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // // drawable.save_to_png("edit7.png");

    RED_CHECK_SIG(drawable.gd, "\x53\x7a\x54\x5e\x4f\x84\xe7\xe8\x2e\xa1\x2d\xda\x49\x04\x6e\x83\xc6\x80\xd8\x04");
}

RED_AUTO_TEST_CASE(TraceWidgetEditClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetEdit is a edit widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, BLACK, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wedit.rdp_input_invalidate(Rect(20 + wedit.x(),
                                    5 + wedit.y(),
                                    30,
                                    10));

    // // drawable.save_to_png("edit8.png");

    RED_CHECK_SIG(drawable.gd, "\x19\xd6\x37\xfc\x56\x80\x9d\x76\xce\xc9\x4d\x84\x02\xbb\x00\x36\x7e\x85\x37\x69");
}

RED_AUTO_TEST_CASE(EventWidgetEdit)
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

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEdit wedit(drawable.gd, parent, &notifier, "abcdef", 0, GREEN, RED, RED, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    wedit.focus(Widget::focus_reason_tabkey);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e1.png");
    RED_CHECK_SIG(drawable.gd, "\x44\xf4\x47\x2a\xf9\x6d\x18\xf5\x38\x34\x9a\xdd\xd5\xa4\x3b\xbb\xd1\xcb\x98\x0d");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    bool ctrl_alt_delete;

    keymap.event(0, 16, ctrl_alt_delete); // 'a'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e2-1.png");
    RED_CHECK_SIG(drawable.gd, "\xae\xc8\x14\xa5\xf8\x5f\x46\xe1\xdd\x19\x11\xf1\xfb\x27\x3b\xc2\x36\x2c\x09\x20");
    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.event(0, 17, ctrl_alt_delete); // 'z'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e2-2.png");
    RED_CHECK_SIG(drawable.gd, "\xbf\x8e\xf0\x40\xdf\x7f\x13\x6f\xb8\x17\x42\xfc\xa6\xa6\x64\xa2\x5f\x6e\x48\xaf");
    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e3.png");
    RED_CHECK_SIG(drawable.gd, "\xed\xb5\xc8\x03\xf8\x89\x29\xc7\x6b\xc3\x54\xd1\x82\xaa\x71\xf8\x45\xc4\xd2\x3b");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e4.png");
    RED_CHECK_SIG(drawable.gd, "\xbf\x8e\xf0\x40\xdf\x7f\x13\x6f\xb8\x17\x42\xfc\xa6\xa6\x64\xa2\x5f\x6e\x48\xaf");

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e5.png");
    RED_CHECK_SIG(drawable.gd, "\xae\xc8\x14\xa5\xf8\x5f\x46\xe1\xdd\x19\x11\xf1\xfb\x27\x3b\xc2\x36\x2c\x09\x20");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e6.png");
    RED_CHECK_SIG(drawable.gd, "\xc0\x74\x67\x34\x1e\x68\x03\xdf\xc8\xf6\x33\x9e\xc2\x12\x02\xaf\x9c\xc1\xb3\x3f");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e7.png");
    RED_CHECK_SIG(drawable.gd, "\x7d\x2e\x43\x00\x55\xb3\x69\x57\x46\xc5\x6e\x1f\x98\xee\x53\x3e\xe1\x9f\xc0\xe4");

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e8.png");
    RED_CHECK_SIG(drawable.gd, "\x47\x6d\xf3\xc2\x37\x0d\x23\x81\x86\x78\xed\x1f\x47\x86\x82\x4d\x1e\x8b\x71\xe6");

    keymap.push_kevent(Keymap2::KEVENT_END);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e9.png");
    RED_CHECK_SIG(drawable.gd, "\x7e\xf2\x83\xa5\x84\x21\x18\x89\xf9\xfb\x9d\xaf\x98\x70\xc9\xb2\x2d\x9a\x9c\x79");

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e10.png");
    RED_CHECK_SIG(drawable.gd, "\x6b\x39\x5e\x15\xd1\xf7\x71\x25\xb9\xae\xb9\x49\x8c\xa1\x95\x26\xb9\x72\x2e\xf4");

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

    wedit.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x+10, y+3, nullptr);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));

    // drawable.save_to_png("edit-e11.png");

    RED_CHECK_SIG(drawable.gd, "\x52\x7f\xea\x38\xfe\xf0\x19\xd7\x47\x86\x9c\x17\x33\xfb\xe5\xea\x7e\x06\x25\x20");
}

RED_AUTO_TEST_CASE(TraceWidgetEditAndComposite)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // WidgetEdit is a edit widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable.gd, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetEdit wedit1(drawable.gd, wcomposite, notifier,
                      "abababab", 4, YELLOW, BLACK, BLACK, font);
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(50, dim.h);
    wedit1.set_xy(0, 0);

    WidgetEdit wedit2(drawable.gd, wcomposite, notifier,
                      "ggghdgh", 2, WHITE, RED, RED, font);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(50, dim.h);
    wedit2.set_xy(0, 100);

    WidgetEdit wedit3(drawable.gd, wcomposite, notifier,
                      "lldlslql", 1, BLUE, RED, RED, font);
    dim = wedit3.get_optimal_dim();
    wedit3.set_wh(50, dim.h);
    wedit3.set_xy(100, 100);

    WidgetEdit wedit4(drawable.gd, wcomposite, notifier,
                      "LLLLMLLM", 20, PINK, DARK_GREEN, DARK_GREEN, font);
    dim = wedit4.get_optimal_dim();
    wedit4.set_wh(50, dim.h);
    wedit4.set_xy(300, 300);

    WidgetEdit wedit5(drawable.gd, wcomposite, notifier,
                      "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, DARK_BLUE, font);
    dim = wedit5.get_optimal_dim();
    wedit5.set_wh(50, dim.h);
    wedit5.set_xy(700, -10);

    WidgetEdit wedit6(drawable.gd, wcomposite, notifier,
                      "xxwwp", 2, ANTHRACITE, PALE_GREEN, PALE_GREEN, font);
    dim = wedit6.get_optimal_dim();
    wedit6.set_wh(50, dim.h);
    wedit6.set_xy(-10, 550);

    wcomposite.add_widget(&wedit1);
    wcomposite.add_widget(&wedit2);
    wcomposite.add_widget(&wedit3);
    wcomposite.add_widget(&wedit4);
    wcomposite.add_widget(&wedit5);
    wcomposite.add_widget(&wedit6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    // drawable.save_to_png("edit9.png");

    RED_CHECK_SIG(drawable.gd, "\xaf\x75\xe8\xb2\x84\x14\x37\x9e\x84\xf4\xdc\x1f\x09\x2f\x12\xa6\xfb\x88\x12\x46");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png("edit10.png");

    RED_CHECK_SIG(drawable.gd, "\x1f\x2d\xf3\xed\xc1\xaa\x00\xdc\x51\xc7\x9b\x51\x71\x84\x20\x62\x92\x37\x4b\xca");
    wcomposite.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetEditScrolling)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEdit wedit(drawable.gd, parent, &parent, "abcde", 0, BLACK, WHITE, WHITE, font,
                     -1u, 1, 1);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    wedit.focus(Widget::focus_reason_tabkey);
    parent.add_widget(&wedit);
    parent.current_focus = &wedit;

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));

    // drawable.save_to_png("edit-s0.png");
    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    bool    ctrl_alt_delete;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png("edit-s0-1.png");

    keymap.event(keyboardFlags, keyCode + 1, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode + 2, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png("edit-s0-2.png");
    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png("edit-s0-3.png");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png("edit-s0-4.png");

    keymap.event(keyboardFlags, keyCode + 9, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png("edit-s1.png");

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png("edit-s1-1.png");

    keymap.push_kevent(Keymap2::KEVENT_END);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png("edit-s1-2.png");

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png("edit-s2.png");

    for (int i = 0; i < 10; i++) {
        keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
        parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    }
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png("edit-s2-1.png");

    RED_CHECK_SIG(drawable.gd, "\x42\x96\x9c\xd7\xdd\x44\x53\x15\xcb\x9b\x46\x5f\xe5\xe6\x1b\xa8\xa8\x3d\x9b\xb4");

    parent.clear();
}
