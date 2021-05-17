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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/widget/password.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/mod/internal/widget/notify_trace.hpp"
#include "test_only/mod/internal/widget/widget_receive_event.hpp"

#include <string_view>

using namespace std::string_view_literals;


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/password/"


RED_AUTO_TEST_CASE(TraceWidgetPassword)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;
    int xtext = 4;
    int ytext = 1;
    size_t password_pos = 2;

    WidgetPassword wpassword(drawable, parent, notifier, "test1", id,
                     fg_color, bg_color, BLACK, global_font_lato_light_16(), password_pos, xtext, ytext);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.x(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword2)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test2", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword3)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test3", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword4)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test4", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword5)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test5", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword6)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPasswordClip)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPasswordClip2)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLUE;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_lato_light_16(), 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.x(),
                                    5 + wpassword.y(),
                                    30,
                                    10));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_8.png");
}

RED_AUTO_TEST_CASE(EventWidgetPassword)
{
    TestGraphic drawable(800, 600);

    NotifyTrace notifier;

    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    // Widget* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, parent, &notifier, "abcdef", 0, YELLOW, RED, RED, global_font_lato_light_16());
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_9.png");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    bool    ctrl_alt_delete;

    keymap.event(0, 16, ctrl_alt_delete); // 'a'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_10.png");
    RED_CHECK(notifier.last_widget == &wpassword);
    RED_CHECK(notifier.last_event == NOTIFY_TEXT_CHANGED);
    notifier.last_event = 0;
    notifier.last_widget = nullptr;

    keymap.event(0, 17, ctrl_alt_delete); // 'z'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_11.png");
    RED_CHECK(notifier.last_widget == &wpassword);
    RED_CHECK(notifier.last_event == NOTIFY_TEXT_CHANGED);
    notifier.last_event = 0;
    notifier.last_widget = nullptr;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_12.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_11.png");

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_10.png");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_15.png");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_16.png");

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_17.png");

    keymap.push_kevent(Keymap2::KEVENT_END);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_9.png");

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_19.png");

    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.last_widget == &wpassword);
    RED_CHECK(notifier.last_event == NOTIFY_SUBMIT);
    notifier.last_widget = nullptr;
    notifier.last_event = 0;

    WidgetReceiveEvent widget_for_receive_event(drawable);

    wpassword.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 10, 3, nullptr);
    RED_CHECK(widget_for_receive_event.last_widget == nullptr);
    RED_CHECK(widget_for_receive_event.last_event == 0);
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);
    notifier.last_widget = nullptr;
    notifier.last_event = 0;
    widget_for_receive_event.last_widget = nullptr;
    widget_for_receive_event.last_event = 0;

    wpassword.rdp_input_invalidate(Rect(0, 0, wpassword.cx(), wpassword.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_20.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPasswordAndComposite)
{
    TestGraphic drawable(800, 600);

    // WidgetPassword is a password widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetPassword wpassword1(drawable, wcomposite, notifier,
                              "abababab", 4, YELLOW, BLACK, BLACK, global_font_lato_light_16());
    Dimension dim = wpassword1.get_optimal_dim();
    wpassword1.set_wh(50, dim.h);
    wpassword1.set_xy(0, 0);

    WidgetPassword wpassword2(drawable, wcomposite, notifier,
                              "ggghdgh", 2, WHITE, RED, RED, global_font_lato_light_16());
    dim = wpassword2.get_optimal_dim();
    wpassword2.set_wh(50, dim.h);
    wpassword2.set_xy(0, 100);

    WidgetPassword wpassword3(drawable, wcomposite, notifier,
                              "lldlslql", 1, BLUE, RED, RED, global_font_lato_light_16());
    dim = wpassword3.get_optimal_dim();
    wpassword3.set_wh(50, dim.h);
    wpassword3.set_xy(100, 100);

    WidgetPassword wpassword4(drawable, wcomposite, notifier,
                              "LLLLMLLM", 20, PINK, DARK_GREEN, DARK_GREEN, global_font_lato_light_16());
    dim = wpassword4.get_optimal_dim();
    wpassword4.set_wh(50, dim.h);
    wpassword4.set_xy(300, 300);

    WidgetPassword wpassword5(drawable, wcomposite, notifier,
                              "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, DARK_BLUE, global_font_lato_light_16());
    dim = wpassword5.get_optimal_dim();
    wpassword5.set_wh(50, dim.h);
    wpassword5.set_xy(700, -10);

    WidgetPassword wpassword6(drawable, wcomposite, notifier,
                              "xxwwp", 2, ANTHRACITE, PALE_GREEN, PALE_GREEN, global_font_lato_light_16());
    dim = wpassword6.get_optimal_dim();
    wpassword6.set_wh(50, dim.h);
    wpassword6.set_xy(-10, 550);

    wcomposite.add_widget(&wpassword1);
    wcomposite.add_widget(&wpassword2);
    wcomposite.add_widget(&wpassword3);
    wcomposite.add_widget(&wpassword4);
    wcomposite.add_widget(&wpassword5);
    wcomposite.add_widget(&wpassword6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_21.png");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png("password-compo2.png");

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_22.png");
    wcomposite.clear();
}

RED_AUTO_TEST_CASE(DataWidgetPassword)
{
    TestGraphic drawable(800, 600);

    NotifyTrace notifier;

    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    // Widget* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, parent, &notifier, "aurélie", 0, YELLOW, RED, RED, global_font_lato_light_16());
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_10.png");
    RED_CHECK(notifier.last_widget == &wpassword);
    RED_CHECK(notifier.last_event == 0);
    notifier.last_widget = nullptr;
    notifier.last_event = 0;

    RED_CHECK("aurélie"sv == wpassword.get_text());

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_15.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_16.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_26.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);


    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_27.png");

    RED_CHECK("aurlie"sv == wpassword.get_text());
}

RED_AUTO_TEST_CASE(DataWidgetPassword2)
{
    TestGraphic drawable(800, 600);

    NotifyTrace notifier;

    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    // Widget* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, parent, &notifier, "aurélie", 0, YELLOW, RED, RED, global_font_lato_light_16());
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_10.png");
    RED_CHECK(notifier.last_widget == &wpassword);
    RED_CHECK(notifier.last_event == 0);
    notifier.last_widget = nullptr;
    notifier.last_event = 0;

    RED_CHECK("aurélie"sv == wpassword.get_text());

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_15.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_16.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_26.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_32.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);


    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_27.png");

    RED_CHECK("aurlie"sv == wpassword.get_text());
}

RED_AUTO_TEST_CASE(DataWidgetPassword3)
{
    TestGraphic drawable(800, 600);

    NotifyTrace notifier;

    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    // Widget* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, parent, &notifier, "aurélie", 0, YELLOW, RED, RED, global_font_lato_light_16());
    parent.add_widget(&wpassword);

    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);


    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_10.png");
    RED_CHECK(notifier.last_widget == &wpassword);
    RED_CHECK(notifier.last_event == 0);
    notifier.last_widget = nullptr;
    notifier.last_event = 0;

    RED_CHECK("aurélie"sv == wpassword.get_text());

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_15.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_16.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_26.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_32.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);


    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_26.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);


    bool    ctrl_alt_delete;

    keymap.event(0, 17, ctrl_alt_delete); // 'z'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_40.png");
    RED_CHECK(notifier.last_widget == &wpassword);
    RED_CHECK(notifier.last_event == NOTIFY_TEXT_CHANGED);
    notifier.last_widget = nullptr;
    notifier.last_event = 0;

    RED_CHECK("aurézlie"sv == wpassword.get_text());

    // cursor overflow

    for (int i = 0; i < 10; i++) {
        keymap.event(0, 17, ctrl_alt_delete); // 'z'
        wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    }
    wpassword.rdp_input_invalidate(wpassword.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_41.png");

}
