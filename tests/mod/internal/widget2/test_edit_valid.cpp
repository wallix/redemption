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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen,
 *              Meng Tan
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetEditValid
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "core/font.hpp"
#include "mod/internal/widget2/edit_valid.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = BLACK;
    int bg_color = WHITE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;
    // int xtext = 1;
    // int ytext = 1;
    // size_t edit_pos = -1;
    int xtext = 4;
    int ytext = 1;
    size_t edit_pos = 2;

    WidgetEditValid wedit(drawable.gd, x, y, cx, parent, notifier, "test1", id,
                          fg_color, bg_color, ANTHRACITE, font, nullptr, false, edit_pos, xtext, ytext);

    parent.set_widget_focus(&wedit, Widget2::focus_reason_tabkey);
    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc0\x77\x32\x6a\x03\x5e\x6f\x3d\xfa\x34\x18\xa0\xe7\x97\x83\xd7\x96\x70\xd5\x52"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


BOOST_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = BLACK;
    int bg_color = WHITE;
    int id = 0;
    int16_t x = 50;
    int16_t y = 100;
    uint16_t cx = 100;
    int xtext = 4;
    int ytext = 1;
    size_t edit_pos = -1;

    WidgetEditValid wedit(drawable.gd, x, y, cx, parent, notifier, nullptr, id,
                          fg_color, bg_color, ANTHRACITE, font, nullptr, false, edit_pos, xtext, ytext);

    parent.add_widget(&wedit);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid2.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x2a\x1c\x5d\x40\x6e\xc7\x3f\xc2\x30\x97\x88\xa5\xcd\xd5\x31\x11\x54\x06\x26\xda"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.x() + 2, wedit.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.x() + 2, wedit.y() + 2, nullptr);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalid3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x7a\x6e\x2f\x6d\x30\x13\x07\x9a\xbf\x10\x34\x86\x46\x5b\xfa\x06\x24\xb7\x27\x64"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wedit.set_text("Ylajali");

    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xab\xac\x98\xab\x2b\x95\x45\xba\x6f\xca\x90\xe4\xcf\xb1\xa0\xc3\x09\x31\x95\x38"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK_EQUAL(std::string("Ylajali"), std::string(wedit.get_text()));


    wedit.set_xy(192,242);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xd8\x14\x04\xa4\x77\xa3\xac\xba\x2d\x0d\x72\x38\x30\xb8\xae\x23\xd6\x52\xa8\x1b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.clear();
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget2::NORMAL_TAB;

    WidgetEditValid wedit1(drawable.gd, 54, 105, 150, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, font, nullptr, false, 0);

    WidgetEditValid wedit2(drawable.gd, 400, 354, 200, parent, notifier, "", id, WHITE, DARK_BLUE, RED, font, nullptr, false, 0);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid6.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x13\x13\xfd\xe8\x88\x3b\xe4\x70\x83\xa3\x33\x9c\x29\x8d\xa1\x3d\x99\x86\xec\x02"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 3, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 3, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xb5\xca\x90\xc6\xf3\xbf\x06\xeb\xcf\x2c\x4a\xb4\x96\x4e\x73\x73\xf7\x8c\x27\xa2"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid8.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x67\x75\xa4\xf3\x00\xf6\xc6\x9e\xbd\x5a\x91\x86\xc5\xbf\xcf\x80\xe5\xd4\x3b\x66"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid9.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x13\xdd\x25\x21\x19\x12\x87\x84\xc8\xcb\x8d\xd9\xf0\x0e\x11\x0f\xac\xd8\x75\xcb"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


BOOST_AUTO_TEST_CASE(TraceWidgetEditLabels)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget2::NORMAL_TAB;

    WidgetEditValid wedit1(drawable.gd, 54, 105, 150, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, font, "edition1", true, 0, 0, 0, false);

    WidgetEditValid wedit2(drawable.gd, 400, 354, 200, parent, notifier, "", id, WHITE, DARK_BLUE, RED, font, "edition2", true, 0, 0, 0, false);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x2c\x99\x06\x6e\xa5\xab\x44\x10\x0c\xd3\xbb\x21\xdb\x5e\x6e\x47\xc0\x9f\x70\x1a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x50\x43\x95\xbc\xc5\xe6\x62\x0e\xec\xa4\x92\x92\x3d\x87\x75\x79\xb2\xc3\x39\xe2"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x67\x06\xdd\x70\x44\x34\x56\x58\x9b\x55\x30\xc4\x9d\x66\x84\xc7\x40\x5d\x00\xe4"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x13\xdd\x25\x21\x19\x12\x87\x84\xc8\xcb\x8d\xd9\xf0\x0e\x11\x0f\xac\xd8\x75\xcb"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x44\x8c\x4a\x65\xe8\x5b\x8c\x6c\x8d\x5b\x5f\x7c\x02\x2c\x04\x7b\xa6\x05\x04\x49"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x9b\x5d\xe4\xd6\xea\x23\xa7\x2d\xb5\xfb\xd3\x1e\xfd\x65\x81\xb3\xd8\x35\x47\x6e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x50\x43\x95\xbc\xc5\xe6\x62\x0e\xec\xa4\x92\x92\x3d\x87\x75\x79\xb2\xc3\x39\xe2"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}
BOOST_AUTO_TEST_CASE(TraceWidgetEditLabelsPassword)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget2::NORMAL_TAB;

    WidgetEditValid wedit1(drawable.gd, 54, 105, 150, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, font, "edition1", true, 0, 0, 0, true);

    WidgetEditValid wedit2(drawable.gd, 400, 354, 200, parent, notifier, "", id, WHITE, DARK_BLUE, RED, font, "edition2", true, 0, 0, 0, true);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xde\xd5\x80\xe8\xe6\x95\x0e\x6e\xfe\x64\xd0\xa6\x08\x7e\x80\x01\x2c\x49\x26\xc3"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x08\x40\xeb\x47\xbe\x25\x55\xec\x37\xe3\xcf\x36\xe0\x37\xab\x35\xde\x1f\xb2\x6f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x34\xfc\x1b\x0c\xc9\x7b\x3d\x02\xa3\xda\x48\x6c\x83\x27\xb8\x8f\xb0\x07\x99\xbe"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x79\x2b\x00\xd2\x15\xd5\x0d\x06\xdb\x71\x61\x3a\xd0\xb0\x74\x67\xf6\xbe\x2a\x5d"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xb2\xb0\xb0\x8f\x5a\x8d\x4f\x1e\x1d\x2d\xf0\xd7\x6e\xed\xf3\x11\x59\xc7\x17\x3a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xab\x47\xb4\xc7\x64\x28\x11\x01\xcd\x6d\xac\x8e\xec\x16\x8a\xaf\xe1\x68\xde\x25"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x08\x40\xeb\x47\xbe\x25\x55\xec\x37\xe3\xcf\x36\xe0\x37\xab\x35\xde\x1f\xb2\x6f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(EventWidgetEditEvents)
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

    WidgetEditValid wedit(drawable.gd, x, y, cx, parent, &notifier, "abcdef", 0, BLACK, WHITE, DARK_BLUE, font, nullptr, false);

    parent.add_widget(&wedit);
    parent.set_widget_focus(&wedit, Widget2::focus_reason_tabkey);

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush1.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xff\xd9\x32\x1d\x74\x81\x1e\xc5\x23\xcf\x2c\xe6\xc1\x6c\x71\x90\x63\xd3\xb4\x97"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xf9\x9b\x1a\xa4\x81\xf3\x5a\xad\x58\x76\xb9\x7e\xda\x1f\xde\xa8\x3d\xb9\x1e\x78"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xff\xd9\x32\x1d\x74\x81\x1e\xc5\x23\xcf\x2c\xe6\xc1\x6c\x71\x90\x63\xd3\xb4\x97"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.x() + 2, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xf9\x9b\x1a\xa4\x81\xf3\x5a\xad\x58\x76\xb9\x7e\xda\x1f\xde\xa8\x3d\xb9\x1e\x78"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xff\xd9\x32\x1d\x74\x81\x1e\xc5\x23\xcf\x2c\xe6\xc1\x6c\x71\x90\x63\xd3\xb4\x97"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          0, 0, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xf9\x9b\x1a\xa4\x81\xf3\x5a\xad\x58\x76\xb9\x7e\xda\x1f\xde\xa8\x3d\xb9\x1e\x78"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = nullptr;


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xf9\x9b\x1a\xa4\x81\xf3\x5a\xad\x58\x76\xb9\x7e\xda\x1f\xde\xa8\x3d\xb9\x1e\x78"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = nullptr;
}
