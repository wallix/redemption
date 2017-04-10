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

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget2/edit.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "fake_draw.hpp"
#include "check_sig.hpp"


RED_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit1.png");

    RED_CHECK_SIG(drawable.gd, "\xa2\x83\xc9\x3f\x2e\x0a\xe8\xe3\xb4\x08\x27\x38\x3c\x56\xa5\xed\x6c\x46\xb4\x04");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test2", id, fg_color, bg_color, 0, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit2.png");

    RED_CHECK_SIG(drawable.gd, "\x84\xd6\xc6\xbe\xb0\xd5\xbe\x01\x99\x72\xc1\xe0\xfc\xd9\x5f\x33\x91\x40\x5e\x81");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test3", id, fg_color, bg_color, 0, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit3.png");

    RED_CHECK_SIG(drawable.gd, "\x6d\x70\x05\x11\xe5\x3d\x98\xa4\x53\x0e\xc2\xcf\x3f\x27\xda\x63\xb3\x50\x43\x92");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test4", id, fg_color, bg_color, 0, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit4.png");

    RED_CHECK_SIG(drawable.gd, "\x1e\xf8\x27\x8b\x51\x40\x95\x9f\xa2\x11\x77\x83\x76\x87\x8e\x56\x0e\x87\xf7\x35");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit5)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test5", id, fg_color, bg_color, 0, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit5.png");

    RED_CHECK_SIG(drawable.gd, "\x4d\x1d\xde\x48\xe4\x3b\xca\x16\x49\xf8\xc6\xa2\x2b\xb6\xd8\x41\x4d\x1d\x10\x02");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit6)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, 0, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit6.png");

    RED_CHECK_SIG(drawable.gd, "\x18\xfd\x61\xa5\x48\x8e\xd1\x96\x2a\x82\xa3\xfc\x1f\x7b\x9a\xce\xbc\xee\x14\x09");
}

RED_AUTO_TEST_CASE(TraceWidgetEditClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, 0, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wedit.rdp_input_invalidate(Rect(20 + wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit7.png");

    RED_CHECK_SIG(drawable.gd, "\xe0\x8f\xa1\x08\x78\x93\x67\x19\xea\xc1\xf0\xa0\x7a\xba\x6f\x91\x88\x6c\x9c\x89");
}

RED_AUTO_TEST_CASE(TraceWidgetEditClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, 0, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wedit.rdp_input_invalidate(Rect(20 + wedit.x(),
                                    5 + wedit.y(),
                                    30,
                                    10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit8.png");

    RED_CHECK_SIG(drawable.gd, "\xf7\x5e\x7f\x51\xd8\xfd\x81\x8f\x1e\x8d\x73\xa8\x9c\xd7\xeb\x94\xf9\x35\xd2\x2b");
}

RED_AUTO_TEST_CASE(EventWidgetEdit)
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

    struct Notify : public NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;
        Notify() = default;
        void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEdit wedit(drawable.gd, parent, &notifier, "abcdef", 0, GREEN, RED, RED, font);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    wedit.focus(Widget2::focus_reason_tabkey);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e1.png");
    RED_CHECK_SIG(drawable.gd, "\x06\x35\x2c\xbb\xf9\xe4\xd6\xb1\xe7\xd8\xa7\x97\x63\x8a\x67\xf3\x58\x3d\x47\x44");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    bool ctrl_alt_delete;

    keymap.event(0, 16, ctrl_alt_delete); // 'a'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e2-1.png");
    RED_CHECK_SIG(drawable.gd, "\x1f\x1b\x42\x70\xd5\x06\xf7\x86\xb9\xee\xfb\x8f\xed\xb5\x5a\x30\xfc\x11\xa1\x5e");
    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.event(0, 17, ctrl_alt_delete); // 'z'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e2-2.png");
    RED_CHECK_SIG(drawable.gd, "\xb9\x6c\xe1\x87\xeb\x7f\x77\x16\x9d\x18\xe6\x4e\xd1\xf2\x11\x5e\x12\x89\x0e\xd7");
    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e3.png");
    RED_CHECK_SIG(drawable.gd, "\x92\x27\x53\x44\xf7\x1c\xcb\x17\x3c\xe2\x2f\xa7\xec\x1d\x50\x3b\x26\xfc\x5c\xe8");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e4.png");
    RED_CHECK_SIG(drawable.gd, "\xb9\x6c\xe1\x87\xeb\x7f\x77\x16\x9d\x18\xe6\x4e\xd1\xf2\x11\x5e\x12\x89\x0e\xd7");

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e5.png");
    RED_CHECK_SIG(drawable.gd, "\x1f\x1b\x42\x70\xd5\x06\xf7\x86\xb9\xee\xfb\x8f\xed\xb5\x5a\x30\xfc\x11\xa1\x5e");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e6.png");
    RED_CHECK_SIG(drawable.gd, "\x16\x3c\xed\x11\xb8\x7f\xc7\xa5\x95\xe0\x7f\xc0\x80\x81\xc4\x45\x86\xcf\xae\x52");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e7.png");
    RED_CHECK_SIG(drawable.gd, "\x2a\xdb\xc9\x67\xa1\x0a\x90\x0b\x71\xba\xee\xb2\xf1\x7a\x69\xbf\x81\x4c\x37\x1c");

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e8.png");
    RED_CHECK_SIG(drawable.gd, "\x10\xde\xc3\x6e\xc7\xeb\x61\xb0\xa0\x52\x79\x66\xff\x98\x38\x22\x4c\xdf\x0f\xb2");

    keymap.push_kevent(Keymap2::KEVENT_END);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e9.png");
    RED_CHECK_SIG(drawable.gd, "\x0d\x81\x9c\x82\x3f\x74\x8c\x99\x6d\x46\xbc\x3b\x82\xd6\x23\xc9\xd3\x59\x62\xc2");

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e10.png");
    RED_CHECK_SIG(drawable.gd, "\xd9\x91\xf5\x9e\x54\x5f\x7e\x35\xe8\x3c\x72\xac\xaf\x2f\x77\x64\x57\x11\xda\x66");

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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e11.png");

    RED_CHECK_SIG(drawable.gd, "\x19\x07\xef\x0d\x1b\xc4\xcc\xba\x55\xf8\x4d\xf3\x59\x0e\xfb\x09\x49\x10\x50\x17");
}

RED_AUTO_TEST_CASE(TraceWidgetEditAndComposite)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit9.png");

    RED_CHECK_SIG(drawable.gd, "\x42\x13\x7c\x26\xa7\x7f\x59\xbe\x96\x0d\x62\xbd\x64\xe9\xe9\xef\xdf\xc1\xae\x17");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit10.png");

    RED_CHECK_SIG(drawable.gd, "\x3e\x5e\xc8\xb9\xdd\x06\xcf\xe0\xfb\xf8\x3d\x06\x72\x8e\xc8\xc5\x1a\xd5\xa5\x86");
    wcomposite.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetEditScrolling)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

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

    wedit.focus(Widget2::focus_reason_tabkey);
    parent.add_widget(&wedit);
    parent.current_focus = &wedit;

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0.png");
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
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-1.png");

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
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-2.png");
    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-3.png");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-4.png");

    keymap.event(keyboardFlags, keyCode + 9, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s1.png");

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s1-1.png");

    keymap.push_kevent(Keymap2::KEVENT_END);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s1-2.png");

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s2.png");

    for (int i = 0; i < 10; i++) {
        keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
        parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    }
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s2-1.png");

    RED_CHECK_SIG(drawable.gd, "\x76\x19\xcc\x5d\xc0\xf6\x6a\xb3\x1b\x4e\xdb\x60\xf5\xa8\x64\x53\xde\xff\x3e\xae");

    parent.clear();
}
