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
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/edit.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/core/font.hpp"


RED_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
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

    WidgetEdit wedit(drawable, parent, notifier, "test1", id,
                     fg_color, bg_color, bg_color, global_font_deja_vu_14(), edit_pos, xtext, ytext);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.x(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png("edit1.png");

    RED_CHECK_SIG(drawable, "\xae\xaf\x11\xb3\xff\x64\x87\x77\xe8\x4a\x15\xd2\x94\x83\x35\x57\x18\x8a\xf4\x2c");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test2", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png("edit2.png");

    RED_CHECK_SIG(drawable, "\x1e\x5d\xc1\x0e\x49\x19\xf8\xc7\x35\x0b\xd7\xbe\x95\x21\xe7\xd9\xd1\xdb\x95\xb8");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test3", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png("edit3.png");

    RED_CHECK_SIG(drawable, "\x97\x5a\xa5\x98\x34\x61\x07\x30\x40\x2e\x02\xda\x08\x61\x91\x95\x09\x64\xde\x57");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit4)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test4", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png("edit4.png");

    RED_CHECK_SIG(drawable, "\x3b\x2c\x5a\x89\x72\x7f\xe4\x45\x86\x04\xfb\xc2\x8e\xf6\xe6\x4c\xd8\x2b\x37\xab");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit5)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test5", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png("edit5.png");

    RED_CHECK_SIG(drawable, "\xb8\x85\xe3\x1f\x50\x89\xc8\x93\x7b\x5e\xc9\x28\x92\x40\x8a\x61\x61\x86\xa8\x9b");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit6)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png("edit6.png");

    RED_CHECK_SIG(drawable, "\xd2\xb7\x7e\xe2\x35\xd1\x0b\x4b\x8d\xc8\xca\x14\x1c\x27\x4a\xe4\x6c\x5e\xf7\x83");
}

RED_AUTO_TEST_CASE(TraceWidgetEditClip)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wedit.rdp_input_invalidate(Rect(20 + wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png("edit7.png");

    RED_CHECK_SIG(drawable, "\x82\xd3\x97\xb5\x70\x57\xa0\x31\x5d\xfe\x62\x5c\xae\xcb\x42\x40\xcd\xbf\xf7\xb4");
}

RED_AUTO_TEST_CASE(TraceWidgetEditClip2)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wedit.rdp_input_invalidate(Rect(20 + wedit.x(),
                                    5 + wedit.y(),
                                    30,
                                    10));

    // drawable.save_to_png("edit8.png");

    RED_CHECK_SIG(drawable, "\xdf\xa2\x5b\x1c\x7d\x42\xd9\x7d\xe6\x1b\xc5\xdc\x73\x45\x22\x3c\xe5\x1d\x04\xe8");
}

RED_AUTO_TEST_CASE(EventWidgetEdit)
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


    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEdit wedit(drawable, parent, &notifier, "abcdef", 0, GREEN, RED, RED, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    wedit.focus(Widget::focus_reason_tabkey);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e1.png");
    RED_CHECK_SIG(drawable, "\xae\x5b\xb4\xa4\xb7\xc4\xbb\xc3\x69\x8e\xf4\x32\x1b\x19\x91\xff\x12\xd3\x87\xc1");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    bool ctrl_alt_delete;

    keymap.event(0, 16, ctrl_alt_delete); // 'a'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e2-1.png");
    RED_CHECK_SIG(drawable, "\x45\x22\xa3\xef\x85\x3f\x81\xc2\xd3\x40\x23\x54\x95\x15\x3a\xca\x22\xfe\x93\x0b");
    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.event(0, 17, ctrl_alt_delete); // 'z'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e2-2.png");
    RED_CHECK_SIG(drawable, "\xe1\x2c\x25\xf6\x93\x30\xf9\x46\x40\x9e\xe4\x6f\x11\x43\x3c\x6b\x15\x2d\x4f\xcb");
    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e3.png");
    RED_CHECK_SIG(drawable, "\x65\x3c\x08\xed\xe0\x8f\x8b\xe6\x53\xae\x90\x5d\x1b\x4b\x7b\x8b\x9d\xaa\x12\x46");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e4.png");
    RED_CHECK_SIG(drawable, "\xe1\x2c\x25\xf6\x93\x30\xf9\x46\x40\x9e\xe4\x6f\x11\x43\x3c\x6b\x15\x2d\x4f\xcb");

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e5.png");
    RED_CHECK_SIG(drawable, "\x45\x22\xa3\xef\x85\x3f\x81\xc2\xd3\x40\x23\x54\x95\x15\x3a\xca\x22\xfe\x93\x0b");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e6.png");
    RED_CHECK_SIG(drawable, "\x6b\xef\xd7\xaf\x7f\x79\x5a\x0e\x8b\xee\xd2\xfb\x26\xaa\x01\x47\x81\xaa\xe0\xb5");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e7.png");
    RED_CHECK_SIG(drawable, "\x83\x00\xdb\xc8\xc1\x75\x0e\xc0\xb6\xa6\x36\x3f\xf5\xb8\x28\xf0\x45\x15\x4b\x3b");

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e8.png");
    RED_CHECK_SIG(drawable, "\x46\x79\x49\x73\x54\xbe\x66\x91\x2b\x7a\xdb\x32\xf6\x39\x2f\x82\xa9\xc7\xc7\xce");

    keymap.push_kevent(Keymap2::KEVENT_END);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e9.png");
    RED_CHECK_SIG(drawable, "\xd4\x54\x30\xb6\x70\xb3\xaa\x14\x1d\x79\x26\xc3\xe9\x88\x86\xeb\x6c\xab\x27\x72");

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png("edit-e10.png");
    RED_CHECK_SIG(drawable, "\x53\xd2\xa3\xd9\xcb\xaa\x39\x63\x7d\x89\x44\x33\x84\xcd\x8b\xa1\xe3\x47\x49\x17");

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

    RED_CHECK_SIG(drawable, "\x36\xb8\x39\xca\x04\xec\x79\xfb\xd6\x96\xf0\x09\x24\x24\xd1\x2d\x13\xd9\x64\x86");
}

RED_AUTO_TEST_CASE(TraceWidgetEditAndComposite)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetEdit wedit1(drawable, wcomposite, notifier,
                      "abababab", 4, YELLOW, BLACK, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(50, dim.h);
    wedit1.set_xy(0, 0);

    WidgetEdit wedit2(drawable, wcomposite, notifier,
                      "ggghdgh", 2, WHITE, RED, RED, global_font_deja_vu_14());
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(50, dim.h);
    wedit2.set_xy(0, 100);

    WidgetEdit wedit3(drawable, wcomposite, notifier,
                      "lldlslql", 1, BLUE, RED, RED, global_font_deja_vu_14());
    dim = wedit3.get_optimal_dim();
    wedit3.set_wh(50, dim.h);
    wedit3.set_xy(100, 100);

    WidgetEdit wedit4(drawable, wcomposite, notifier,
                      "LLLLMLLM", 20, PINK, DARK_GREEN, DARK_GREEN, global_font_deja_vu_14());
    dim = wedit4.get_optimal_dim();
    wedit4.set_wh(50, dim.h);
    wedit4.set_xy(300, 300);

    WidgetEdit wedit5(drawable, wcomposite, notifier,
                      "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, DARK_BLUE, global_font_deja_vu_14());
    dim = wedit5.get_optimal_dim();
    wedit5.set_wh(50, dim.h);
    wedit5.set_xy(700, -10);

    WidgetEdit wedit6(drawable, wcomposite, notifier,
                      "xxwwp", 2, ANTHRACITE, PALE_GREEN, PALE_GREEN, global_font_deja_vu_14());
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

    RED_CHECK_SIG(drawable, "\x03\x46\x64\x48\x8f\x30\x38\xd8\x0e\x09\x18\xb8\x90\x36\x00\x19\x2e\x6f\x47\xb5");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png("edit10.png");

    RED_CHECK_SIG(drawable, "\x3e\x02\x5f\x15\xfe\xa8\xf4\x59\x9a\xa7\x37\xa9\xeb\xf4\xe4\xad\x9e\x8c\x55\x85");
    wcomposite.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetEditScrolling)
{
    TestGraphic drawable(800, 600);


    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEdit wedit(drawable, parent, &parent, "abcde", 0, BLACK, WHITE, WHITE, global_font_deja_vu_14(),
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

    RED_CHECK_SIG(drawable, "\x94\x78\xfe\x80\x68\x73\xd2\x7c\xa5\xe4\x46\x77\xdb\x7f\x25\x18\xf3\x83\xc3\x38");

    parent.clear();
}
