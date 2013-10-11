/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Meng Tan

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestScreen
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/flat_button.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/composite.hpp"
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
};

BOOST_AUTO_TEST_CASE(TestScreenEvent)
{
    TestDraw drawable(800, 600);
    WidgetScreen wscreen(drawable, drawable.gd.drawable.width, drawable.gd.drawable.height);

    wscreen.refresh(wscreen.rect);
    wscreen.tab_flag = Widget2::NORMAL_TAB;
    Notify notifier1;
    Notify notifier2;
    Notify notifier3;
    Notify notifier4;

    WidgetFlatButton wbutton1(drawable, 0, 0, wscreen, &notifier1, "button 1");
    WidgetFlatButton wbutton2(drawable, 0, 30, wscreen, &notifier2, "button 2");
    WidgetFlatButton wbutton3(drawable, 100, 0, wscreen, &notifier3, "button 3");
    WidgetFlatButton wbutton4(drawable, 100, 30, wscreen, &notifier4, "button 4");

    wscreen.add_widget(&wbutton1);
    wscreen.add_widget(&wbutton2);
    wscreen.add_widget(&wbutton3);
    wscreen.add_widget(&wbutton4);

    wscreen.set_widget_focus(&wbutton2);

    wscreen.refresh(wscreen.rect);

    char message[1024];

    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == &wbutton2);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == 0);
    BOOST_CHECK(notifier2.event == FOCUS_BEGIN);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x74\x69\x18\x27\x93\x1c\x1e\xae\x6a\xef"
                   "\xae\x75\x1c\x04\xb2\x48\x8d\x8e\xb6\xe0"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == &wbutton2);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == 0);
    BOOST_CHECK(notifier2.event == FOCUS_END);
    BOOST_CHECK(notifier3.event == FOCUS_BEGIN);
    notifier2.sender = 0;
    notifier3.sender = 0;
    notifier2.event = 0;
    notifier3.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x19\x1e\x37\x8d\x90\x58\x36\x82\x21\x51"
                   "\x12\x68\xec\x92\x3f\x37\x9f\x23\x4f\xf5"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier3.event == FOCUS_END);
    BOOST_CHECK(notifier4.event == FOCUS_BEGIN);
    notifier3.sender = 0;
    notifier4.sender = 0;
    notifier3.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen3.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xf9\x47\x9e\x37\xd2\x04\x8c\xec\xf6\xe9"
                   "\xb6\xc9\x57\xb2\xe2\x34\x8d\xc7\x06\x5c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier1.event == FOCUS_BEGIN);
    BOOST_CHECK(notifier4.event == FOCUS_END);
    notifier1.sender = 0;
    notifier4.sender = 0;
    notifier1.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen4.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x58\x5e\xbb\x1d\x23\x70\x76\xc9\x54\xe9"
                   "\x86\xbb\xa4\x77\xf0\xd2\x86\x8a\xe1\x3b"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier1.event == FOCUS_END);
    BOOST_CHECK(notifier4.event == FOCUS_BEGIN);
    notifier1.sender = 0;
    notifier4.sender = 0;
    notifier1.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen5.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xf9\x47\x9e\x37\xd2\x04\x8c\xec\xf6\xe9"
                   "\xb6\xc9\x57\xb2\xe2\x34\x8d\xc7\x06\x5c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier3.event == FOCUS_BEGIN);
    BOOST_CHECK(notifier4.event == FOCUS_END);
    notifier3.sender = 0;
    notifier4.sender = 0;
    notifier3.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen6.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x19\x1e\x37\x8d\x90\x58\x36\x82\x21\x51"
                   "\x12\x68\xec\x92\x3f\x37\x9f\x23\x4f\xf5"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                            wbutton1.dx(), wbutton1.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == 0);
    BOOST_CHECK(notifier1.event == FOCUS_BEGIN);
    BOOST_CHECK(notifier3.event == FOCUS_END);
    notifier1.sender = 0;
    notifier3.sender = 0;
    notifier1.event = 0;
    notifier3.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen7.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x24\xcd\x37\x42\xc5\xda\xd7\xe3\x27\x73"
                   "\x7d\x06\x26\x08\x1b\x79\x97\x86\x64\xef"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton2.dx(), wbutton2.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == 0);
    BOOST_CHECK(notifier1.event == 0);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen8.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x58\x5e\xbb\x1d\x23\x70\x76\xc9\x54\xe9"
                   "\x86\xbb\xa4\x77\xf0\xd2\x86\x8a\xe1\x3b"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == &wbutton2);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == 0);
    BOOST_CHECK(notifier1.event == FOCUS_END);
    BOOST_CHECK(notifier2.event == FOCUS_BEGIN);
    notifier1.sender = 0;
    notifier2.sender = 0;
    notifier1.event = 0;
    notifier2.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen9.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x74\x69\x18\x27\x93\x1c\x1e\xae\x6a\xef"
                   "\xae\x75\x1c\x04\xb2\x48\x8d\x8e\xb6\xe0"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                            wbutton4.dx(), wbutton4.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == &wbutton2);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier2.event == FOCUS_END);
    BOOST_CHECK(notifier4.event == FOCUS_BEGIN);
    notifier2.sender = 0;
    notifier4.sender = 0;
    notifier2.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen10.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x65\xf6\x7c\x32\x30\xc9\x87\xc1\x6d\xc3"
                   "\x7e\xb4\x8f\xbd\x70\x7e\x80\xf4\x9e\x36"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton4.dx(), wbutton4.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier4.event == NOTIFY_SUBMIT);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen11.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xf9\x47\x9e\x37\xd2\x04\x8c\xec\xf6\xe9"
                   "\xb6\xc9\x57\xb2\xe2\x34\x8d\xc7\x06\x5c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.show_tooltip(NULL, "tooltip test", 30, 35);

    wscreen.rdp_input_invalidate(wscreen.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "screen12.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xf7\x64\xa8\x5b\x50\x00\xb1\x63\xfc\xb2"
                   "\x20\x1a\x8c\x9d\x45\x43\x6e\x95\x55\x06"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.show_tooltip(NULL, NULL, 30, 35);
    wscreen.rdp_input_invalidate(wscreen.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen13.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xf9\x47\x9e\x37\xd2\x04\x8c\xec\xf6\xe9"
                   "\xb6\xc9\x57\xb2\xe2\x34\x8d\xc7\x06\x5c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    wscreen.clear();


}
