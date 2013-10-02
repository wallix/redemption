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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x4a\x82\xfb\x92\xf0\xbb\x4c\x98\x3d\x4c"
    //     "\xfe\x39\xa3\x87\xbb\x41\xc1\xff\xdb\x65"
    //                )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x30\xe9\xf0\xae\xb0\x31\xae\x48\xf2\x6d"
                   "\x8d\x11\x54\x66\xe4\x58\xf3\x63\x67\xfd"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xff\xe5\xbc\x52\xc6\x5a\x98\xd0\x82\xfc"
    //     "\x36\x9b\xb4\x73\x18\x19\xd3\x4d\x66\xcd"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9b\x53\xfe\x82\x86\x25\xd2\x41\x82\xf5"
                   "\x07\x69\x8a\x4f\xeb\x5d\x69\x8f\xb4\xfb"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xe5\x54\x52\xaf\xdb\x0e\x7b\x03\xf9\x54"
    //     "\x58\x72\x1a\x05\xf2\x21\x57\x2b\xa8\x46"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x43\x62\xf8\xe9\xcd\x64\x99\xc3\x67\xa3"
                   "\xf9\x5c\xf9\x64\xb3\x75\x85\xdf\xa4\x08"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xb6\xc8\xfc\x80\xd1\x71\x62\x09\x60\x04"
    //     "\x8b\x3b\x4a\x7e\x5f\xcb\xef\x17\xa4\x1f"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\xff\x91\x14\x99\x1f\xce\x55\x9a\x1a\x5a"
                   "\xa8\x60\x96\xa8\x0a\x2e\xa3\x54\x70\xe0"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xe5\x54\x52\xaf\xdb\x0e\x7b\x03\xf9\x54"
    //     "\x58\x72\x1a\x05\xf2\x21\x57\x2b\xa8\x46"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x43\x62\xf8\xe9\xcd\x64\x99\xc3\x67\xa3"
                   "\xf9\x5c\xf9\x64\xb3\x75\x85\xdf\xa4\x08"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xff\xe5\xbc\x52\xc6\x5a\x98\xd0\x82\xfc"
    //     "\x36\x9b\xb4\x73\x18\x19\xd3\x4d\x66\xcd"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9b\x53\xfe\x82\x86\x25\xd2\x41\x82\xf5"
                   "\x07\x69\x8a\x4f\xeb\x5d\x69\x8f\xb4\xfb"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x4e\x10\xd5\x60\x33\xec\x6c\xf4\x75\x22"
    //     "\x3c\x97\x0b\x7e\xf4\x69\x35\xb5\x4e\x9d"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x74\x3f\x21\x10\x62\xaa\xce\x94\x4c\xb1"
                   "\x86\xc8\x01\x0a\x49\xbe\xfd\xb8\x3c\x31"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xb6\xc8\xfc\x80\xd1\x71\x62\x09\x60\x04"
    //     "\x8b\x3b\x4a\x7e\x5f\xcb\xef\x17\xa4\x1f"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\xff\x91\x14\x99\x1f\xce\x55\x9a\x1a\x5a"
                   "\xa8\x60\x96\xa8\x0a\x2e\xa3\x54\x70\xe0"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x4a\x82\xfb\x92\xf0\xbb\x4c\x98\x3d\x4c"
    //                "\xfe\x39\xa3\x87\xbb\x41\xc1\xff\xdb\x65"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x30\xe9\xf0\xae\xb0\x31\xae\x48\xf2\x6d"
                   "\x8d\x11\x54\x66\xe4\x58\xf3\x63\x67\xfd"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xc2\xbd\x62\x95\xee\x4f\xb9\x15\x90\x4c"
    //     "\xa3\xa1\xb8\x5b\x0a\x84\xbe\xa1\xc2\x09"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x33\x65\x86\x9d\x92\xeb\x59\x55\x65\x8d"
                   "\x7c\xb3\x04\xc5\x88\x86\x21\x0a\xe7\xb4"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xe5\x54\x52\xaf\xdb\x0e\x7b\x03\xf9\x54"
    //     "\x58\x72\x1a\x05\xf2\x21\x57\x2b\xa8\x46"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x43\x62\xf8\xe9\xcd\x64\x99\xc3\x67\xa3"
                   "\xf9\x5c\xf9\x64\xb3\x75\x85\xdf\xa4\x08"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.clear();
}
