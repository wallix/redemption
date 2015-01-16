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
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT

#include "config.hpp"
#include "internal/widget2/edit_valid.hpp"
#include "internal/widget2/screen.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "./"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestDraw drawable(800, 600);

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.font);
    NotifyApi * notifier = NULL;
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

    WidgetEditValid wedit(drawable, x, y, cx, parent, notifier, "test1", id,
                          fg_color, bg_color, ANTHRACITE, ini.font, edit_pos, xtext, ytext);

    parent.set_widget_focus(&wedit, Widget2::focus_reason_tabkey);
    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x43\x1a\x9e\xfb\xab\x79\x1e\xc1\x92\x35\x77\x67\x37\x65\xc0\xe2\x08\x96\x46\xbe"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


BOOST_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestDraw drawable(800, 600);

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.font);
    NotifyApi * notifier = NULL;
    int fg_color = BLACK;
    int bg_color = WHITE;
    int id = 0;
    int16_t x = 50;
    int16_t y = 100;
    uint16_t cx = 100;
    int xtext = 4;
    int ytext = 1;
    size_t edit_pos = -1;

    WidgetEditValid wedit(drawable, x, y, cx, parent, notifier, NULL, id,
                          fg_color, bg_color, ANTHRACITE, ini.font, edit_pos, xtext, ytext);

    parent.add_widget(&wedit);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid2.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x2a\x1c\x5d\x40\x6e\xc7\x3f\xc2\x30\x97\x88\xa5\xcd\xd5\x31\x11\x54\x06\x26\xda"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.centerx(), wedit.centery(), NULL);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.centerx(), wedit.centery(), NULL);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalid3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x28\x1f\xcf\x1a\x29\x79\x32\xb9\x1c\x54\x81\xb9\x52\x35\xd6\x02\xd9\x18\x03\xb7"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wedit.set_text("Ylajali");

    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xac\x0f\xe6\xa3\x1c\x5c\xaa\xd8\x23\xf9\x74\x87\x10\x7f\x9a\x99\x93\xe3\x26\x0e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK_EQUAL(std::string("Ylajali"), std::string(wedit.get_text()));


    wedit.set_xy(192,242);

    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x0d\x98\xe4\xf0\x9a\x17\x25\x97\x47\xa4\x6b\xa5\xf7\xf0\xbf\xe2\x7e\x47\xe3\xf6"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.clear();
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestDraw drawable(800, 600);

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.font);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget2::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, 54, 105, 150, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, ini.font, 0);

    WidgetEditValid wedit2(drawable, 400, 354, 200, parent, notifier, "", id, WHITE, DARK_BLUE, RED, ini.font, 0);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid6.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xd6\x5c\xb4\xd0\x67\xcb\xde\x2e\x67\xe1\x0e\xb3\xc3\x87\x7f\x4e\x48\x41\x2b\x8a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.centerx(), wedit1.centery(), NULL);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.centerx(), wedit1.centery(), NULL);

    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x38\xcd\xa8\xb6\x09\x6a\x45\xd8\x5a\x59\x00\xcc\x8a\xf8\x32\xb8\x1b\xa1\x0f\x83"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    BStream decoded_data(256);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid8.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x48\x87\x70\xd9\xcb\x19\x80\x3c\x78\x39\xf3\x8c\x19\x68\x7a\xfb\x83\x48\x11\x32"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid9.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x90\xf3\xf4\xe8\x54\x42\xb0\xd3\x78\x0a\xd4\xc9\x95\xfe\x8a\x1a\x9b\xf0\x0f\x93"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


BOOST_AUTO_TEST_CASE(TraceWidgetEditLabels)
{
    TestDraw drawable(800, 600);

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.font);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget2::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, 54, 105, 150, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, ini.font, 0, 0, 0, false, "edition1");

    WidgetEditValid wedit2(drawable, 400, 354, 200, parent, notifier, "", id, WHITE, DARK_BLUE, RED, ini.font, 0, 0, 0, false, "edition2");

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xee\x99\x8c\x46\x8d\xa8\x97\x61\x20\xec\x96\x69\x49\x11\x05\xc6\xbc\xb7\xa4\x7e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.centerx(), wedit1.centery(), NULL);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.centerx(), wedit1.centery(), NULL);

    parent.rdp_input_invalidate(parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xe6\x44\xa5\x73\xaa\xa6\xb8\xe9\x24\x40\x75\xe8\x70\xb0\x55\xf8\x40\xcf\x56\x05"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    BStream decoded_data(256);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x12\x97\x1f\x45\xfa\x6a\xc9\x8a\xba\xca\xe1\xc0\x3f\xce\xae\x6a\x79\xb8\x88\xa7"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x90\xf3\xf4\xe8\x54\x42\xb0\xd3\x78\x0a\xd4\xc9\x95\xfe\x8a\x1a\x9b\xf0\x0f\x93"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x61\xd8\x2b\x91\xc4\x2f\x0c\x76\x96\xde\xf8\x99\xa6\x27\x21\xa2\x80\x61\x79\x7b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xd4\x58\xbb\x57\x0e\xa5\xf1\xc9\x6f\x04\xa9\x4c\xf3\xca\xd4\x62\x40\x10\x20\x8b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xe6\x44\xa5\x73\xaa\xa6\xb8\xe9\x24\x40\x75\xe8\x70\xb0\x55\xf8\x40\xcf\x56\x05"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}
BOOST_AUTO_TEST_CASE(TraceWidgetEditLabelsPassword)
{
    TestDraw drawable(800, 600);

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.font);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget2::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, 54, 105, 150, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, ini.font, 0, 0, 0, true, "edition1");

    WidgetEditValid wedit2(drawable, 400, 354, 200, parent, notifier, "", id, WHITE, DARK_BLUE, RED, ini.font, 0, 0, 0, true, "edition2");

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x80\x25\x26\xad\x53\x31\x1b\xf5\xcd\x45\xeb\x7c\xb6\x38\xf8\x1d\xcc\x28\xb5\x9c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.centerx(), wedit1.centery(), NULL);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.centerx(), wedit1.centery(), NULL);

    parent.rdp_input_invalidate(parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x5e\x8c\xcc\x96\x77\x05\x6f\x9c\xaa\x5a\x6f\x8e\x3e\x60\x43\xdb\x2f\xbd\x6c\xa1"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    BStream decoded_data(256);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x3b\xc3\x1d\x74\xb9\x1e\x4b\xfa\x14\xe1\xaa\x70\x92\x03\x67\x04\x5b\xef\xd4\x4a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x16\x0a\x1e\xe1\xc6\x82\xc0\x62\x31\xef\xa4\xc2\xc4\xa5\x4c\xc1\x9c\xcd\x98\x3e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x18\xcd\x04\xf8\xe2\x9e\x11\x4b\xb8\x9b\xb5\xab\x31\xca\xc0\xa8\x4b\x86\x83\xe4"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x15\x32\x42\x60\xe8\xe6\x4f\x68\x88\xb6\x93\xa8\x9e\xbe\x82\x3a\x0e\xf0\xe9\x55"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    //drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x5e\x8c\xcc\x96\x77\x05\x6f\x9c\xaa\x5a\x6f\x8e\x3e\x60\x43\xdb\x2f\xbd\x6c\xa1"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(EventWidgetEditEvents)
{
    TestDraw drawable(800, 600);

    struct WidgetReceiveEvent : public Widget2 {
        Widget2* sender;
        NotifyApi::notify_event_t event;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget2(drawable, Rect(), *this, NULL)
        , sender(0)
        , event(0)
        {}

        virtual void draw(const Rect&)
        {}

        virtual void notify(Widget2* sender, NotifyApi::notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event(drawable);

    struct Notify : public NotifyApi {
        Widget2* sender;
        notify_event_t event;
        Notify()
        : sender(0)
        , event(0)
        {}
        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable, 800, 600, ini.font);

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEditValid wedit(drawable, x, y, cx, parent, &notifier, "abcdef", 0, BLACK, WHITE, DARK_BLUE, ini.font);

    parent.add_widget(&wedit);
    parent.set_widget_focus(&wedit, Widget2::focus_reason_tabkey);

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.lx() - 5, wedit.centery(), NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush1.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xf1\x89\xfa\xd2\xd5\x21\xb8\x57\x2f\xf8\x5b\xa0\x20\x6d\xd7\x52\x21\x96\x46\x2c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.lx() - 5, wedit.centery(), NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x82\x9c\xf4\x2a\xbc\x97\x81\x9e\x32\xa5\x77\x5e\x2a\xed\x5a\xef\xbf\xe8\xd8\x7f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = 0;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.lx() - 5, wedit.centery(), NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xf1\x89\xfa\xd2\xd5\x21\xb8\x57\x2f\xf8\x5b\xa0\x20\x6d\xd7\x52\x21\x96\x46\x2c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.centerx(), wedit.centery(), NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x82\x9c\xf4\x2a\xbc\x97\x81\x9e\x32\xa5\x77\x5e\x2a\xed\x5a\xef\xbf\xe8\xd8\x7f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = 0;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.lx() - 5, wedit.centery(), NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xf1\x89\xfa\xd2\xd5\x21\xb8\x57\x2f\xf8\x5b\xa0\x20\x6d\xd7\x52\x21\x96\x46\x2c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          0, 0, NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x82\x9c\xf4\x2a\xbc\x97\x81\x9e\x32\xa5\x77\x5e\x2a\xed\x5a\xef\xbf\xe8\xd8\x7f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = 0;


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x82\x9c\xf4\x2a\xbc\x97\x81\x9e\x32\xa5\x77\x5e\x2a\xed\x5a\xef\xbf\xe8\xd8\x7f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = 0;
}
