/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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

#include "mod/internal/widget/selector.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/selector/"


RED_AUTO_TEST_CASE(TraceWidgetSelector)
{
    TestGraphic drawable(800, 600);


    // WidgetSelector is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Language::en);

    chars_view const add1[] = {
        "rdp"_av, "qa\\administrateur@10.10.14.111"_av, "RDP"_av};
    selector.add_device(add1);

    chars_view const add2[] = {
        "rdp"_av, "administrateur@qa@10.10.14.111"_av, "RDP"_av};
    selector.add_device(add2);

    chars_view const add3[] = {
        "rdp"_av, "administrateur@qa@10.10.14.27"_av, "RDP"_av};
    selector.add_device(add3);

    chars_view const add4[] = {
        "rdp"_av, "administrateur@qa@10.10.14.103"_av, "RDP"_av};
    selector.add_device(add4);

    chars_view const add5[] = {
        "rdp"_av, "administrateur@qa@10.10.14.33"_av, "RDP"_av};
    selector.add_device(add5);

    selector.selector_lines.set_selection(0);

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_1.png");

    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorResize)
{
    TestGraphic drawable(640, 480);


    // WidgetSelector is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 648, 480, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.weight[0] = 33;
    params.weight[1] = 34;
    params.weight[2] = 33;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier,
                                "1", "1",  extra_button, params, global_font_deja_vu_14(), Theme(), Language::en);

    chars_view const add1[] = {
        "rdp"_av, "qa\\administrateur@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add1);

    chars_view const add2[] = {
        "rdp"_av, "administrateur@qa@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add2);

    chars_view const add3[] = {
        "rdp"_av, "administrateur@qa@10.10.14.27"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add3);

    chars_view const add4[] = {
        "rdp"_av, "administrateur@qa@10.10.14.103"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add4);

    chars_view const add5[] = {
        "rdp"_av, "administrateur@qa@10.10.14.33"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add5);

    selector.selector_lines.set_selection(0);

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_resize_1.png");

    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_resize_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetSelector2)
{
    TestGraphic drawable(800, 600);


    // WidgetSelector is a selector widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.weight[0] = 33;
    params.weight[1] = 34;
    params.weight[2] = 33;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(),  Theme(), Language::en);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorClip)
{
    TestGraphic drawable(800, 600);


    // WidgetSelector is a selector widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.weight[0] = 33;
    params.weight[1] = 34;
    params.weight[2] = 33;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Language::en);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.x(),
                                      0 + selector.y(),
                                      selector.cx(),
                                      selector.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorClip2)
{
    TestGraphic drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context


    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.weight[0] = 33;
    params.weight[1] = 34;
    params.weight[2] = 33;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1",  extra_button, params, global_font_deja_vu_14(), Theme(), Language::en);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    selector.rdp_input_invalidate(Rect(20 + selector.x(),
                                      5 + selector.y(),
                                      30,
                                      10));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorEventSelect)
{
    TestGraphic drawable(800, 600);


    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.weight[0] = 33;
    params.weight[1] = 34;
    params.weight[2] = 33;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Language::en);

    chars_view const add1[] = {
        "rdp"_av, "qa\\administrateur@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add1);

    chars_view const add2[] = {
        "rdp"_av, "administrateur@qa@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add2);

    chars_view const add3[] = {
        "rdp"_av, "administrateur@qa@10.10.14.27"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add3);

    chars_view const add4[] = {
        "rdp"_av, "administrateur@qa@10.10.14.103"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add4);

    chars_view const add5[] = {
        "rdp"_av, "administrateur@qa@10.10.14.33"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add5);

    selector.set_widget_focus(&selector.selector_lines, Widget::focus_reason_tabkey);
    selector.selector_lines.set_selection(0);

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    selector.selector_lines.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                            selector.selector_lines.x() + 20,
                                            selector.selector_lines.y() + 40,
                                            nullptr);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_6.png");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_7.png");

    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_8.png");

    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_7.png");

    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_6.png");

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_7.png");

    // int x = selector.selector_lines.rect.x + 5;
    // int y = selector.selector_lines.rect.y + 3;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, nullptr);
    // x += selector.selector_lines.group_w;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, nullptr);
    // x += selector.selector_lines.target_w;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, nullptr);
    // x += selector.selector_lines.protocol_w;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, nullptr);
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorFilter)
{
    TestGraphic drawable(800, 600);


    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.weight[0] = 33;
    params.weight[1] = 34;
    params.weight[2] = 33;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";


    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Language::en);

    chars_view const add1[] = {
        "reptile"_av, "snake@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add1);

    chars_view const add2[] = {
        "bird"_av, "raven@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add2);

    chars_view const add3[] = {
        "reptile"_av, "lezard@10.10.14.27"_av,
        "VNC"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add3);

    chars_view const add4[] = {
        "fish"_av, "shark@10.10.14.103"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add4);

    chars_view const add5[] = {
        "bird"_av, "eagle@10.10.14.33"_av,
        "VNC"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add5);

    int curx = 0;
    int cury = 0;

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    selector.selector_lines.set_selection(0);

    curx = selector.edit_filters[0].x() + 2;
    cury = selector.edit_filters[0].y() + 2;
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                             curx, cury,
                             nullptr);
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                             curx, cury,
                             nullptr);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_12.png");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);

    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_13.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_14.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_15.png");

    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_16.png");

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_17.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_18.png");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_19.png");

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "selector_20.png");
}
