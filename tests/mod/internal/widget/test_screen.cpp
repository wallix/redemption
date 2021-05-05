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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/widget/flat_button.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

#include "test_only/mod/internal/widget/notify_trace.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/screen/"


RED_AUTO_TEST_CASE(TestScreenEvent)
{
    TestGraphic drawable(800, 600);
    Theme colors;

    WidgetScreen wscreen(drawable, drawable.width(), drawable.height(), global_font_lato_light_16(), nullptr, Theme{});

    wscreen.rdp_input_invalidate(wscreen.get_rect());
    wscreen.tab_flag = Widget::NORMAL_TAB;
    NotifyTrace notifier1;
    NotifyTrace notifier2;
    NotifyTrace notifier3;
    NotifyTrace notifier4;

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

    RED_CHECK(notifier1.last_widget == nullptr);
    RED_CHECK(notifier2.last_widget == &wbutton2);
    RED_CHECK(notifier3.last_widget == nullptr);
    RED_CHECK(notifier4.last_widget == nullptr);
    RED_CHECK(notifier2.last_event == NOTIFY_FOCUS_BEGIN);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_1.png");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.last_widget == nullptr);
    RED_CHECK(notifier2.last_widget == &wbutton2);
    RED_CHECK(notifier3.last_widget == &wbutton3);
    RED_CHECK(notifier4.last_widget == nullptr);
    RED_CHECK(notifier2.last_event == NOTIFY_FOCUS_END);
    RED_CHECK(notifier3.last_event == NOTIFY_FOCUS_BEGIN);
    notifier2.last_widget = nullptr;
    notifier3.last_widget = nullptr;
    notifier2.last_event = 0;
    notifier3.last_event = 0;
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_2.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.last_widget == nullptr);
    RED_CHECK(notifier2.last_widget == nullptr);
    RED_CHECK(notifier3.last_widget == &wbutton3);
    RED_CHECK(notifier4.last_widget == &wbutton4);
    RED_CHECK(notifier3.last_event == NOTIFY_FOCUS_END);
    RED_CHECK(notifier4.last_event == NOTIFY_FOCUS_BEGIN);
    notifier3.last_widget = nullptr;
    notifier4.last_widget = nullptr;
    notifier3.last_event = 0;
    notifier4.last_event = 0;
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_3.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.last_widget == &wbutton1);
    RED_CHECK(notifier2.last_widget == nullptr);
    RED_CHECK(notifier3.last_widget == nullptr);
    RED_CHECK(notifier4.last_widget == &wbutton4);
    RED_CHECK(notifier1.last_event == NOTIFY_FOCUS_BEGIN);
    RED_CHECK(notifier4.last_event == NOTIFY_FOCUS_END);
    notifier1.last_widget = nullptr;
    notifier4.last_widget = nullptr;
    notifier1.last_event = 0;
    notifier4.last_event = 0;
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_4.png");

    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.last_widget == &wbutton1);
    RED_CHECK(notifier2.last_widget == nullptr);
    RED_CHECK(notifier3.last_widget == nullptr);
    RED_CHECK(notifier4.last_widget == &wbutton4);
    RED_CHECK(notifier1.last_event == NOTIFY_FOCUS_END);
    RED_CHECK(notifier4.last_event == NOTIFY_FOCUS_BEGIN);
    notifier1.last_widget = nullptr;
    notifier4.last_widget = nullptr;
    notifier1.last_event = 0;
    notifier4.last_event = 0;
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_3.png");

    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.last_widget == nullptr);
    RED_CHECK(notifier2.last_widget == nullptr);
    RED_CHECK(notifier3.last_widget == &wbutton3);
    RED_CHECK(notifier4.last_widget == &wbutton4);
    RED_CHECK(notifier3.last_event == NOTIFY_FOCUS_BEGIN);
    RED_CHECK(notifier4.last_event == NOTIFY_FOCUS_END);
    notifier3.last_widget = nullptr;
    notifier4.last_widget = nullptr;
    notifier3.last_event = 0;
    notifier4.last_event = 0;
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_2.png");

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                            wbutton1.x(), wbutton1.y(), &keymap);
    RED_CHECK(notifier1.last_widget == &wbutton1);
    RED_CHECK(notifier2.last_widget == nullptr);
    RED_CHECK(notifier3.last_widget == &wbutton3);
    RED_CHECK(notifier4.last_widget == nullptr);
    RED_CHECK(notifier1.last_event == NOTIFY_FOCUS_BEGIN);
    RED_CHECK(notifier3.last_event == NOTIFY_FOCUS_END);
    notifier1.last_widget = nullptr;
    notifier3.last_widget = nullptr;
    notifier1.last_event = 0;
    notifier3.last_event = 0;
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_7.png");

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton2.x(), wbutton2.y(), &keymap);
    RED_CHECK(notifier1.last_widget == nullptr);
    RED_CHECK(notifier2.last_widget == nullptr);
    RED_CHECK(notifier3.last_widget == nullptr);
    RED_CHECK(notifier4.last_widget == nullptr);
    RED_CHECK(notifier1.last_event == 0);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_4.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    RED_CHECK(notifier1.last_widget == &wbutton1);
    RED_CHECK(notifier2.last_widget == &wbutton2);
    RED_CHECK(notifier3.last_widget == nullptr);
    RED_CHECK(notifier4.last_widget == nullptr);
    RED_CHECK(notifier1.last_event == NOTIFY_FOCUS_END);
    RED_CHECK(notifier2.last_event == NOTIFY_FOCUS_BEGIN);
    notifier1.last_widget = nullptr;
    notifier2.last_widget = nullptr;
    notifier1.last_event = 0;
    notifier2.last_event = 0;
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_1.png");

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                            wbutton4.x(), wbutton4.y(), &keymap);
    RED_CHECK(notifier1.last_widget == nullptr);
    RED_CHECK(notifier2.last_widget == &wbutton2);
    RED_CHECK(notifier3.last_widget == nullptr);
    RED_CHECK(notifier4.last_widget == &wbutton4);
    RED_CHECK(notifier2.last_event == NOTIFY_FOCUS_END);
    RED_CHECK(notifier4.last_event == NOTIFY_FOCUS_BEGIN);
    notifier2.last_widget = nullptr;
    notifier4.last_widget = nullptr;
    notifier2.last_event = 0;
    notifier4.last_event = 0;
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_10.png");

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton4.x(), wbutton4.y(), &keymap);
    RED_CHECK(notifier1.last_widget == nullptr);
    RED_CHECK(notifier2.last_widget == nullptr);
    RED_CHECK(notifier3.last_widget == nullptr);
    RED_CHECK(notifier4.last_widget == &wbutton4);
    RED_CHECK(notifier4.last_event == NOTIFY_SUBMIT);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_3.png");

    wscreen.show_tooltip(nullptr, "tooltip test", 30, 35, Rect(0, 0, 500, 41));

    wscreen.rdp_input_invalidate(wscreen.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_12.png");

    wscreen.show_tooltip(nullptr, nullptr, 30, 35, Rect(0, 0, 0, 0));
    wscreen.rdp_input_invalidate(wscreen.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "screen_3.png");
    wscreen.clear();
}
