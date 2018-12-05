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
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/flat_button.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

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
    TestGraphic drawable(800, 600);
    Theme colors;

    WidgetScreen wscreen(drawable, global_font_lato_light_16(), nullptr, Theme{});
    wscreen.set_wh(drawable.width(), drawable.height());

    wscreen.rdp_input_invalidate(wscreen.get_rect());
    wscreen.tab_flag = Widget::NORMAL_TAB;
    Notify notifier1;
    Notify notifier2;
    Notify notifier3;
    Notify notifier4;

    WidgetFlatButton wbutton1(drawable, wscreen, &notifier1, "button 1",
                              0, WHITE, DARK_BLUE_BIS, WINBLUE, 2, global_font_lato_light_16());
    Dimension dim = wbutton1.get_optimal_dim();
    wbutton1.set_wh(dim);
    wbutton1.set_xy(0, 0);

    WidgetFlatButton wbutton2(drawable, wscreen, &notifier2, "button 2",
                              0, WHITE, DARK_BLUE_BIS, WINBLUE, 2, global_font_lato_light_16());
    dim = wbutton2.get_optimal_dim();
    wbutton2.set_wh(dim);
    wbutton2.set_xy(0, 30);

    WidgetFlatButton wbutton3(drawable, wscreen, &notifier3, "button 3",
                              0, WHITE, DARK_BLUE_BIS, WINBLUE, 2, global_font_lato_light_16());
    dim = wbutton3.get_optimal_dim();
    wbutton3.set_wh(dim);
    wbutton3.set_xy(100, 0);

    WidgetFlatButton wbutton4(drawable, wscreen, &notifier4, "button 4",
                              0, WHITE, DARK_BLUE_BIS, WINBLUE, 2, global_font_lato_light_16());
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
    // drawable.save_to_png("screen.png");
    RED_CHECK_SIG(drawable, "\xe1\xf9\x82\xc6\x5b\x9c\xa5\x91\xc9\x36\x5f\x40\x81\x37\xe3\xef\xad\xad\xe4\x41");


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
    // drawable.save_to_png("screen2.png");
    RED_CHECK_SIG(drawable, "\x53\xcf\x97\x98\xed\x5d\x77\x87\x76\xf0\xd4\x30\x47\x3a\x77\xbf\xdc\x41\xa9\xd8");


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
    // drawable.save_to_png("screen3.png");
    RED_CHECK_SIG(drawable, "\x53\x3e\x1c\x66\xf5\x96\xfe\x74\x3d\x7e\x22\xf9\xcf\x4e\xb4\x04\x24\x25\x34\xb4");


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
    // drawable.save_to_png("screen4.png");
    RED_CHECK_SIG(drawable, "\xb3\x2b\xb4\xc9\x9d\x0c\xc5\xb8\x01\x05\x96\xf6\x5d\x25\xaa\xbe\xb8\xe0\x05\x4a");


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
    // drawable.save_to_png("screen5.png");
    RED_CHECK_SIG(drawable, "\x53\x3e\x1c\x66\xf5\x96\xfe\x74\x3d\x7e\x22\xf9\xcf\x4e\xb4\x04\x24\x25\x34\xb4");


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
    // drawable.save_to_png("screen6.png");
    RED_CHECK_SIG(drawable, "\x53\xcf\x97\x98\xed\x5d\x77\x87\x76\xf0\xd4\x30\x47\x3a\x77\xbf\xdc\x41\xa9\xd8");


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
    // drawable.save_to_png("screen7.png");
    RED_CHECK_SIG(drawable, "\x5b\xf6\x87\x47\x13\xb8\x5d\x26\xa1\xe9\xd5\xaa\x36\x30\xec\xac\x75\x75\xa8\xd7");

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton2.x(), wbutton2.y(), &keymap);
    RED_CHECK(notifier1.sender == nullptr);
    RED_CHECK(notifier2.sender == nullptr);
    RED_CHECK(notifier3.sender == nullptr);
    RED_CHECK(notifier4.sender == nullptr);
    RED_CHECK(notifier1.event == 0);
    // drawable.save_to_png("screen8.png");
    RED_CHECK_SIG(drawable, "\xb3\x2b\xb4\xc9\x9d\x0c\xc5\xb8\x01\x05\x96\xf6\x5d\x25\xaa\xbe\xb8\xe0\x05\x4a");

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
    // drawable.save_to_png("screen9.png");
    RED_CHECK_SIG(drawable, "\xe1\xf9\x82\xc6\x5b\x9c\xa5\x91\xc9\x36\x5f\x40\x81\x37\xe3\xef\xad\xad\xe4\x41");

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
    // drawable.save_to_png("screen10.png");
    RED_CHECK_SIG(drawable, "\xb2\x29\x3f\x10\x1b\xe0\xb6\x0b\x08\xae\xd0\x6b\xbb\x58\x13\x8b\xa7\xd3\xe6\x97");

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton4.x(), wbutton4.y(), &keymap);
    RED_CHECK(notifier1.sender == nullptr);
    RED_CHECK(notifier2.sender == nullptr);
    RED_CHECK(notifier3.sender == nullptr);
    RED_CHECK(notifier4.sender == &wbutton4);
    RED_CHECK(notifier4.event == NOTIFY_SUBMIT);
    // drawable.save_to_png("screen11.png");
    RED_CHECK_SIG(drawable, "\x53\x3e\x1c\x66\xf5\x96\xfe\x74\x3d\x7e\x22\xf9\xcf\x4e\xb4\x04\x24\x25\x34\xb4");

    wscreen.show_tooltip(nullptr, "tooltip test", 30, 35, Rect(0, 0, 0, 0));

    wscreen.rdp_input_invalidate(wscreen.get_rect());
    // drawable.save_to_png("screen12.png");
    RED_CHECK_SIG(drawable, "\xd4\x05\xe4\xa8\x68\x4a\xac\xe6\xab\xb0\x9a\x95\x56\x96\xb5\xc1\x5e\x8c\xc5\x05");

    wscreen.show_tooltip(nullptr, nullptr, 30, 35, Rect(0, 0, 0, 0));
    wscreen.rdp_input_invalidate(wscreen.get_rect());
    // drawable.save_to_png("screen13.png");
    RED_CHECK_SIG(drawable, "\x53\x3e\x1c\x66\xf5\x96\xfe\x74\x3d\x7e\x22\xf9\xcf\x4e\xb4\x04\x24\x25\x34\xb4");
    wscreen.clear();
}
