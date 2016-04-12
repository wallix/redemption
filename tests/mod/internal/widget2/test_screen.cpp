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

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget2/flat_button.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

struct Notify : public NotifyApi {
    Widget2* sender = nullptr;
    notify_event_t event = 0;

    Notify() = default;

    virtual void notify(Widget2* sender, notify_event_t event) override
    {
        this->sender = sender;
        this->event = event;
    }
};

BOOST_AUTO_TEST_CASE(TestScreenEvent)
{
    TestDraw drawable(800, 600);
    Theme colors;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen wscreen(drawable, drawable.gd.width(), drawable.gd.height(), font);

    wscreen.refresh(wscreen.rect);
    wscreen.tab_flag = Widget2::NORMAL_TAB;
    Notify notifier1;
    Notify notifier2;
    Notify notifier3;
    Notify notifier4;

    WidgetFlatButton wbutton1(drawable, 0, 0, wscreen, &notifier1, "button 1",
                              true, 0, WHITE, DARK_BLUE_BIS, WINBLUE, font);
    WidgetFlatButton wbutton2(drawable, 0, 30, wscreen, &notifier2, "button 2",
                              true, 0, WHITE, DARK_BLUE_BIS, WINBLUE, font);
    WidgetFlatButton wbutton3(drawable, 100, 0, wscreen, &notifier3, "button 3",
                              true, 0, WHITE, DARK_BLUE_BIS, WINBLUE, font);
    WidgetFlatButton wbutton4(drawable, 100, 30, wscreen, &notifier4, "button 4",
                              true, 0, WHITE, DARK_BLUE_BIS, WINBLUE, font);

    wscreen.add_widget(&wbutton1);
    wscreen.add_widget(&wbutton2);
    wscreen.add_widget(&wbutton3);
    wscreen.add_widget(&wbutton4);

    wscreen.set_widget_focus(&wbutton2, Widget2::focus_reason_tabkey);

    wscreen.refresh(wscreen.rect);

    char message[1024];

    BOOST_CHECK(notifier1.sender == nullptr);
    BOOST_CHECK(notifier2.sender == &wbutton2);
    BOOST_CHECK(notifier3.sender == nullptr);
    BOOST_CHECK(notifier4.sender == nullptr);
    BOOST_CHECK(notifier2.event == NOTIFY_FOCUS_BEGIN);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x49\x97\x1f\xdf\x2a\xfd\x3b\xa8\xcf\x22\x22\x83\x6e\x73\xa3\x24\x5f\xf4\x15\xb7"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == nullptr);
    BOOST_CHECK(notifier2.sender == &wbutton2);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == nullptr);
    BOOST_CHECK(notifier2.event == NOTIFY_FOCUS_END);
    BOOST_CHECK(notifier3.event == NOTIFY_FOCUS_BEGIN);
    notifier2.sender = nullptr;
    notifier3.sender = nullptr;
    notifier2.event = 0;
    notifier3.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen2.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x10\x17\x80\x3d\xfb\x0f\x3f\xb3\x97\xca\xad\x84\xf5\x71\x73\x2e\x35\x08\xbe\x06"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == nullptr);
    BOOST_CHECK(notifier2.sender == nullptr);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier3.event == NOTIFY_FOCUS_END);
    BOOST_CHECK(notifier4.event == NOTIFY_FOCUS_BEGIN);
    notifier3.sender = nullptr;
    notifier4.sender = nullptr;
    notifier3.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen3.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x8c\xfe\x9f\xab\x76\x33\xc1\x16\x0f\xe4\x95\x68\x06\x9d\xf1\x44\x68\x88\x0e\x46"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == nullptr);
    BOOST_CHECK(notifier3.sender == nullptr);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier1.event == NOTIFY_FOCUS_BEGIN);
    BOOST_CHECK(notifier4.event == NOTIFY_FOCUS_END);
    notifier1.sender = nullptr;
    notifier4.sender = nullptr;
    notifier1.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen4.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xe3\x48\x04\x83\x3d\xfc\x06\xd9\xaa\x8f\x66\x00\xfd\x4d\x7f\x46\x4e\x1c\xfa\xa4"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == nullptr);
    BOOST_CHECK(notifier3.sender == nullptr);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier1.event == NOTIFY_FOCUS_END);
    BOOST_CHECK(notifier4.event == NOTIFY_FOCUS_BEGIN);
    notifier1.sender = nullptr;
    notifier4.sender = nullptr;
    notifier1.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen5.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x8c\xfe\x9f\xab\x76\x33\xc1\x16\x0f\xe4\x95\x68\x06\x9d\xf1\x44\x68\x88\x0e\x46"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == nullptr);
    BOOST_CHECK(notifier2.sender == nullptr);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier3.event == NOTIFY_FOCUS_BEGIN);
    BOOST_CHECK(notifier4.event == NOTIFY_FOCUS_END);
    notifier3.sender = nullptr;
    notifier4.sender = nullptr;
    notifier3.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen6.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x10\x17\x80\x3d\xfb\x0f\x3f\xb3\x97\xca\xad\x84\xf5\x71\x73\x2e\x35\x08\xbe\x06"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                            wbutton1.dx(), wbutton1.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == nullptr);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == nullptr);
    BOOST_CHECK(notifier1.event == NOTIFY_FOCUS_BEGIN);
    BOOST_CHECK(notifier3.event == NOTIFY_FOCUS_END);
    notifier1.sender = nullptr;
    notifier3.sender = nullptr;
    notifier1.event = 0;
    notifier3.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen7.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x9b\x0c\x48\xa6\xf5\xae\x1b\xa1\xd7\xba\xdf\xf0\xd1\x86\xad\x3c\x04\x40\x1a\xdd"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton2.dx(), wbutton2.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == nullptr);
    BOOST_CHECK(notifier2.sender == nullptr);
    BOOST_CHECK(notifier3.sender == nullptr);
    BOOST_CHECK(notifier4.sender == nullptr);
    BOOST_CHECK(notifier1.event == 0);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen8.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xe3\x48\x04\x83\x3d\xfc\x06\xd9\xaa\x8f\x66\x00\xfd\x4d\x7f\x46\x4e\x1c\xfa\xa4"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == &wbutton2);
    BOOST_CHECK(notifier3.sender == nullptr);
    BOOST_CHECK(notifier4.sender == nullptr);
    BOOST_CHECK(notifier1.event == NOTIFY_FOCUS_END);
    BOOST_CHECK(notifier2.event == NOTIFY_FOCUS_BEGIN);
    notifier1.sender = nullptr;
    notifier2.sender = nullptr;
    notifier1.event = 0;
    notifier2.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen9.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x49\x97\x1f\xdf\x2a\xfd\x3b\xa8\xcf\x22\x22\x83\x6e\x73\xa3\x24\x5f\xf4\x15\xb7"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                            wbutton4.dx(), wbutton4.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == nullptr);
    BOOST_CHECK(notifier2.sender == &wbutton2);
    BOOST_CHECK(notifier3.sender == nullptr);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier2.event == NOTIFY_FOCUS_END);
    BOOST_CHECK(notifier4.event == NOTIFY_FOCUS_BEGIN);
    notifier2.sender = nullptr;
    notifier4.sender = nullptr;
    notifier2.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen10.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xb2\x1c\xdd\xe3\x82\xda\x9e\x53\x37\x70\xc2\xa3\xef\x5d\xb0\x04\x01\x6c\xb8\xbb"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton4.dx(), wbutton4.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == nullptr);
    BOOST_CHECK(notifier2.sender == nullptr);
    BOOST_CHECK(notifier3.sender == nullptr);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier4.event == NOTIFY_SUBMIT);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen11.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x8c\xfe\x9f\xab\x76\x33\xc1\x16\x0f\xe4\x95\x68\x06\x9d\xf1\x44\x68\x88\x0e\x46"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.show_tooltip(nullptr, "tooltip test", 30, 35);

    wscreen.rdp_input_invalidate(wscreen.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen12.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xf3\x4c\xce\x96\x35\x02\xb4\x5c\xb0\x19\xf6\xa9\xa4\xb4\xf9\x26\x75\x7c\x53\x2d"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.show_tooltip(nullptr, nullptr, 30, 35);
    wscreen.rdp_input_invalidate(wscreen.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen13.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x8c\xfe\x9f\xab\x76\x33\xc1\x16\x0f\xe4\x95\x68\x06\x9d\xf1\x44\x68\x88\x0e\x46"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    wscreen.clear();
}
