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

#include "mod/internal/widget/selector.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


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

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    array_view_const_char const add1[] = {
        "rdp"_av, "qa\\administrateur@10.10.14.111"_av, "RDP"_av};
    selector.add_device(add1);

    array_view_const_char const add2[] = {
        "rdp"_av, "administrateur@qa@10.10.14.111"_av, "RDP"_av};
    selector.add_device(add2);

    array_view_const_char const add3[] = {
        "rdp"_av, "administrateur@qa@10.10.14.27"_av, "RDP"_av};
    selector.add_device(add3);

    array_view_const_char const add4[] = {
        "rdp"_av, "administrateur@qa@10.10.14.103"_av, "RDP"_av};
    selector.add_device(add4);

    array_view_const_char const add5[] = {
        "rdp"_av, "administrateur@qa@10.10.14.33"_av, "RDP"_av};
    selector.add_device(add5);

    selector.selector_lines.set_selection(0);

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector1.png");

    RED_CHECK_SIG(drawable, "\x87\x7a\x17\x85\x48\x78\x04\xd1\xb9\xf5\x1a\x50\x0d\xc0\x7c\xa5\x47\x56\x55\x74");


    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector2.png");

    RED_CHECK_SIG(drawable, "\xa9\xf3\x4a\x66\x45\xf8\xaf\x92\x0b\x75\xc9\x51\x8c\x39\x63\xb2\x20\x12\x3d\xe4");
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
                                "1", "1",  extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    array_view_const_char const add1[] = {
        "rdp"_av, "qa\\administrateur@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add1);

    array_view_const_char const add2[] = {
        "rdp"_av, "administrateur@qa@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add2);

    array_view_const_char const add3[] = {
        "rdp"_av, "administrateur@qa@10.10.14.27"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add3);

    array_view_const_char const add4[] = {
        "rdp"_av, "administrateur@qa@10.10.14.103"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add4);

    array_view_const_char const add5[] = {
        "rdp"_av, "administrateur@qa@10.10.14.33"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add5);

    selector.selector_lines.set_selection(0);

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector-resize1.png");

    RED_CHECK_SIG(drawable, "\x60\x3a\xbd\x08\x4f\x11\x2d\x54\x10\xd6\x77\xae\x36\x99\x25\x1a\x9a\xfc\xa7\x39");


    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector-resize2.png");

    RED_CHECK_SIG(drawable, "\x3f\x01\x69\x46\xc9\xd0\x04\x8d\x28\x65\x76\x55\xc0\x8b\x66\xb7\xd6\x8f\x0b\x4a");
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

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(),  Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector3.png");

    RED_CHECK_SIG(drawable, "\x80\xd3\xcb\xcd\x40\x82\x6f\xb4\x3b\x7d\x6b\x2e\xae\x10\xef\x80\x69\x4f\x01\x6f");
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

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.x(),
                                      0 + selector.y(),
                                      selector.cx(),
                                      selector.cy()));

    //drawable.save_to_png("selector4.png");

    RED_CHECK_SIG(drawable, "\xc4\x1f\xe7\x63\xe0\x8f\xbd\x60\x6a\x85\x69\x45\x1a\x0f\x66\x82\xfd\xcf\x9b\x03");
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

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1",  extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    selector.rdp_input_invalidate(Rect(20 + selector.x(),
                                      5 + selector.y(),
                                      30,
                                      10));

    //drawable.save_to_png("selector5.png");

    RED_CHECK_SIG(drawable, "\x9d\xbe\x64\x88\x34\x4b\x79\x83\x6a\xa1\x36\xad\xf4\x9e\xe6\x89\xb8\xf3\x86\x87");
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

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    array_view_const_char const add1[] = {
        "rdp"_av, "qa\\administrateur@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add1);

    array_view_const_char const add2[] = {
        "rdp"_av, "administrateur@qa@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add2);

    array_view_const_char const add3[] = {
        "rdp"_av, "administrateur@qa@10.10.14.27"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add3);

    array_view_const_char const add4[] = {
        "rdp"_av, "administrateur@qa@10.10.14.103"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add4);

    array_view_const_char const add5[] = {
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

    //drawable.save_to_png("selector6-1.png");

    RED_CHECK_SIG(drawable, "\x59\x8d\x46\x0d\xe2\x74\x7e\x4a\x17\xbc\xfd\x12\x3c\xc2\xe2\x2a\xc6\x47\x8b\x81");


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector6-2.png");

    RED_CHECK_SIG(drawable, "\x16\xdf\x52\xec\x39\xc3\x0f\xfe\x24\x9a\xa6\xfa\x47\x2f\x14\xe4\xf6\xb1\x75\xda");


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector6-3.png");

    RED_CHECK_SIG(drawable, "\xd0\xb0\xcc\x0d\x0c\x37\x79\xcc\xde\x7f\x4e\x5b\x98\x3e\x11\x5d\x73\x0e\x2f\x89");


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector6-4.png");

    RED_CHECK_SIG(drawable, "\x16\xdf\x52\xec\x39\xc3\x0f\xfe\x24\x9a\xa6\xfa\x47\x2f\x14\xe4\xf6\xb1\x75\xda");


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector6-5.png");

    RED_CHECK_SIG(drawable, "\x59\x8d\x46\x0d\xe2\x74\x7e\x4a\x17\xbc\xfd\x12\x3c\xc2\xe2\x2a\xc6\x47\x8b\x81");


    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector6-6.png");

    RED_CHECK_SIG(drawable, "\x16\xdf\x52\xec\x39\xc3\x0f\xfe\x24\x9a\xa6\xfa\x47\x2f\x14\xe4\xf6\xb1\x75\xda");

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


    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    array_view_const_char const add1[] = {
        "reptile"_av, "snake@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add1);

    array_view_const_char const add2[] = {
        "bird"_av, "raven@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add2);

    array_view_const_char const add3[] = {
        "reptile"_av, "lezard@10.10.14.27"_av,
        "VNC"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add3);

    array_view_const_char const add4[] = {
        "fish"_av, "shark@10.10.14.103"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add4);

    array_view_const_char const add5[] = {
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

    //drawable.save_to_png("selector7-1.png");

    RED_CHECK_SIG(drawable, "\xe0\x47\xea\x42\xe6\x9c\x41\xf4\x1f\x96\xe3\x95\x3d\x7b\x31\x3a\xdb\x23\x8f\x03");


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);

    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector7-2.png");

    RED_CHECK_SIG(drawable, "\x45\xad\xdb\x14\xc4\x13\xdf\xf0\x73\x90\x48\x2c\xf9\x73\xb5\x5f\x16\x6a\x50\xa9");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector7-3.png");

    RED_CHECK_SIG(drawable, "\x05\xf2\xa1\x89\xa4\x85\xd9\xd8\xa1\xee\x78\xed\x01\xe5\x8f\x9a\x71\x2b\x42\x9c");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector7-4.png");

    RED_CHECK_SIG(drawable, "\x73\xf9\x3f\x58\x4c\x22\xe5\x9f\x65\x66\x9c\xba\x79\xb4\x41\x7a\xa8\xf3\x0d\xbd");


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector7-5.png");

    RED_CHECK_SIG(drawable, "\x77\xdf\xb0\x68\x9e\x14\xc2\xc9\x64\x2f\xf7\x6d\xc5\xf4\xe3\x69\x33\x6c\x58\x00");


    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector7-6.png");

    RED_CHECK_SIG(drawable, "\x34\x69\xda\x57\x87\x9e\x41\x9e\x61\xe0\x1a\x29\x5b\xef\xa2\x93\x7e\xbe\x46\x97");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector7-7.png");

    RED_CHECK_SIG(drawable, "\x7a\xe4\x21\xd6\xf7\xdb\xcb\x8a\x19\x40\x6d\xbe\xda\xd1\xf4\xae\xcc\xdb\xc5\xdb");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    //drawable.save_to_png("selector7-8.png");

    RED_CHECK_SIG(drawable, "\x48\x5d\x5e\x29\x99\xa6\x3b\x29\x88\xd7\x84\xea\x21\x15\xf5\x5f\xc5\x63\xb8\xb6");


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

    //drawable.save_to_png("selector7-9.png");

    RED_CHECK_SIG(drawable, "\x4b\xb1\x5d\xa1\xb1\xcd\x93\x05\x8a\x6e\x31\x66\x74\x04\x4f\xba\x45\xc4\xe4\x4e");
}
