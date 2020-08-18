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

#include "mod/internal/widget/edit.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/edit/"


RED_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;
    int xtext = 4;
    int ytext = 1;
    size_t edit_pos = 2;

    WidgetEdit wedit(drawable, parent, notifier, "test1", id,
                     fg_color, bg_color, bg_color, global_font_deja_vu_14(), edit_pos, xtext, ytext);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.x(),
                                    wedit.cx(),
                                    wedit.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test2", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test3", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit4)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test4", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit5)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test5", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit6)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEditClip)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wedit.rdp_input_invalidate(Rect(20 + wedit.x(),
                                    wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_8.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEditClip2)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, parent, notifier, "test6", id, fg_color, bg_color, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wedit.rdp_input_invalidate(Rect(20 + wedit.x(),
                                    5 + wedit.y(),
                                    30,
                                    10));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_9.png");
}

RED_AUTO_TEST_CASE(EventWidgetEdit)
{
    TestGraphic drawable(800, 600);

    struct WidgetReceiveEvent : public Widget {
        Widget* sender = nullptr;
        NotifyApi::notify_event_t event = 0;

        WidgetReceiveEvent(TestGraphic& drawable)
        : Widget(drawable, *this, nullptr)
        {}

        void rdp_input_invalidate(Rect /*r*/) override
        {}

        void notify(Widget* sender, NotifyApi::notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event(drawable);

    struct Notify : public NotifyApi {
        Widget* sender = nullptr;
        notify_event_t event = 0;
        Notify() = default;
        void notify(Widget* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;


    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEdit wedit(drawable, parent, &notifier, "abcdef", 0, GREEN, RED, RED, global_font_deja_vu_14());
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    wedit.focus(Widget::focus_reason_tabkey);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_10.png");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    bool ctrl_alt_delete;

    keymap.event(0, 16, ctrl_alt_delete); // 'a'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_11.png");
    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.event(0, 17, ctrl_alt_delete); // 'z'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_12.png");
    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_13.png");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_12.png");

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_11.png");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_16.png");

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_17.png");

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_18.png");

    keymap.push_kevent(Keymap2::KEVENT_END);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_19.png");

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_20.png");

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

    wedit.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x+10, y+3, nullptr);
    RED_CHECK(widget_for_receive_event.sender == nullptr);
    RED_CHECK(widget_for_receive_event.event == 0);
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_21.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEditAndComposite)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetEdit wedit1(drawable, wcomposite, notifier,
                      "abababab", 4, YELLOW, BLACK, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(50, dim.h);
    wedit1.set_xy(0, 0);

    WidgetEdit wedit2(drawable, wcomposite, notifier,
                      "ggghdgh", 2, WHITE, RED, RED, global_font_deja_vu_14());
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(50, dim.h);
    wedit2.set_xy(0, 100);

    WidgetEdit wedit3(drawable, wcomposite, notifier,
                      "lldlslql", 1, BLUE, RED, RED, global_font_deja_vu_14());
    dim = wedit3.get_optimal_dim();
    wedit3.set_wh(50, dim.h);
    wedit3.set_xy(100, 100);

    WidgetEdit wedit4(drawable, wcomposite, notifier,
                      "LLLLMLLM", 20, PINK, DARK_GREEN, DARK_GREEN, global_font_deja_vu_14());
    dim = wedit4.get_optimal_dim();
    wedit4.set_wh(50, dim.h);
    wedit4.set_xy(300, 300);

    WidgetEdit wedit5(drawable, wcomposite, notifier,
                      "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, DARK_BLUE, global_font_deja_vu_14());
    dim = wedit5.get_optimal_dim();
    wedit5.set_wh(50, dim.h);
    wedit5.set_xy(700, -10);

    WidgetEdit wedit6(drawable, wcomposite, notifier,
                      "xxwwp", 2, ANTHRACITE, PALE_GREEN, PALE_GREEN, global_font_deja_vu_14());
    dim = wedit6.get_optimal_dim();
    wedit6.set_wh(50, dim.h);
    wedit6.set_xy(-10, 550);

    wcomposite.add_widget(&wedit1);
    wcomposite.add_widget(&wedit2);
    wcomposite.add_widget(&wedit3);
    wcomposite.add_widget(&wedit4);
    wcomposite.add_widget(&wedit5);
    wcomposite.add_widget(&wedit6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_22.png");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_23.png");

    wcomposite.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetEditScrolling)
{
    TestGraphic drawable(800, 600);


    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEdit wedit(drawable, parent, &parent, "abcde", 0, BLACK, WHITE, WHITE, global_font_deja_vu_14(),
                     -1u, 1, 1);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    wedit.focus(Widget::focus_reason_tabkey);
    parent.add_widget(&wedit);
    parent.current_focus = &wedit;

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    bool    ctrl_alt_delete;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_24.png");

    keymap.event(keyboardFlags, keyCode + 1, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode + 2, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_25.png");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_26.png");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_27.png");

    keymap.event(keyboardFlags, keyCode + 9, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_28.png");

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_29.png");

    keymap.push_kevent(Keymap2::KEVENT_END);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_28.png");

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_31.png");

    for (int i = 0; i < 10; i++) {
        keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
        parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    }
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_32.png");

    parent.clear();
}
