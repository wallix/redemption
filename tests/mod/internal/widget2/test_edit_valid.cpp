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
        "\xb4\x9e\xa4\x1f\x87\x5f\x0a\xfa\x47\x18\xd5\x1d\x17\x6f\x89\x8e\x3f\xc4\x6f\x8e"
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
        "\x60\x94\x19\x4b\xa3\xfe\xdd\x62\x2c\xc3\xc6\x26\xc4\xcd\x5e\x4d\x05\xac\xc0\x14"
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
        "\x13\x90\x5f\xd9\x12\x95\x7d\x04\x69\x21\xde\x98\xe7\x20\xef\x2a\x6c\x8a\x51\xeb"
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
        "\x22\x15\x45\xcd\x67\xf9\x21\xa2\xfe\x61\xb7\x03\xa8\xd8\x4d\xf6\x45\x31\xc3\xda"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK_EQUAL(std::string("Ylajali"), std::string(wedit.get_text()));


    wedit.set_xy(192,242);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xa4\xce\x24\xc2\x41\x18\x23\xdf\xe0\x0d\xb0\xc7\x0d\xc9\x9c\x6e\xe5\x55\x25\x9f"
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
        "\xa6\xfd\x3c\xc6\x72\xc7\x63\xca\x36\x35\x96\xfb\x04\xac\xef\x4c\x35\x12\x8a\xcb"
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
        "\x06\x38\xa7\x92\x81\x06\xa9\xb6\x22\xfc\xee\x21\x97\xcd\x9a\xd8\x77\xd0\xae\x0f"
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
        "\xc1\xb2\x96\xdf\x74\xff\x2f\xca\x45\x98\x9b\xcf\xf9\xc7\xdc\x05\xc2\x2a\x90\x21"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid9.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xe1\x31\x6a\x10\x0d\x6e\x28\xeb\xb0\xdc\xcb\x65\x53\x3e\x06\x0c\x68\x43\x55\x92"
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
        "\x93\x01\xd5\xc3\xfa\xd9\x53\x44\x90\xc4\x44\xbb\xa2\xe3\x46\x7e\x9d\xae\xcd\xf3"
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
        "\x24\x4c\x36\x82\x87\x20\x16\x5c\xbf\xd0\xe9\xf0\x7f\xc9\xe3\x28\x23\x8d\x89\x32"
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
        "\x32\x4e\xec\xfc\x6b\xa2\xcf\xda\xd6\x97\xce\x55\xc8\x94\x6a\xfa\xf0\x4a\x19\xda"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xe1\x31\x6a\x10\x0d\x6e\x28\xeb\xb0\xdc\xcb\x65\x53\x3e\x06\x0c\x68\x43\x55\x92"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x88\x07\x91\xc7\x4f\xc2\xde\x75\x8b\xbb\xe9\x28\x04\x5e\xdd\x71\x37\x39\xc4\xdf"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x04\x06\xeb\x97\xe3\xd4\x55\x5c\xb7\x51\x03\x79\x61\x79\xae\x1f\xd3\x74\xd2\x87"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x24\x4c\x36\x82\x87\x20\x16\x5c\xbf\xd0\xe9\xf0\x7f\xc9\xe3\x28\x23\x8d\x89\x32"
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
        "\x15\xab\x61\x8e\x93\x7c\xbc\xac\x7c\x7e\x1a\x09\x19\xd1\x76\x04\xc0\xde\x6e\x62"
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
        "\x31\x23\x5c\x2a\x80\xa9\x54\xb7\x3e\x06\x84\x79\xae\x3b\x6a\xff\xf3\x59\x65\xfc"
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
        "\x3e\x9e\x78\xcc\x28\xcd\x88\x6e\xa3\x8f\xe1\x1b\x61\x23\xa9\x8b\xb7\x1a\x5e\xc5"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x56\xb3\xd8\x8d\xf3\xd3\x49\x13\x76\x13\x3b\x30\xdc\x67\xd2\x0f\xc9\xa0\x1d\x3a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x3a\x8b\x6e\x21\x1d\x52\xcb\xb6\x92\x3d\xbc\x74\xa7\x06\xce\x5c\x15\xbb\x67\x81"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x42\xad\x80\xa8\x65\x6c\x59\x66\x31\x70\x31\xae\xc2\x94\xd9\xa2\xd9\xff\x28\x7e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x31\x23\x5c\x2a\x80\xa9\x54\xb7\x3e\x06\x84\x79\xae\x3b\x6a\xff\xf3\x59\x65\xfc"
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
        "\x2d\x0e\x3b\x69\x9f\x39\x04\xe3\xf4\xb2\xc0\xda\x71\xcc\xa7\x32\xac\x46\xdf\xf4"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x15\x6a\x56\x04\x2e\xe2\x27\xaf\xfd\xcc\x24\x2d\x81\x23\x0b\x06\x74\xba\xbb\xbb"
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
        "\x2d\x0e\x3b\x69\x9f\x39\x04\xe3\xf4\xb2\xc0\xda\x71\xcc\xa7\x32\xac\x46\xdf\xf4"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.x() + 2, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x15\x6a\x56\x04\x2e\xe2\x27\xaf\xfd\xcc\x24\x2d\x81\x23\x0b\x06\x74\xba\xbb\xbb"
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
        "\x2d\x0e\x3b\x69\x9f\x39\x04\xe3\xf4\xb2\xc0\xda\x71\xcc\xa7\x32\xac\x46\xdf\xf4"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          0, 0, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x15\x6a\x56\x04\x2e\xe2\x27\xaf\xfd\xcc\x24\x2d\x81\x23\x0b\x06\x74\xba\xbb\xbb"
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
        "\x15\x6a\x56\x04\x2e\xe2\x27\xaf\xfd\xcc\x24\x2d\x81\x23\x0b\x06\x74\xba\xbb\xbb"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = nullptr;
}
