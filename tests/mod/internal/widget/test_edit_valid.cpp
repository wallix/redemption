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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/widget/edit_valid.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

#include <string_view>

using namespace std::string_view_literals;


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/edit_valid/"

RED_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestGraphic drawable(800, 600);

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLACK;
    BGRColor bg_color = WHITE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;
    int xtext = 4;
    int ytext = 1;
    size_t edit_pos = 2;

    WidgetEditValid wedit(drawable, parent, notifier, "test1", id,
                          fg_color, bg_color, ANTHRACITE, bg_color, global_font_deja_vu_14(), nullptr, false, edit_pos, xtext, ytext);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    parent.set_widget_focus(&wedit, Widget::focus_reason_tabkey);
    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_1.png");
}


RED_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLACK;
    BGRColor bg_color = WHITE;
    int id = 0;
    int16_t x = 50;
    int16_t y = 100;
    uint16_t cx = 100;
    int xtext = 4;
    int ytext = 1;
    size_t edit_pos = -1;

    WidgetEditValid wedit(drawable, parent, notifier, nullptr, id,
                          fg_color, bg_color, ANTHRACITE, bg_color, global_font_deja_vu_14(), nullptr, false, edit_pos, xtext, ytext);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    parent.add_widget(&wedit);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_2.png");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.x() + 2, wedit.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.x() + 2, wedit.y() + 2, nullptr);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_3.png");

    wedit.set_text("Ylajali");

    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_4.png");

    RED_CHECK_EQUAL("Ylajali"sv, wedit.get_text());


    wedit.set_xy(192,242);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_5.png");

    parent.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestGraphic drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, bg_color, global_font_deja_vu_14(), nullptr, false, 0);
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(150, dim.h);
    wedit1.set_xy(54, 105);

    WidgetEditValid wedit2(drawable, parent, notifier, "", id, WHITE, DARK_BLUE, RED, DARK_BLUE, global_font_deja_vu_14(), nullptr, false, 0);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_6.png");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 3, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 3, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_7.png");

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_8.png");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_9.png");
}


RED_AUTO_TEST_CASE(TraceWidgetEditLabels)
{
    TestGraphic drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, bg_color, global_font_deja_vu_14(), "edition1", true, 0, 0, 0, false);
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(150, dim.h);
    wedit1.set_xy(54, 105);

    WidgetEditValid wedit2(drawable, parent, notifier, "", id, WHITE, DARK_BLUE, RED, DARK_BLUE, global_font_deja_vu_14(), "edition2", true, 0, 0, 0, false);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_10.png");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_11.png");

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_12.png");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_9.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_14.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_15.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_11.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEditLabelsPassword)
{
    TestGraphic drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, bg_color, global_font_deja_vu_14(), "edition1", true, 0, 0, 0, true);
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(150, dim.h);
    wedit1.set_xy(54, 105);

    WidgetEditValid wedit2(drawable, parent, notifier, "", id, WHITE, DARK_BLUE, RED, DARK_BLUE, global_font_deja_vu_14(), "edition2", true, 0, 0, 0, true);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_17.png");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_18.png");

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_19.png");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_20.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_21.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_22.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_18.png");
}

RED_AUTO_TEST_CASE(EventWidgetEditEvents)
{
    TestGraphic drawable(800, 600);

    struct WidgetReceiveEvent : public Widget {
        Widget* sender = nullptr;
        NotifyApi::notify_event_t event = 0;

        WidgetReceiveEvent(TestGraphic& drawable)
        : Widget(drawable, *this, nullptr)
        {}

        void rdp_input_invalidate(const Rect /*r*/) override
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

    WidgetEditValid wedit(drawable, parent, &notifier, "abcdef", 0, BLACK, WHITE, DARK_BLUE, WHITE, global_font_deja_vu_14(), nullptr, false);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    parent.add_widget(&wedit);
    parent.set_widget_focus(&wedit, Widget::focus_reason_tabkey);

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.eright() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_24.png");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.eright() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.eright() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_24.png");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.x() + 2, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.eright() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_24.png");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          0, 0, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = nullptr;


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = nullptr;
}
