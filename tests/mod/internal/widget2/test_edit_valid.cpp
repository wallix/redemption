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
    int xtext = 4;
    int ytext = 1;
    size_t edit_pos = 2;

    WidgetEditValid wedit(drawable, x, y, cx, parent, notifier, "test1", id,
                          fg_color, bg_color, DARK_GREY, WINBLUE, edit_pos, xtext, ytext);

    parent.set_widget_focus(&wedit);
    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xb7\xd8\xe4\x14\xe6\x8e\x6f\x10\x20\x21"
                   "\x71\x81\x29\xd3\xcc\xbb\x30\x64\x9e\x75"
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
                          fg_color, bg_color, DARK_GREY, WINBLUE, edit_pos, xtext, ytext);

    parent.add_widget(&wedit);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x01\xad\x67\x9e\xbd\x9c\x28\xd7\x3b\xb9"
                   "\xd9\xd0\x84\x2d\xed\x90\xe3\x6c\xf9\x9f"
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
                   "\x4e\xa8\x74\x61\x93\x59\x35\x71\x83\xd4"
                   "\x7e\x48\x72\xfe\x98\xd9\x43\xa1\x1a\xdb"
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
                   "\xbd\x0f\x4d\x91\x7c\x0b\xa7\x21\x5c\x6a"
                   "\x7a\x3c\x4b\x3b\xa7\xaa\xd3\x82\x78\x91"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK_EQUAL(std::string("Ylajali"), std::string(wedit.get_text()));


    wedit.set_xy(192,242);

    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid5.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x02\x91\x96\xd7\x1f\x2c\x23\xa5\x88\xfa"
                   "\xef\x70\xea\x76\xc3\x31\xaa\xce\x00\x21"
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

    WidgetEditValid wedit1(drawable, 54, 105, 150, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, WINBLUE, 0);

    WidgetEditValid wedit2(drawable, 400, 354, 200, parent, notifier, "", id, WHITE, DARK_BLUE, RED, WINBLUE, 0);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x38\x7d\xce\xb1\xfb\xf9\xa7\xe0\xc9\xa2"
                   "\xe3\x48\x9c\xda\x2c\x48\xa2\x38\x80\x97"
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
                   "\x9a\xda\x3f\xb6\x47\xf4\x10\xba\xd5\x53"
                   "\x64\xad\xa2\x6b\x3f\x18\x5d\x50\x5a\xef"
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
                   "\xb8\x5c\x9f\xf7\x22\x0b\x09\x01\x1c\xa7"
                   "\x76\x11\x5f\x20\xaf\x88\x10\x69\xab\x55"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid9.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x3d\x06\xd9\xd4\x9e\x0b\x71\x9a\xf3\x73"
                   "\xed\x5c\xd8\x93\xad\x7f\xa5\x92\x48\xc0"
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

    WidgetEditValid wedit(drawable, x, y, cx, parent, &notifier, "abcdef", 0, BLACK, WHITE, DARK_BLUE, WINBLUE);

    parent.add_widget(&wedit);
    parent.set_widget_focus(&wedit);

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.lx() - 5, wedit.centery(), NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush1.png");

    char message[1024];

    if (!check_sig(drawable.gd.drawable, message,
                   "\x91\xc3\xb2\xfe\x5f\xc6\xc5\x06\x60\x7e"
                   "\xd0\x3d\x33\x86\x42\x48\xab\x79\xb5\x58"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.lx() - 5, wedit.centery(), NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x87\x7f\x9c\xbf\x1a\xe2\xa0\x7e\x1f\x57"
                   "\xd7\xe5\x3c\xd8\xa4\xe6\xbd\x0b\xf8\xe0"
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
                   "\x91\xc3\xb2\xfe\x5f\xc6\xc5\x06\x60\x7e"
                   "\xd0\x3d\x33\x86\x42\x48\xab\x79\xb5\x58"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.centerx(), wedit.centery(), NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush4.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x87\x7f\x9c\xbf\x1a\xe2\xa0\x7e\x1f\x57"
                   "\xd7\xe5\x3c\xd8\xa4\xe6\xbd\x0b\xf8\xe0"
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
                   "\x91\xc3\xb2\xfe\x5f\xc6\xc5\x06\x60\x7e"
                   "\xd0\x3d\x33\x86\x42\x48\xab\x79\xb5\x58"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          0, 0, NULL);
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush6.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x87\x7f\x9c\xbf\x1a\xe2\xa0\x7e\x1f\x57"
                   "\xd7\xe5\x3c\xd8\xa4\xe6\xbd\x0b\xf8\xe0"
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
                   "\x87\x7f\x9c\xbf\x1a\xe2\xa0\x7e\x1f\x57"
                   "\xd7\xe5\x3c\xd8\xa4\xe6\xbd\x0b\xf8\xe0"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = 0;
}
