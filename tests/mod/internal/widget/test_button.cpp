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

#include "mod/internal/widget/button.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/mod/internal/widget/notify_trace.hpp"
#include "test_only/mod/internal/widget/widget_receive_event.hpp"


namespace
{
struct ButtonContextTest
{
    TestGraphic drawable;
    WidgetScreen parent;
    WidgetButton wbutton;

    ButtonContextTest(
        char const* text,
        int16_t x, int16_t y,
        int xtext = 0, int ytext = 0,
        int16_t dx = 0, int16_t dy = 0,
        uint16_t cx = 0, uint16_t cy = 0,
        unsigned border_width = 2
    )
    : ButtonContextTest(
        800, 600, text, x, y, xtext, ytext, dx, dy, cx, cy, border_width)
    {}

    ButtonContextTest(
        int16_t w, int16_t h,
        char const* text,
        int16_t x, int16_t y,
        int xtext = 0, int ytext = 0,
        int16_t dx = 0, int16_t dy = 0,
        uint16_t cx = 0, uint16_t cy = 0,
        unsigned border_width = 2
    )
    : drawable(w, h)
    , parent{drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{}}
    , wbutton{drawable, parent, /*notifier=*/nullptr, text, /*id=*/0,
              /*fg_color=*/RED, /*bg_color=*/YELLOW, /*fc_color=*/WINBLUE,
              border_width, global_font_deja_vu_14(), xtext, ytext}
    {
        Dimension dim = wbutton.get_optimal_dim();
        wbutton.set_wh(dim);
        wbutton.set_xy(x, y);

        // ask to widget to redraw at it's current position
        wbutton.rdp_input_invalidate(Rect(
            dx + wbutton.x(),
            dy + wbutton.y(),
            cx ? cx : wbutton.cx(),
            cy ? cy : wbutton.cy()
        ));
    }
};
}

#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/button/"

RED_AUTO_TEST_CASE(TraceWidgetButton)
{
    RED_CHECK_IMG(ButtonContextTest("test1", 0, 0, 4, 1).drawable, IMG_TEST_PATH "button_1.png");
    RED_CHECK_IMG(ButtonContextTest("test2", 10, 100).drawable, IMG_TEST_PATH "button_2.png");
    RED_CHECK_IMG(ButtonContextTest("test3", -10, 500).drawable, IMG_TEST_PATH "button_3.png");
    RED_CHECK_IMG(ButtonContextTest("test4", 770, 500).drawable, IMG_TEST_PATH "button_4.png");
    RED_CHECK_IMG(ButtonContextTest("test5", -20, -7).drawable, IMG_TEST_PATH "button_5.png");
    RED_CHECK_IMG(ButtonContextTest("test6", 760, -7).drawable, IMG_TEST_PATH "button_6.png");
    RED_CHECK_IMG(ButtonContextTest("test6", 760, -7, 0, 0, 20).drawable, IMG_TEST_PATH "button_7.png");
    RED_CHECK_IMG(ButtonContextTest("test6", 0, 0, 0, 0, 20, 5, 30, 10).drawable, IMG_TEST_PATH "button_8.png");
}

RED_AUTO_TEST_CASE(TraceWidgetButtonDownAndUp)
{
    ButtonContextTest button("test6", 10, 10, 4, 1);
    RED_CHECK_IMG(button.drawable, IMG_TEST_PATH "button_9.png");

    auto& wbutton = button.wbutton;

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15);
    wbutton.rdp_input_invalidate(wbutton.get_rect());
    RED_CHECK_IMG(button.drawable, IMG_TEST_PATH "button_10.png");

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 15, 15);
    wbutton.rdp_input_invalidate(wbutton.get_rect());
    RED_CHECK_IMG(button.drawable, IMG_TEST_PATH "button_9.png");
}

RED_AUTO_TEST_CASE(TraceWidgetButtonEvent)
{
    TestGraphic drawable(800, 600);

    WidgetReceiveEvent widget_for_receive_event(drawable);

    NotifyTrace notifier;

    Widget& parent = widget_for_receive_event;
    int16_t x = 0;
    int16_t y = 0;


    WidgetButton wbutton(drawable, parent, &notifier, "", 0, WHITE,
                         DARK_BLUE_BIS, WINBLUE, 2, global_font_deja_vu_14());
    Dimension dim = wbutton.get_optimal_dim();
    wbutton.set_wh(dim);
    wbutton.set_xy(x, y);

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y);
    RED_CHECK(widget_for_receive_event.last_widget == nullptr);
    RED_CHECK(widget_for_receive_event.last_event == 0);
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y);
    RED_CHECK(widget_for_receive_event.last_widget == nullptr);
    RED_CHECK(widget_for_receive_event.last_event == 0);
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y);
    RED_CHECK(widget_for_receive_event.last_widget == nullptr);
    RED_CHECK(widget_for_receive_event.last_event == 0);
    RED_CHECK(notifier.last_widget == &wbutton);
    RED_CHECK(notifier.last_event == NOTIFY_SUBMIT);

    notifier.last_widget = nullptr;
    notifier.last_event = 0;
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y);
    RED_CHECK(widget_for_receive_event.last_widget == nullptr);
    RED_CHECK(widget_for_receive_event.last_event == 0);
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));
    using KFlags = Keymap::KbdFlags;
    using Scancode = Keymap::Scancode;

    keymap.event(KFlags(), Scancode(0x10)); // 'a'
    wbutton.rdp_input_scancode(KFlags(), Scancode(0x10), 0, keymap);
    RED_CHECK(widget_for_receive_event.last_widget == nullptr);
    RED_CHECK(widget_for_receive_event.last_event == 0);
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    keymap.event(KFlags(), Scancode(0x39)); // ' '
    wbutton.rdp_input_scancode(KFlags(), Scancode(0x39), 0, keymap);
    RED_CHECK(widget_for_receive_event.last_widget == nullptr);
    RED_CHECK(widget_for_receive_event.last_event == 0);
    RED_CHECK(notifier.last_widget == &wbutton);
    RED_CHECK(notifier.last_event == NOTIFY_SUBMIT);
    notifier.last_widget = nullptr;
    notifier.last_event = 0;

    keymap.event(KFlags(), Scancode(0x1c)); // Enter
    wbutton.rdp_input_scancode(KFlags(), Scancode(0x1c), 0, keymap);
    RED_CHECK(widget_for_receive_event.last_widget == nullptr);
    RED_CHECK(widget_for_receive_event.last_event == 0);
    RED_CHECK(notifier.last_widget == &wbutton);
    RED_CHECK(notifier.last_event == NOTIFY_SUBMIT);
    notifier.last_widget = nullptr;
    notifier.last_event = 0;
}

RED_AUTO_TEST_CASE(TraceWidgetButtonAndComposite)
{
    TestGraphic drawable(800, 600);


    // WidgetButton is a button widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetButton wbutton1(drawable, wcomposite, notifier,
                          "abababab", 0, YELLOW, BLACK, WINBLUE, 2,
                          global_font_deja_vu_14());
    Dimension dim = wbutton1.get_optimal_dim();
    wbutton1.set_wh(dim);
    wbutton1.set_xy(0, 0);

    WidgetButton wbutton2(drawable, wcomposite, notifier,
                          "ggghdgh", 0, WHITE, RED, WINBLUE, 2,
                          global_font_deja_vu_14());
    dim = wbutton2.get_optimal_dim();
    wbutton2.set_wh(dim);
    wbutton2.set_xy(0, 100);

    WidgetButton wbutton3(drawable, wcomposite, notifier,
                          "lldlslql", 0, BLUE, RED, WINBLUE, 2,
                          global_font_deja_vu_14());
    dim = wbutton3.get_optimal_dim();
    wbutton3.set_wh(dim);
    wbutton3.set_xy(100, 100);

    WidgetButton wbutton4(drawable, wcomposite, notifier,
                          "LLLLMLLM", 0, PINK, DARK_GREEN, WINBLUE, 2,
                          global_font_deja_vu_14());
    dim = wbutton4.get_optimal_dim();
    wbutton4.set_wh(dim);
    wbutton4.set_xy(300, 300);

    WidgetButton wbutton5(drawable, wcomposite, notifier,
                          "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, WINBLUE, 2,
                          global_font_deja_vu_14());
    dim = wbutton5.get_optimal_dim();
    wbutton5.set_wh(dim);
    wbutton5.set_xy(700, -10);

    WidgetButton wbutton6(drawable, wcomposite, notifier,
                          "xxwwp", 0, ANTHRACITE, PALE_GREEN, WINBLUE, 2,
                          global_font_deja_vu_14());
    dim = wbutton6.get_optimal_dim();
    wbutton6.set_wh(dim);
    wbutton6.set_xy(-10, 550);

    wcomposite.add_widget(&wbutton1);
    wcomposite.add_widget(&wbutton2);
    wcomposite.add_widget(&wbutton3);
    wcomposite.add_widget(&wbutton4);
    wcomposite.add_widget(&wbutton5);
    wcomposite.add_widget(&wbutton6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_12.png");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "button_13.png");

    wcomposite.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetButtonFocus)
{
    ButtonContextTest button(72, 40, "test7", 10, 10, 4, 1);
    RED_CHECK_IMG(button.drawable, IMG_TEST_PATH "button_14.png");

    auto& wbutton = button.wbutton;

    wbutton.focus(Widget::focus_reason_tabkey);
    wbutton.rdp_input_invalidate(wbutton.get_rect());
    RED_CHECK_IMG(button.drawable, IMG_TEST_PATH "button_15.png");

    wbutton.blur();
    wbutton.rdp_input_invalidate(wbutton.get_rect());
    RED_CHECK_IMG(button.drawable, IMG_TEST_PATH "button_14.png");

    wbutton.focus(Widget::focus_reason_tabkey);
    wbutton.rdp_input_invalidate(wbutton.get_rect());
    RED_CHECK_IMG(button.drawable, IMG_TEST_PATH "button_15.png");
}

RED_AUTO_TEST_CASE(TraceWidgetButtonLite)
{
    ButtonContextTest button("test1", 0, 0, 4, 1, 0, 0, 0, 0, /*border_width=*/1);
    RED_CHECK_IMG(button.drawable, IMG_TEST_PATH "button18.png");

    auto& wbutton = button.wbutton;

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, wbutton.x() + 2, wbutton.y() + 2);
    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));
    RED_CHECK_IMG(button.drawable, IMG_TEST_PATH "button19.png");
}

RED_AUTO_TEST_CASE(TraceWidgetButtonStrong)
{
    ButtonContextTest button("test1", 0, 0, 4, 1, 0, 0, 0, 0, /*border_width=*/5);
    RED_CHECK_IMG(button.drawable, IMG_TEST_PATH "button20.png");

    auto& wbutton = button.wbutton;

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, wbutton.x() + 2, wbutton.y() + 2);
    wbutton.rdp_input_invalidate(Rect(wbutton.x(), wbutton.y(), wbutton.cx(), wbutton.cy()));
    RED_CHECK_IMG(button.drawable, IMG_TEST_PATH "button21.png");
}
