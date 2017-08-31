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

#define RED_TEST_MODULE TestScreen
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget/flat_button.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/mod/fake_draw.hpp"

struct Notify : public NotifyApi {
    Widget* sender = nullptr;
    notify_event_t event = 0;

    Notify() = default;

    void notify(Widget* sender, notify_event_t event) override
    {
        this->sender = sender;
        this->event = event;
    }
};

RED_AUTO_TEST_CASE(TestScreenEvent)
{
    TestDraw drawable(800, 600);
    Theme colors;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen wscreen(drawable.gd, font, nullptr, Theme{});
    wscreen.set_wh(drawable.gd.width(), drawable.gd.height());

    wscreen.rdp_input_invalidate(wscreen.get_rect());
    wscreen.tab_flag = Widget::NORMAL_TAB;
    Notify notifier1;
    Notify notifier2;
    Notify notifier3;
    Notify notifier4;

    WidgetFlatButton wbutton1(drawable.gd, wscreen, &notifier1, "button 1",
                              0, WHITE, DARK_BLUE_BIS, WINBLUE, 2, font);
    Dimension dim = wbutton1.get_optimal_dim();
    wbutton1.set_wh(dim);
    wbutton1.set_xy(0, 0);

    WidgetFlatButton wbutton2(drawable.gd, wscreen, &notifier2, "button 2",
                              0, WHITE, DARK_BLUE_BIS, WINBLUE, 2, font);
    dim = wbutton2.get_optimal_dim();
    wbutton2.set_wh(dim);
    wbutton2.set_xy(0, 30);

    WidgetFlatButton wbutton3(drawable.gd, wscreen, &notifier3, "button 3",
                              0, WHITE, DARK_BLUE_BIS, WINBLUE, 2, font);
    dim = wbutton3.get_optimal_dim();
    wbutton3.set_wh(dim);
    wbutton3.set_xy(100, 0);

    WidgetFlatButton wbutton4(drawable.gd, wscreen, &notifier4, "button 4",
                              0, WHITE, DARK_BLUE_BIS, WINBLUE, 2, font);
    dim = wbutton4.get_optimal_dim();
    wbutton4.set_wh(dim);
    wbutton4.set_xy(100, 30);

    wscreen.add_widget(&wbutton1);
    wscreen.add_widget(&wbutton2);
    wscreen.add_widget(&wbutton3);
    wscreen.add_widget(&wbutton4);

    wscreen.set_widget_focus(&wbutton2, Widget::focus_reason_tabkey);

    wscreen.rdp_input_invalidate(wscreen.get_rect());

    RED_CHECK(notifier1.sender == nullptr);
    RED_CHECK(notifier2.sender == &wbutton2);
    RED_CHECK(notifier3.sender == nullptr);
    RED_CHECK(notifier4.sender == nullptr);
    RED_CHECK(notifier2.event == NOTIFY_FOCUS_BEGIN);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen.png");
    RED_CHECK_SIG(drawable.gd, "\x59\x08\x30\x3d\x32\xec\x54\x34\x2e\x53\x69\x36\x33\x82\x61\x90\x9c\x81\xe6\x34");


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.sender == nullptr);
    RED_CHECK(notifier2.sender == &wbutton2);
    RED_CHECK(notifier3.sender == &wbutton3);
    RED_CHECK(notifier4.sender == nullptr);
    RED_CHECK(notifier2.event == NOTIFY_FOCUS_END);
    RED_CHECK(notifier3.event == NOTIFY_FOCUS_BEGIN);
    notifier2.sender = nullptr;
    notifier3.sender = nullptr;
    notifier2.event = 0;
    notifier3.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen2.png");
    RED_CHECK_SIG(drawable.gd, "\x33\xfc\x94\x92\xf4\xa3\xed\x2d\xe1\x5e\x30\x56\x93\x59\xd6\x53\xb7\xad\x83\x91");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.sender == nullptr);
    RED_CHECK(notifier2.sender == nullptr);
    RED_CHECK(notifier3.sender == &wbutton3);
    RED_CHECK(notifier4.sender == &wbutton4);
    RED_CHECK(notifier3.event == NOTIFY_FOCUS_END);
    RED_CHECK(notifier4.event == NOTIFY_FOCUS_BEGIN);
    notifier3.sender = nullptr;
    notifier4.sender = nullptr;
    notifier3.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen3.png");
    RED_CHECK_SIG(drawable.gd, "\xf7\x36\x47\x38\xf5\x14\xdf\xb1\x2a\x5e\x5c\x3c\x0a\x8d\x7f\x7e\x5f\x7e\xf6\xb3");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.sender == &wbutton1);
    RED_CHECK(notifier2.sender == nullptr);
    RED_CHECK(notifier3.sender == nullptr);
    RED_CHECK(notifier4.sender == &wbutton4);
    RED_CHECK(notifier1.event == NOTIFY_FOCUS_BEGIN);
    RED_CHECK(notifier4.event == NOTIFY_FOCUS_END);
    notifier1.sender = nullptr;
    notifier4.sender = nullptr;
    notifier1.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen4.png");
    RED_CHECK_SIG(drawable.gd, "\x39\xd8\xaa\x23\xe3\xe9\xf7\x41\xee\x53\xa9\xae\xe3\xb2\xb1\x90\x8d\xda\x5a\xb2");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.sender == &wbutton1);
    RED_CHECK(notifier2.sender == nullptr);
    RED_CHECK(notifier3.sender == nullptr);
    RED_CHECK(notifier4.sender == &wbutton4);
    RED_CHECK(notifier1.event == NOTIFY_FOCUS_END);
    RED_CHECK(notifier4.event == NOTIFY_FOCUS_BEGIN);
    notifier1.sender = nullptr;
    notifier4.sender = nullptr;
    notifier1.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen5.png");
    RED_CHECK_SIG(drawable.gd, "\xf7\x36\x47\x38\xf5\x14\xdf\xb1\x2a\x5e\x5c\x3c\x0a\x8d\x7f\x7e\x5f\x7e\xf6\xb3");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.sender == nullptr);
    RED_CHECK(notifier2.sender == nullptr);
    RED_CHECK(notifier3.sender == &wbutton3);
    RED_CHECK(notifier4.sender == &wbutton4);
    RED_CHECK(notifier3.event == NOTIFY_FOCUS_BEGIN);
    RED_CHECK(notifier4.event == NOTIFY_FOCUS_END);
    notifier3.sender = nullptr;
    notifier4.sender = nullptr;
    notifier3.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen6.png");
    RED_CHECK_SIG(drawable.gd, "\x33\xfc\x94\x92\xf4\xa3\xed\x2d\xe1\x5e\x30\x56\x93\x59\xd6\x53\xb7\xad\x83\x91");


    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                            wbutton1.x(), wbutton1.y(), &keymap);
    RED_CHECK(notifier1.sender == &wbutton1);
    RED_CHECK(notifier2.sender == nullptr);
    RED_CHECK(notifier3.sender == &wbutton3);
    RED_CHECK(notifier4.sender == nullptr);
    RED_CHECK(notifier1.event == NOTIFY_FOCUS_BEGIN);
    RED_CHECK(notifier3.event == NOTIFY_FOCUS_END);
    notifier1.sender = nullptr;
    notifier3.sender = nullptr;
    notifier1.event = 0;
    notifier3.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen7.png");
    RED_CHECK_SIG(drawable.gd, "\x69\xec\xfa\xd2\x80\xd5\xc1\xf9\x32\xee\x2c\x73\x2c\x7f\xa5\x91\xd1\x6a\xfc\xb1");

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton2.x(), wbutton2.y(), &keymap);
    RED_CHECK(notifier1.sender == nullptr);
    RED_CHECK(notifier2.sender == nullptr);
    RED_CHECK(notifier3.sender == nullptr);
    RED_CHECK(notifier4.sender == nullptr);
    RED_CHECK(notifier1.event == 0);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen8.png");
    RED_CHECK_SIG(drawable.gd, "\x39\xd8\xaa\x23\xe3\xe9\xf7\x41\xee\x53\xa9\xae\xe3\xb2\xb1\x90\x8d\xda\x5a\xb2");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.sender == &wbutton1);
    RED_CHECK(notifier2.sender == &wbutton2);
    RED_CHECK(notifier3.sender == nullptr);
    RED_CHECK(notifier4.sender == nullptr);
    RED_CHECK(notifier1.event == NOTIFY_FOCUS_END);
    RED_CHECK(notifier2.event == NOTIFY_FOCUS_BEGIN);
    notifier1.sender = nullptr;
    notifier2.sender = nullptr;
    notifier1.event = 0;
    notifier2.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen9.png");
    RED_CHECK_SIG(drawable.gd, "\x59\x08\x30\x3d\x32\xec\x54\x34\x2e\x53\x69\x36\x33\x82\x61\x90\x9c\x81\xe6\x34");

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                            wbutton4.x(), wbutton4.y(), &keymap);
    RED_CHECK(notifier1.sender == nullptr);
    RED_CHECK(notifier2.sender == &wbutton2);
    RED_CHECK(notifier3.sender == nullptr);
    RED_CHECK(notifier4.sender == &wbutton4);
    RED_CHECK(notifier2.event == NOTIFY_FOCUS_END);
    RED_CHECK(notifier4.event == NOTIFY_FOCUS_BEGIN);
    notifier2.sender = nullptr;
    notifier4.sender = nullptr;
    notifier2.event = 0;
    notifier4.event = 0;
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen10.png");
    RED_CHECK_SIG(drawable.gd, "\x22\x61\x9e\x42\x44\x31\x03\x6d\xae\x4f\x79\x0e\x7f\xc1\xa7\x26\x57\x0e\xed\xf7");

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton4.x(), wbutton4.y(), &keymap);
    RED_CHECK(notifier1.sender == nullptr);
    RED_CHECK(notifier2.sender == nullptr);
    RED_CHECK(notifier3.sender == nullptr);
    RED_CHECK(notifier4.sender == &wbutton4);
    RED_CHECK(notifier4.event == NOTIFY_SUBMIT);
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen11.png");
    RED_CHECK_SIG(drawable.gd, "\xf7\x36\x47\x38\xf5\x14\xdf\xb1\x2a\x5e\x5c\x3c\x0a\x8d\x7f\x7e\x5f\x7e\xf6\xb3");

    wscreen.show_tooltip(nullptr, "tooltip test", 30, 35, Rect(0, 0, 0, 0));

    wscreen.rdp_input_invalidate(wscreen.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen12.png");
    RED_CHECK_SIG(drawable.gd, "\x10\x0e\x20\x6a\x1d\xdf\xbe\xab\x1b\x3f\x63\x14\xb7\xdf\x18\xc7\x73\x6f\xf1\x88");

    wscreen.show_tooltip(nullptr, nullptr, 30, 35, Rect(0, 0, 0, 0));
    wscreen.rdp_input_invalidate(wscreen.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "screen13.png");
    RED_CHECK_SIG(drawable.gd, "\xf7\x36\x47\x38\xf5\x14\xdf\xb1\x2a\x5e\x5c\x3c\x0a\x8d\x7f\x7e\x5f\x7e\xf6\xb3");
    wscreen.clear();
}
