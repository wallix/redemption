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

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/edit.hpp"
#include "internal/widget2/edit_valid.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/composite.hpp"

// #include "internal/widget2/widget_composite.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif
#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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
                          fg_color, bg_color, ANTHRACITE, edit_pos, xtext, ytext);

    parent.set_widget_focus(&wedit);
    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x09\x2c\x8a\xae\x57\x89\x17\x0f\xe4\x59"
                   "\xcb\x89\xe2\xef\xfe\xe6\xde\x77\xc3\xd3"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


BOOST_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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
                          fg_color, bg_color, ANTHRACITE, edit_pos, xtext, ytext);

    parent.add_widget(&wedit);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x2a\x1c\x5d\x40\x6e\xc7\x3f\xc2\x30\x97"
                   "\x88\xa5\xcd\xd5\x31\x11\x54\x06\x26\xda"
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid3.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x10\x48\x36\x49\xcb\xd6\xdc\xcb\xa1\x08"
                   "\x8b\x7e\x81\xb0\x34\x7b\x15\xd3\xfc\xcf"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wedit.set_text("Ylajali");

    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid4.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x16\x65\xcb\xee\x4e\xcc\x8b\x60\x95\x4a"
                   "\xaa\x07\xca\x40\x69\x12\x07\xb6\xc3\xfd"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK_EQUAL(std::string("Ylajali"), std::string(wedit.get_text()));


    wedit.set_xy(192,242);

    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid5.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x31\xa1\xda\xc6\x2c\x1c\x25\xa1\x32\x6e"
                   "\x0f\xac\x06\x5f\x9b\xa8\xe0\xa1\x79\x0a"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.clear();
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget2::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, 54, 105, 150, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, 0);

    WidgetEditValid wedit2(drawable, 400, 354, 200, parent, notifier, "", id, WHITE, DARK_BLUE, RED, 0);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x5f\x21\xbd\x8f\x6f\x43\xa9\x76\xc0\xc2"
                   "\xe8\xd2\x3d\x6b\x1e\x63\x62\xa1\x1b\xd3"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.centerx(), wedit1.centery(), NULL);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.centerx(), wedit1.centery(), NULL);

    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid7.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xdd\x04\xa2\x95\x55\x8c\xf5\xe6\x8c\x16"
                   "\xa1\xf4\x5e\xc7\xe0\x74\x17\x86\x94\x67"
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

    if (!check_sig(drawable.gd.drawable, message,
                   "\xcc\x43\xf3\x90\x16\xe1\x37\xbc\xe9\xcf"
                   "\xac\x2e\xfb\xc7\x27\x77\x63\xb3\xac\x8f"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid9.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x14\xd8\x17\x26\x71\xcd\xd1\x0e\xd0\x4a"
                   "\x94\xb8\xe9\x25\x87\x84\x8b\x72\x84\x7a"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


BOOST_AUTO_TEST_CASE(TraceWidgetEditLabels)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget2::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, 54, 105, 150, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, 0, 0, 0, false, "edition1");

    WidgetEditValid wedit2(drawable, 400, 354, 200, parent, notifier, "", id, WHITE, DARK_BLUE, RED, 0, 0, 0, false, "edition2");

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x70\x36\xa0\xc0\x16\x20\xbc\x08\x2a\x06"
                   "\x4c\x69\x2e\x87\xa3\x97\x60\x1a\x65\xac"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.centerx(), wedit1.centery(), NULL);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.centerx(), wedit1.centery(), NULL);

    parent.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x2a\x09\x42\xa4\xf7\xc6\x8b\x5a\x11\xda"
                   "\x47\x8c\x3b\xc0\x0a\x0d\x0f\x78\x5c\xa1"
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

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel3.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x18\xba\xdc\xab\x2e\x15\xb9\xeb\x6b\x25"
                   "\xec\x67\x4c\x83\x7f\xfe\x4e\x36\x0d\xcf"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel4.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x14\xd8\x17\x26\x71\xcd\xd1\x0e\xd0\x4a"
                   "\x94\xb8\xe9\x25\x87\x84\x8b\x72\x84\x7a"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel5.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x8a\xda\xbb\x2f\x91\xa0\x97\x3c\x70\xe4"
                   "\x05\x13\xbd\x24\x11\x8b\xa2\x68\x62\xad"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel6.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x3f\xea\xbb\x60\xc6\x4c\x6a\x8c\xf0\xe7"
                   "\x3f\x88\x46\x16\xd6\x0d\x8e\xfa\xe7\x59"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel7.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x2a\x09\x42\xa4\xf7\xc6\x8b\x5a\x11\xda"
                   "\x47\x8c\x3b\xc0\x0a\x0d\x0f\x78\x5c\xa1"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}
BOOST_AUTO_TEST_CASE(TraceWidgetEditLabelsPassword)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget2::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, 54, 105, 150, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, 0, 0, 0, true, "edition1");

    WidgetEditValid wedit2(drawable, 400, 354, 200, parent, notifier, "", id, WHITE, DARK_BLUE, RED, 0, 0, 0, true, "edition2");

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x00\xbf\x12\x52\xa8\xad\x8e\x3f\x5a\x6a"
                   "\xdc\x26\x1a\x10\x0b\x46\x29\xa1\xc4\xcc"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.centerx(), wedit1.centery(), NULL);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.centerx(), wedit1.centery(), NULL);

    parent.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xcd\xca\x28\x8f\x2d\xe7\x07\xab\x82\x62"
                   "\x14\x57\x18\x80\xa9\x8f\xc9\x74\x7b\x7c"
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

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass3.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x00\x24\x21\xf5\x22\xf2\x99\x3c\x9f\x63"
                   "\x2a\xa9\xb7\x04\x8e\x9a\x6a\x8e\x91\x72"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass4.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xd4\xe7\x43\x1d\x6d\x74\x17\x10\xbb\x46"
                   "\xb1\xae\xe6\xa9\x60\x76\x41\x3a\x6c\xd8"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass5.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xc3\x26\xf7\xf8\xcd\x69\x5a\xb7\xa6\x62"
                   "\xc5\xe4\x86\xfb\x68\x4a\x01\xc8\x65\x06"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass6.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x35\x7a\x44\x04\x82\x88\xa1\xc1\xc5\x4f"
                   "\xdb\xc6\xa1\x78\xef\x09\xc4\xab\xb3\xfb"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass7.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xcd\xca\x28\x8f\x2d\xe7\x07\xab\x82\x62"
                   "\x14\x57\x18\x80\xa9\x8f\xc9\x74\x7b\x7c"
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
    WidgetScreen parent(drawable, 800, 600);

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEditValid wedit(drawable, x, y, cx, parent, &notifier, "abcdef", 0, BLACK, WHITE, DARK_BLUE);

    parent.add_widget(&wedit);
    parent.set_widget_focus(&wedit);

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.lx() - 5, wedit.centery(), NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush1.png");

    char message[1024];

    if (!check_sig(drawable.gd.drawable, message,
                   "\x9e\x7e\xbf\x26\xd1\x02\x15\x49\x85\xf1"
                   "\x13\x0d\xab\xa7\x74\xce\xe6\x73\x56\x14"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.lx() - 5, wedit.centery(), NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x2f\x49\xb8\xec\x34\x44\x1a\xbf\x77\x02"
                   "\x17\x43\x91\x2c\x92\x98\x7d\x52\x16\x6a"
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

    if (!check_sig(drawable.gd.drawable, message,
                   "\x9e\x7e\xbf\x26\xd1\x02\x15\x49\x85\xf1"
                   "\x13\x0d\xab\xa7\x74\xce\xe6\x73\x56\x14"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.centerx(), wedit.centery(), NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush4.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x2f\x49\xb8\xec\x34\x44\x1a\xbf\x77\x02"
                   "\x17\x43\x91\x2c\x92\x98\x7d\x52\x16\x6a"
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

    if (!check_sig(drawable.gd.drawable, message,
                   "\x9e\x7e\xbf\x26\xd1\x02\x15\x49\x85\xf1"
                   "\x13\x0d\xab\xa7\x74\xce\xe6\x73\x56\x14"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          0, 0, NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush6.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x2f\x49\xb8\xec\x34\x44\x1a\xbf\x77\x02"
                   "\x17\x43\x91\x2c\x92\x98\x7d\x52\x16\x6a"
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

    if (!check_sig(drawable.gd.drawable, message,
                   "\x2f\x49\xb8\xec\x34\x44\x1a\xbf\x77\x02"
                   "\x17\x43\x91\x2c\x92\x98\x7d\x52\x16\x6a"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = 0;
}
