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
#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/mod/internal/widget/notify_trace.hpp"
#include "test_only/mod/internal/widget/widget_receive_event.hpp"

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
                           wedit.x() + 2, wedit.y() + 2);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           wedit.x() + 2, wedit.y() + 2);

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
                           wedit1.x() + 50, wedit1.y() + 3);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           wedit1.x() + 50, wedit1.y() + 3);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_7.png");

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));

    auto rdp_input_scancode = [&](uint16_t scancode_and_flags){
        using KFlags = Keymap::KbdFlags;
        using Scancode = Keymap::Scancode;
        auto scancode = Scancode(scancode_and_flags);
        auto flags = KFlags(scancode_and_flags & 0xff00u);
        keymap.event(flags, scancode);
        parent.rdp_input_scancode(flags, scancode, 0, keymap);
        keymap.event(flags | KFlags::Release, scancode);
        parent.rdp_input_scancode(flags | KFlags::Release, scancode, 0, keymap);
    };

    rdp_input_scancode(0x0F); // tab
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_8.png");

    rdp_input_scancode(0x10); // 'a'
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
                           wedit1.x() + 50, wedit1.y() + 2);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           wedit1.x() + 50, wedit1.y() + 2);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_11.png");

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));

    auto rdp_input_scancode = [&](uint16_t scancode_and_flags){
        using KFlags = Keymap::KbdFlags;
        using Scancode = Keymap::Scancode;
        auto scancode = Scancode(scancode_and_flags);
        auto flags = KFlags(scancode_and_flags & 0xff00u);
        keymap.event(flags, scancode);
        parent.rdp_input_scancode(flags, scancode, 0, keymap);
        keymap.event(flags | KFlags::Release, scancode);
        parent.rdp_input_scancode(flags | KFlags::Release, scancode, 0, keymap);
    };

    rdp_input_scancode(0x0F); // tab
    parent.rdp_input_invalidate(parent.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_12.png");

    rdp_input_scancode(0x10); // 'a'
    parent.rdp_input_invalidate(parent.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_9.png");

    rdp_input_scancode(0x0F); // tab
    parent.rdp_input_invalidate(parent.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_14.png");

    rdp_input_scancode(0x0F); // tab
    rdp_input_scancode(0x0E); // backspace
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_15.png");

    rdp_input_scancode(0x0F); // tab
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
                           wedit1.x() + 50, wedit1.y() + 2);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           wedit1.x() + 50, wedit1.y() + 2);

    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_18.png");

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));

    auto rdp_input_scancode = [&](uint16_t scancode_and_flags){
        using KFlags = Keymap::KbdFlags;
        using Scancode = Keymap::Scancode;
        auto scancode = Scancode(scancode_and_flags);
        auto flags = KFlags(scancode_and_flags & 0xff00u);
        keymap.event(flags, scancode);
        parent.rdp_input_scancode(flags, scancode, 0, keymap);
        keymap.event(flags | KFlags::Release, scancode);
        parent.rdp_input_scancode(flags | KFlags::Release, scancode, 0, keymap);
    };

    rdp_input_scancode(0x0F); // tab
    parent.rdp_input_invalidate(parent.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_19.png");

    rdp_input_scancode(0x10); // 'a'
    parent.rdp_input_invalidate(parent.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_20.png");

    rdp_input_scancode(0x0F); // tab
    parent.rdp_input_invalidate(parent.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_21.png");

    rdp_input_scancode(0x0F); // tab
    rdp_input_scancode(0x0E); // backspace
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_22.png");

    rdp_input_scancode(0x0F); // tab
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_18.png");
}

RED_AUTO_TEST_CASE(EventWidgetEditEvents)
{
    TestGraphic drawable(800, 600);

    WidgetReceiveEvent widget_for_receive_event(drawable);

    NotifyTrace notifier;

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
                           wedit.eright() - 5, wedit.y() + 2);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_24.png");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           wedit.eright() - 5, wedit.y() + 2);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(notifier.last_widget == &wedit);
    RED_CHECK(notifier.last_event == NOTIFY_SUBMIT);
    notifier.last_event = 0;
    notifier.last_widget = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                           wedit.eright() - 5, wedit.y() + 2);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_24.png");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           wedit.x() + 2, wedit.y() + 2);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);
    notifier.last_event = 0;
    notifier.last_widget = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                           wedit.eright() - 5, wedit.y() + 2);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_24.png");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 0, 0);
    parent.rdp_input_invalidate(parent.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);
    notifier.last_event = 0;
    notifier.last_widget = nullptr;

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));

    auto rdp_input_scancode = [&](uint16_t scancode_and_flags){
        using KFlags = Keymap::KbdFlags;
        using Scancode = Keymap::Scancode;
        auto scancode = Scancode(scancode_and_flags);
        auto flags = KFlags(scancode_and_flags & 0xff00u);
        keymap.event(flags, scancode);
        parent.rdp_input_scancode(flags, scancode, 0, keymap);
        keymap.event(flags | KFlags::Release, scancode);
        parent.rdp_input_scancode(flags | KFlags::Release, scancode, 0, keymap);
    };

    rdp_input_scancode(0x1C); // enter
    parent.rdp_input_invalidate(parent.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(notifier.last_widget == &wedit);
    RED_CHECK(notifier.last_event == NOTIFY_SUBMIT);
    notifier.last_event = 0;
    notifier.last_widget = nullptr;
}
