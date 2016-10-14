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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetEdit
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget2/edit.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "fake_draw.hpp"
#include "check_sig.hpp"


BOOST_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
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

    WidgetEdit wedit(drawable.gd, x, y, cx, parent, notifier, "test1", id,
                     fg_color, bg_color, bg_color, font, edit_pos, xtext, ytext);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dx(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xf0\x05\x04\x2e\x52\x31\xa1\xea\x1a\xaf\x81\x19\x22\x17\xf2\x34\xec\x62\x2b\x71"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, x, y, cx, parent, notifier, "test2", id, fg_color, bg_color, 0, font);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit2.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xfa\x6a\x22\x83\xfb\x42\x4e\xb3\xb2\x2f\x78\xff\x44\xc6\xbb\x2b\xc1\x28\x19\xfe"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, x, y, cx, parent, notifier, "test3", id, fg_color, bg_color, 0, font);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit3.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x3b\x23\xea\x2f\xcc\x5d\xe4\x95\x9c\x09\xb4\xe5\x3c\xed\xb0\x76\xa9\xb4\xbf\xbe"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, x, y, cx, parent, notifier, "test4", id, fg_color, bg_color, 0, font);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit4.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x1e\xf8\x27\x8b\x51\x40\x95\x9f\xa2\x11\x77\x83\x76\x87\x8e\x56\x0e\x87\xf7\x35"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit5)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, x, y, cx, parent, notifier, "test5", id, fg_color, bg_color, 0, font);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit5.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xa3\xe9\x73\x7b\x41\x4b\xb6\xa3\x82\x52\x0a\x78\xb6\x7d\x45\xe8\xa2\x36\x7e\xfb"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit6)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0, font);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit6.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x18\xfd\x61\xa5\x48\x8e\xd1\x96\x2a\x82\xa3\xfc\x1f\x7b\x9a\xce\xbc\xee\x14\x09"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0, font);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wedit.rdp_input_invalidate(Rect(20 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit7.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xe0\x8f\xa1\x08\x78\x93\x67\x19\xea\xc1\xf0\xa0\x7a\xba\x6f\x91\x88\x6c\x9c\x89"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable.gd, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0, font);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wedit.rdp_input_invalidate(Rect(20 + wedit.dx(),
                                    5 + wedit.dy(),
                                    30,
                                    10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit8.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xf7\x5e\x7f\x51\xd8\xfd\x81\x8f\x1e\x8d\x73\xa8\x9c\xd7\xeb\x94\xf9\x35\xd2\x2b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetEdit)
{
    TestDraw drawable(800, 600);

    struct WidgetReceiveEvent : public Widget2 {
        Widget2* sender = nullptr;
        NotifyApi::notify_event_t event = 0;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget2(drawable.gd, Rect(), *this, nullptr)
        {}

        void draw(const Rect&) override
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

    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEdit wedit(drawable.gd, x, y, cx, parent, &notifier, "abcdef", 0, GREEN, RED, RED, font);
    wedit.focus(Widget2::focus_reason_tabkey);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xdc\xe9\x07\xc1\xe1\xe7\xd5\x8e\x3f\x2a\x54\xd5\x66\x86\xc8\xdb\x37\x3a\x23\xa6"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    bool ctrl_alt_delete;

    keymap.event(0, 16, ctrl_alt_delete); // 'a'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e2-1.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x89\x76\x82\x20\xa8\x1b\x60\xdf\x6f\xba\x92\x86\x42\xc9\xae\x26\xdd\x1c\xfc\xca"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.event(0, 17, ctrl_alt_delete); // 'z'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e2-2.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x23\xd1\xc8\x61\xf4\x14\xd8\x95\x37\xed\x79\x46\xe0\x61\x23\x12\xf1\x8a\x9e\x11"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e3.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x4d\x2f\xed\x50\x29\xc5\xb3\x54\x88\xe2\xee\xed\x30\x4b\x96\x13\x81\x4a\x22\x90"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e4.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x23\xd1\xc8\x61\xf4\x14\xd8\x95\x37\xed\x79\x46\xe0\x61\x23\x12\xf1\x8a\x9e\x11"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e5.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x89\x76\x82\x20\xa8\x1b\x60\xdf\x6f\xba\x92\x86\x42\xc9\xae\x26\xdd\x1c\xfc\xca"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e6.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x09\x85\xa8\x29\xe6\xb5\x46\x6c\xc5\xdc\xc3\xe7\xe3\x80\x74\xba\xc6\x00\xd9\x9a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e7.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x44\xca\x69\xd8\xd0\x75\xc6\x91\x9f\xa6\x12\x3c\x79\x2d\x25\x79\x35\x38\x68\x9a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e8.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xab\x92\x17\xaf\x75\x9d\x38\x42\x1f\x42\xd7\x61\xec\xb5\x0b\x16\x31\xf6\x37\x42"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e9.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xba\xab\x31\x83\x37\xfb\x68\x1e\x20\x6e\x97\x70\x95\x77\xea\xe7\x80\x58\xdd\xbc"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e10.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x0d\xbd\x1f\x41\x80\x22\x7d\xdd\x0f\xf6\xa6\xe4\xff\x9a\x78\x8b\x9d\x30\xf4\x12"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

    wedit.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x+10, y+3, nullptr);
    BOOST_CHECK(widget_for_receive_event.sender == nullptr);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e11.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x15\x63\xc0\x11\x75\xbb\xfb\x6d\x2e\xae\xec\xcc\x0f\xfb\xc7\xf4\x77\x23\x12\xe7"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditAndComposite)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable.gd, Rect(0,0,800,600), parent, notifier);

    WidgetEdit wedit1(drawable.gd, 0,0, 50, wcomposite, notifier,
                      "abababab", 4, YELLOW, BLACK, BLACK, font);
    WidgetEdit wedit2(drawable.gd, 0,100, 50, wcomposite, notifier,
                        "ggghdgh", 2, WHITE, RED, RED, font);
    WidgetEdit wedit3(drawable.gd, 100,100, 50, wcomposite, notifier,
                        "lldlslql", 1, BLUE, RED, RED, font);
    WidgetEdit wedit4(drawable.gd, 300,300, 50, wcomposite, notifier,
                        "LLLLMLLM", 20, PINK, DARK_GREEN, DARK_GREEN, font);
    WidgetEdit wedit5(drawable.gd, 700,-10, 50, wcomposite, notifier,
                        "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, DARK_BLUE, font);
    WidgetEdit wedit6(drawable.gd, -10,550, 50, wcomposite, notifier,
                        "xxwwp", 2, ANTHRACITE, PALE_GREEN, PALE_GREEN, font);

    wcomposite.add_widget(&wedit1);
    wcomposite.add_widget(&wedit2);
    wcomposite.add_widget(&wedit3);
    wcomposite.add_widget(&wedit4);
    wcomposite.add_widget(&wedit5);
    wcomposite.add_widget(&wedit6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit9.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xf6\x62\xa4\xfe\x63\x12\xc0\xee\x9f\x30\x5a\xe3\x22\xb5\x7b\x7b\x98\x40\xd5\xcb"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit10.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xf5\xce\xef\xb9\x39\xd8\xe1\x5e\xa0\x8d\xb8\x7f\x4e\x9d\xa4\x0d\x2a\x4e\x31\x45"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    wcomposite.clear();
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditScrolling)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;
    WidgetEdit wedit(drawable.gd, x, y, cx, parent, &parent, "abcde", 0, BLACK, WHITE, WHITE, font,
                     -1u, 1, 1);
    wedit.focus(Widget2::focus_reason_tabkey);
    parent.add_widget(&wedit);
    parent.current_focus = &wedit;

    char message[1024];

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

    if (!check_sig(drawable.gd.impl(), message,
        "\xb8\x48\x0d\x9d\x5d\x4a\xee\x17\x75\x67\x5e\x31\x68\x51\xa0\xe0\x17\xea\xad\xc4"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.clear();
}
